#ifndef MONO_SLAM_FRONT_END_TRACKING_H_
#define MONO_SLAM_FRONT_END_TRACKING_H_

#include "mono_slam/local_mapping.h"
#include "mono_slam/common_include.h"
#include "mono_slam/frame.h"
#include "mono_slam/initialization.h"
#include "mono_slam/map.h"
#include "mono_slam/system.h"
#include "mono_slam/viewer.h"

namespace mono_slam {

class System;
class LocalMapping;
class Map;
class Frame;
class Viewer;
class Initializer;
class KeyFrameDB;

class Tracking {
 public:
  EIGEN_MAKE_ALIGNED_OPERATOR_NEW
  using Ptr = sptr<Tracking>;

  enum class State { NOT_INITIALIZED_YET, GOOD, LOST };

  Tracking::State state_;            // Tracking state.
  Frame::Ptr last_frame_ = nullptr;  // Last frame.
  Frame::Ptr curr_frame_ = nullptr;  // Current frame.
  SE3 T_curr_last_;  // Rigid transformation from last_frame_ to curr_frame_
                     // assuming constant velocity.
  unordered_set<Frame::Ptr>
      local_co_kfs_;  // Local covisible keyframes having
                      // visual overlapping with current frame.
  // FIXME Seems the effect of this is not significant. Remove this?
  int last_kf_id_;  // Id of last keyframe. Frequency of keyframe
                    // insertion is partly limited by this.

  // Linked components.
  sptr<System> system_ = nullptr;              // System.
  sptr<LocalMapping> local_mapper_ = nullptr;  // Local mapper.
  Map::Ptr map_ = nullptr;                     // Map.
  KeyframeDB::Ptr keyframe_db_ = nullptr;      // Keyframe database.
  sptr<Viewer> viewer_ = nullptr;              // Viewer.

  Tracking();
  ~Tracking();

  // Entry function.
  void addImage(const cv::Mat& img);

  // Setters.
  void setSystem(sptr<System> system);
  void setLocalMapper(sptr<LocalMapping> local_mapper);
  void setMap(Map::Ptr map);
  void setKeyframeDB(KeyframeDB::Ptr keyframe_db);
  void setViewer(sptr<Viewer> viewer);
  void setInitializer(uptr<Initializer> initializer);
  void setVocabulary(const sptr<Vocabulary>& voc);
  void setCamera(Camera* cam);

  void reset();

 private:
  // Track current frame.
  void trackCurrentFrame();

  // Initialize map: collect two consecutive frames and try initialization.
  bool initMap();

  // Track current frame from last frame assuming contant velocity model.
  bool trackFromLastFrame();

  // Track local map to make the tracking more robust.
  void trackFromLocalMap();

  // Update local covisible keyframes to be used in tracking from local map.
  void updateLocalCoKfs();

  // True if the criteria of inserting new keyframe are satisfied.
  bool needNewKf();

  // Relocalize if tracking is lost.
  bool relocalization();

 private:
  // User specified objects.
  uptr<Initializer> initializer_ = nullptr;          // Initializer.
  sptr<Vocabulary> voc_ = nullptr;                   // Vocabulary.
  Camera* cam_ = nullptr;                        // Camera.
  cv::Ptr<cv::FeatureDetector> detector_ = nullptr;  // Feature detector.
};

namespace tracking_utils {}  // namespace tracking_utils

}  // namespace mono_slam

#endif  // MY_FRONT_END_TRACKING