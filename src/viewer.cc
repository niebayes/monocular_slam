#include "mono_slam/viewer.h"

#include "mono_slam/initialization.h"
#include "utils/opencv_drawer_utils.h"

// Our own scoped_lock class meeting BasicLockable requirement despite locking
// only two mutexes at a time.
class scoped_lock2 {
  // Mutexes are of course maintained by references.
  std::mutex& mut_1_;
  std::mutex& mut_2_;

 public:
  //! Follow the RAII idiom, acquire resources during construction and
  //! deallocate resources upon destruction.
  scoped_lock2(std::mutex& mut_1, std::mutex& mut_2)
      : mut_1_(mut_1), mut_2_(mut_2) {
    lock();
  }
  ~scoped_lock2() { unlock(); }
  //! Disable both copy constructor and assignment operator.
  //! The preference of doing this is that since C++11 move semantics shall
  //! always be perfered over ctor and assignment. By disable both, we're
  //! enforced to implement more efficient codes where there's need to copy or
  //! assign instances.
  scoped_lock2(const scoped_lock2&) = delete;
  scoped_lock2& operator=(const scoped_lock2&) = delete;
  void lock() { std::lock(mut_1_, mut_2_); }
  void unlock() {
    mut_1_.unlock();
    mut_2_.unlock();
  }
};

namespace mono_slam {

class Initializer;

Viewer::Viewer() {
  pcl_viewer_.reset(new viewer_utils::PclViewer());
  startThread();
}

void Viewer::startThread() {
  is_running_.store(true);
  thread_ = std::thread(std::bind(&Viewer::drawingLoop, this));
}

void Viewer::stopThread() {
  LOG(INFO) << "Request stopping viewer ...";
  is_running_.store(false);
  update_cond_var_.notify_one();
  thread_.join();
  LOG(INFO) << "Viewer stopped.";
}

void Viewer::informUpdate() {
  lock_g lock(mut_);
  update_cond_var_.notify_one();
}

void Viewer::drawingLoop() {
  while (is_running_.load()) {
    {
      scoped_lock2 lock(mut_, tracker_->mut_);
      // Only perform drawing when the tracking is good.
      update_cond_var_.wait(lock,
                            [this] { return tracker_->state_ == State::GOOD; });
      // Get frames and map points by copy making viewer thread completely
      // independent of others.
      last_frame_ = tracker_->last_frame_;
      curr_frame_ = tracker_->curr_frame_;
      points_ = map_->getAllMapPoints();
    }
    LOG(INFO) << "Viewer is updating ...";
    // Invoke OpenCV drawer.
    cv::Mat img_show;
    if (last_frame_->is_datum_)  // If system was just initialized.
      viewer_utils::OpencvDrawer::drawMatches(
          last_frame_, curr_frame_, tracker_->initializer_->inlier_matches_,
          img_show);
    else
      viewer_utils::OpencvDrawer::drawKeyPoints(curr_frame, img_show);

    // Invoke PCL viewer.
    updateMap();
    LOG(INFO) << "Updated viewer.";
  }
}

void Viewer::updateMap() {
  lock_g lock(mut_);
  drawTrajectory();
  drawMapPoints();
  pcl_viewer_->spinOnce();
}

void Viewer::drawTrajectory() {
  const bool no_ground_truth = tracker_->system_->pose_ground_truths_.empty();
  if (last_frame_->is_datum_) {
    pcl_viewer_->insertPoseEstimate(last_frame_->pose(), true);
    if (!no_ground_truth) {
      pcl_viewer_->insertPoseGroundTruth(
          tracker_->system_->pose_ground_truths_.at(last_frame_->id_));
    }
  }
  pcl_viewer_->insertPoseEstimate(curr_frame_->pose(),
                                  curr_frame_->isKeyframe());
  if (!no_ground_truth) {
    pcl_viewer_->insertPoseGroundTruth(
        tracker_->system_->pose_ground_truths_.at(curr_frame_->id_));
  }
}

void Viewer::drawMapPoints() {
  for (const MapPoint::Ptr& point : points_) {
    if (!point) continue;
    // If the map point is first observed by curr_frame_, it's the new map point
    // and will be rendered with a different color.
    if (point->ref_frame_id_ == curr_frame_->id_)
      pcl_viewer_->insertNewMapPoint(point->pos());
    else
      pcl_viewer_->insertMapPoint(point->pos());
  }
}

void Viewer::reset() {
  lock_g lock(mut_);
  last_frame_.reset();
  curr_frame_.reset();
  points_.clear();
  pcl_viewer_.reset(new viewer_utils::PclViewer());
}

void Viewer::setTracker(sptr<Tracking> tracker) { tracker_ = tracker; }

void Viewer::setMap(sptr<Map> map) { map_ = map; }

}  // namespace mono_slam
