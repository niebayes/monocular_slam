#ifndef MONO_SLAM_FRONT_END_TRACKING_H_
#define MONO_SLAM_FRONT_END_TRACKING_H_

#include "mono_slam/back_end_local_mapping.h"
#include "mono_slam/common_include.h"
#include "mono_slam/frame.h"
#include "mono_slam/front_end_map_initialization.h"
#include "mono_slam/keyframe_database.h"
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

  Tracking::State state_;  // Tracking state.

  Frame::Ptr last_frame_ = nullptr;  // Last frame.
  Frame::Ptr curr_frame_ = nullptr;  // Current frame.
  // FIXME Need this?
  vector<int> matches_;  // Matches of last frame and current frame such that
                         // last_frame_[i] <-> curr_frame_[matches_[i]].
  SE3 const_velocity_;   // Right transformation from last_frame_ to curr_frame_
                         // assuming contant velocity.
  int last_keyframe_id_;

  // Linked components.
  sptr<System> system_ = nullptr;              // System.
  sptr<LocalMapping> local_mapper_ = nullptr;  // Local mapper.
  Map::Ptr map_ = nullptr;                     // Map.
  KeyframeDB::Ptr keyframe_db_ = nullptr;      // Keyframe database.
  sptr<Viewer> viewer_ = nullptr;              // Viewer.

  Tracking();

  // Entry function.
  void AddImage(const cv::Mat& img);

  // Setters.
  void SetSystem(sptr<System> system);
  void SetLocalMapper(sptr<LocalMapping> local_mapper);
  void SetMap(Map::Ptr map);
  void SetKeyframeDB(KeyframeDB::Ptr keyframe_db);
  void SetViewer(sptr<Viewer> viewer);
  void SetInitializer(uptr<Initializer> initializer);
  void SetVocabulary(const sptr<Vocabulary>& voc);
  void SetCamera(Camera::Ptr cam);
  void SetFeatureDetector(const cv::Ptr<cv::FeatureDetector>& detector);

  void Reset();

 private:
  // Track current frame.
  void TrackCurrentFrame();

  // Initialize map: collect two consecutive frames and try initialization.
  bool InitMap();

  // Track current frame from last frame assuming contant velocity model.
  bool TrackWithConstantVelocityModel();

  // Track local map to make the tracking more robust.
  void TrackLocalMap();

  // True if the criteria of inserting new keyframe are satisfied.
  bool NeedNewKeyframe();

  // Relocalize if tracking is lost.
  bool Relocalization();

 private:
  // User specified objects.
  uptr<Initializer> initializer_ = nullptr;          // Initializer.
  sptr<Vocabulary> voc_ = nullptr;                   // Vocabulary.
  Camera::Ptr cam_ = nullptr;                        // Camera.
  cv::Ptr<cv::FeatureDetector> detector_ = nullptr;  // Feature detector.
};

namespace tracking_utils {}  // namespace tracking_utils

}  // namespace mono_slam

#endif  // MY_FRONT_END_TRACKING