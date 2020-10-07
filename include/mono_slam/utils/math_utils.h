#ifndef MONO_SLAM_UTILS_MATH_UTILS_H_
#define MONO_SLAM_UTILS_MATH_UTILS_H_

#include <algorithm>
#include <chrono>
#include <cmath>
#include <iterator>
#include <vector>

#include "glog/logging.h"
#include "mono_slam/config.h"

using namespace mono_slam;

namespace math_utils {

inline int uniform_random_int(const int low, const int high) {
  const unsigned seed =
      std::chrono::system_clock::now().time_since_epoch().count();
  std::default_random_engine generator(seed);
  std::uniform_int_distribution<int> distribution(low, high);
  auto dice_once = std::bind(distribution, generator);
  return dice_once();
}

inline double degree2radian(const double degree) {
  return degree * EIGEN_PI / 180.0;
}

inline double radian2degree(const double radian) {
  return radian * 180.0 / EIGEN_PI;
}

inline Mat34 kRt2mat(const Mat33& K, const Mat33& R, const Vec3& t) {
  Mat34 Rt;
  Rt.leftCols(3) = R;
  Rt.rightCols(1) = t;
  return K * Rt;
}

// FIXME Should this function "sort" data vector in place?
template <typename T>
inline T get_median(std::vector<T>& data_vec) {
  CHECK_EQ(data_vec.empty(), false);
  typename std::vector<T>::iterator it = std::next(
      data_vec.begin(), std::floor(static_cast<int>(data_vec.size()) / 2));
  std::nth_element(data_vec.begin(), it, data_vec.end());
  return *it;
}

}  // namespace math_utils

#endif  // MONO_SLAM_UTILS_MATH_UTILS_H_