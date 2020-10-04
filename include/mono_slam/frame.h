#ifndef MONO_SLAM_FRAME_H_
#define MONO_SLAM_FRAME_H_

#include "mono_slam/camera.h"
#include "mono_slam/common_include.h"
#include "mono_slam/feature.h"
#include "mono_slam/g2o_optimizer/types.h"
#include "mono_slam/map_point.h"

namespace mono_slam {

class Camera;
struct Feature;
class MapPoint;

class Frame {
 public:
  EIGEN_MAKE_ALIGNED_OPERATOR_NEW
  using Ptr = sptr<Frame>;
  using Features = std::vector<sptr<Feature>>;  // weak_ptr to avoid cyclic ref.

  // FIXME Should frame has a member denoting self a frame to be deleted?

  static int frame_cnt_;             // Global frame counter, starting from 0.
  const int id_;                     // Unique frame identity.
  bool is_keyframe_;                 // Is this frame a keyframe?
  Features feats_;                   // Features extracted in this frame.
  const Camera::Ptr cam_ = nullptr;  // Linked camera.
  DBoW3::BowVector bow_vec_;         // Bag of words vector.
  DBoW3::FeatureVector feat_vec_;    // Feature vector.

  // Temporary variables used for relocalization.
  int query_frame_id_;     // Id of currently quering frame.
  int n_sharing_words_;    // Number of sharing words between this and currently
                           // quering frame.
  double bow_simi_score_;  // Similarity score between the bag-of-words vector
                           // of this and that of currently quering frame.
  bool is_candidate_already_;  // Is this keyframe selected as relocalization
                               // candidate already?

  // Temporary g2o keyframe vertex storing the optimized result.
  sptr<g2o_types::VertexFrame> v_frame_ = nullptr;

  // Variables used for covisibility graph.
  unordered_map<Frame::Ptr, int> co_kf_weights_;
  forward_list<Frame::Ptr> co_kfs_;
  forward_list<int> co_weights_;

  Frame(const cv::Mat& img, Camera::Ptr cam, const sptr<Vocabulary>& voc,
        const cv::Ptr<cv::FeatureDetector>& detector);

  inline const SE3& pose() const { return cam_->pose(); }

  void setPose(const SE3& T_c_w);

  inline bool isKeyframe() const { return is_keyframe_; }

  void setKeyframe();

  // Number of observations (i.e. number of features observed in this frame).
  inline int numObs() const { return feats_.size(); }

  // Extract features.
  void extractFeatures(const cv::Mat& img,
                       const cv::Ptr<cv::FeatureDetector>& detector);

  // Compute bag of words representation.
  void computeBoW(const sptr<Vocabulary>& voc);

  // Search features given searching radius and image pyramid level range.
  vector<int> searchFeatures(const Vec2& pt, const int radius,
                             const int level_low, const int level_high) const;

  // Check if the given map point is Observable by this frame.
  bool isObservable(const sptr<MapPoint>& point) const;

  void addConnection();

  void deleteConnection();

  void updateCoKfsAndWeights();

  void updateCoInfo();

  double computeSceneMedianDepth();

  inline const forward_list<Frame::Ptr>& getCoKfs(
      const int n = std::numeric_limits<int>::max()) const {
    const int n_kfs = static_cast<int>(
        std::distance(co_kf_weights_.cbegin(), co_kf_weights_.cend()));
    if (n > n_kfs) return co_kf_weights_;
    return forward_list(co_kf_weights_.cbegin(),
                        std::next(co_kf_weights_.cbegin(), n));
  }

  // Compute number of tracked map points (i.e. ones that are observed by more
  // than min_n_obs frames).
  int computeTrackedPoints(const int min_n_obs) const;

  // FIXME Personally, this method should be in map.
  // Erase the links between this frame and other stuff.
  void erase();

 private:
  // Image bounds.
  static double x_min_;
  static double x_max_;
  static double y_min_;
  static double y_max_;
};

namespace frame_utils {

void undistortKeypoints(const Mat33& K, const Vec4& dist_coeffs,
                        std::vector<cv::KeyPoint>& kpts);

void computeImageBounds(const cv::Mat& img, const Mat33& K,
                        const Vec4& dist_coeffs, cv::Mat& corners);

}  // namespace frame_utils
}  // namespace mono_slam

#endif  // MONO_SLAM_FRAME_H_