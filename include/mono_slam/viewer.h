#ifndef MONO_SLAM_VIEWER_H_
#define MONO_SLAM_VIEWER_H_

#include "mono_slam/common_include.h"
#include "mono_slam/map.h"
#include "mono_slam/system.h"

namespace mono_slam {

class System;
class Map;

class Viewer {
 public:
  using Ptr = sptr<Viewer>;

  Viewer();

  void reset();

  void setMap(const Map::Ptr& map);

  void setSystem(const sptr<System>& system);

 private:
  Map::Ptr map_ = nullptr;
  sptr<System> system_ = nullptr;
};

}  // namespace mono_slam

#endif  // MONO_SLAM_VIEWER_H_