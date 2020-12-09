#ifndef AABB_H_
#define AABB_H_


#include <glm/glm.hpp>

namespace GLOO {


struct AABB {
  AABB() {
  }
  AABB(const glm::vec3& _mn, const glm::vec3& _mx) : mn(_mn), mx(_mx) {
  }
  AABB(float mnx, float mny, float mnz, float mxx, float mxy, float mxz)
      : mn(glm::vec3(mnx, mny, mnz)), mx(glm::vec3(mxx, mxy, mxz)) {
  }


  void UnionWith(const AABB& other);
  bool Overlap(const AABB& other) const;
  bool Contain(const AABB& other) const;

  glm::vec3 mn, mx;

  // Note: below is added for final project
  glm::vec3 GetCenter() const;
  void PrintBox() const;
  // BoxBound bound;
  bool isInf;
};

} //namespace GLOO


#endif
