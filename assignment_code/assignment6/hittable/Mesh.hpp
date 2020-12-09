#ifndef MESH_H_
#define MESH_H_

#include "HittableBase.hpp"

#include "gloo/alias_types.hpp"

#include "Triangle.hpp"
#include "Octree.hpp"
#include "BVH.hpp"
#include "ArgParser.hpp"

namespace GLOO {
// class Octree;
class BVH;

class Mesh : public HittableBase {
 public:
  Mesh(std::unique_ptr<PositionArray> positions,
       std::unique_ptr<NormalArray> normals,
       std::unique_ptr<IndexArray> indices,
       IndexerType idxType);

  bool Intersect(const Ray& ray, float t_min, HitRecord& record) const override;
  AABB CreateBoundingBox() const override;
  bool IsMesh() const override {
    return true;
  }
  const std::vector<Triangle>& GetTriangles() const {
    return triangles_;
  }

 private:
  std::vector<Triangle> triangles_;
  std::unique_ptr<Octree> octree_;
  std::unique_ptr<BVH> bvh_;
  IndexerType idx_type_;
};
}  // namespace GLOO

#endif
