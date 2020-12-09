#include "Triangle.hpp"

#include <iostream>
#include <stdexcept>

#include <glm/common.hpp>
#include <glm/gtx/string_cast.hpp>
#include <cassert>

#include "Plane.hpp"

namespace GLOO {
Triangle::Triangle(const glm::vec3& p0,
                   const glm::vec3& p1,
                   const glm::vec3& p2,
                   const glm::vec3& n0,
                   const glm::vec3& n1,
                   const glm::vec3& n2) {
  positions_.push_back(p0);
  positions_.push_back(p1);
  positions_.push_back(p2);
  normals_.push_back(n0);
  normals_.push_back(n1);
  normals_.push_back(n2);
}

Triangle::Triangle(const std::vector<glm::vec3>& positions,
                   const std::vector<glm::vec3>& normals) {
}

AABB Triangle::CreateBoundingBox() const {
  AABB bvol;
  const Triangle *triangle = this;
  bvol.mn = bvol.mx = triangle->GetPosition(0);
  for (int i = 1; i < 3; i++) {
    for (int dim = 0; dim < 3; dim++) {
      bvol.mn[dim] = std::min(bvol.mn[dim], triangle->GetPosition(i)[dim]);
      bvol.mx[dim] = std::max(bvol.mx[dim], triangle->GetPosition(i)[dim]);
    }
  }
  return bvol;
}

bool Triangle::Intersect(const Ray& ray, float t_min, HitRecord& record) const {
  // Equations from Ray casting II lecture slides
  glm::mat3 A = glm::mat3(
    positions_[0] - positions_[1], positions_[1] - positions_[2], ray.GetDirection()
  );

  // Cramer's rule
  float A_det = glm::determinant(A);
  assert(A_det > 0.0);
  glm::mat3 beta_num = glm::mat3(
    positions_[0] - ray.GetOrigin(), positions_[0] - positions_[2], ray.GetDirection()
  );
  glm::mat3 gamma_num = glm::mat3(
    positions_[0] - positions_[1], positions_[0] - ray.GetOrigin(), ray.GetDirection()
  );
  glm::mat3 t_num = glm::mat3(
    positions_[0] - positions_[1], positions_[0] - positions_[2], positions_[0] - ray.GetOrigin()
  );

  float beta = glm::determinant(beta_num) / A_det;
  float gamma = glm::determinant(gamma_num) / A_det;
  float t = glm::determinant(t_num) / A_det;
  float alpha = 1.0 - beta - gamma;
  if (t > record.time || t < t_min || alpha < 0.0 || beta < 0.0 || gamma < 0.0) {
    return false;
  }
  record.time = t;
  record.normal = glm::normalize(alpha*normals_[0] + beta*normals_[1] + gamma*normals_[2]);
  return true;
}
}  // namespace GLOO
