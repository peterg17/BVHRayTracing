#include "Plane.hpp"
#include <iostream>

namespace GLOO {
Plane::Plane(const glm::vec3& normal, float d) {
  normal_ = normal;
  d_ = d;
}

AABB Plane::CreateBoundingBox() const {
  // should we set special flags to handle this instead?
  // AABB bvol(glm::vec3(-10000,-10000,-10000), glm::vec3(10000,10000,10000));
  AABB bvol;
  bvol.isInf = true;
  return bvol;
}

bool Plane::Intersect(const Ray& ray, float t_min, HitRecord& record) const {
  // TODO: Implement ray-plane intersection.
  glm::vec3 R_d = ray.GetDirection();
  glm::vec3 R_o = ray.GetOrigin();
  float num = d_ - glm::dot(normal_, R_o);
  float denom = glm::dot(normal_, glm::normalize(R_d));
  // std::cout << "numerator is: " << num << std::endl;
  // std::cout << "normal is: " << glm::to_string(normal_) << std::endl;
  float t = num / denom;
  // std::cout << "Plane intersects at t: " << t << std::endl; 
  
  // verify that intersection is closer than before
  if (t >= record.time) {
    return false;
  }
  if (t < t_min) {
    return false;
  }

  record.normal = normal_;
  record.time = t;
  return true;
}
}  // namespace GLOO
