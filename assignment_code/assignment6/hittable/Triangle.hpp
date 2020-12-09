#ifndef TRIANGLE_H_
#define TRIANGLE_H_

#include <vector>

#include "AABB.hpp"
#include "HittableBase.hpp"

namespace GLOO {
class Triangle : public HittableBase {
 public:
  Triangle(const glm::vec3& p0,
           const glm::vec3& p1,
           const glm::vec3& p2,
           const glm::vec3& n0,
           const glm::vec3& n1,
           const glm::vec3& n2);
  Triangle(const std::vector<glm::vec3>& positions,
           const std::vector<glm::vec3>& normals);

  bool Intersect(const Ray& ray, float t_min, HitRecord& record) const override;
  bool IsMesh() const override {
    return false;
  }
  glm::vec3 GetPosition(size_t i) const {
    return positions_[i];
  }
  glm::vec3 GetNormal(size_t i) const {
    return normals_[i];
  }

  std::string ToString() const {
    std::string triangleStr;
    triangleStr += "[" + glm::to_string(GetPosition(0));
    triangleStr += " , " + glm::to_string(GetPosition(1));
    triangleStr += " , " + glm::to_string(GetPosition(2)) + "]";
    return triangleStr;
  }

  // bool operator < (const Triangle& triangle) const {
  //   AABB bbox1 = h1->CreateBoundingBox();
  //   AABB bbox2 = h1->CreateBoundingBox();
  //   glm::vec3 center1 = bbox1.GetCenter();
  //   glm::vec3 center2 = bbox2.GetCenter();
  // }

  AABB CreateBoundingBox() const override;

 private:
  std::vector<glm::vec3> positions_;
  std::vector<glm::vec3> normals_;
};
}  // namespace GLOO

#endif
