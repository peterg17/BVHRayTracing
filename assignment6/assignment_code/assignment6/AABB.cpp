#include "AABB.hpp"
#include <algorithm>
#include <iostream>

namespace {

bool IntervalIntersect(float* a, float* b) {
  if (a[0] > b[1]) {
    return a[0] <= b[1];
  } else {
    return b[0] <= a[1];
  }
}

}

namespace GLOO {

bool AABB::Overlap(const AABB& other) const {
  for (int dim = 0; dim < 3; dim++) {
    float ia[2] = {mn[dim], mx[dim]};
    float ib[2] = {other.mn[dim], other.mx[dim]};
    bool intersect = IntervalIntersect(ia, ib);
    if (!intersect) {
      return false;
    }
  }
  return true;  
}

bool AABB::Contain(const AABB& other) const {
  for (int dim = 0; dim < 3; dim++) {
    if (mn[dim] > other.mn[dim] || mx[dim] < other.mx[dim]) {
      return false;
    }
  }
  return true;
}

void AABB::UnionWith(const AABB& other) {
  for (int dim = 0; dim < 3; dim++) {
    mn[dim] = std::min(mn[dim], other.mn[dim]);
    mx[dim] = std::max(mx[dim], other.mx[dim]);
  }
}

void AABB::PrintBox() const {
  std::cout << "[AABB] bbox at [(" << mn[0] << " , " << mn[1] << " , " << mn[2] << "),";
  std::cout << "(" << mx[0] << " , " << mx[1] << " , " << mx[2] <<  ")]"  << std::endl;
}

glm::vec3 AABB::GetCenter() const {
  glm::vec3 half = glm::vec3(0.5, 0.5, 0.5);
  glm::vec3 center = (mn + mx) * half;
  return center;
}

// void 


}