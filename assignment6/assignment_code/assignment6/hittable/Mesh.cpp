#include "Mesh.hpp"

#include "time.h"
#include <functional>
#include <stdexcept>
#include <iostream>

#include "gloo/utils.hpp"

namespace GLOO {
Mesh::Mesh(std::unique_ptr<PositionArray> positions,
           std::unique_ptr<NormalArray> normals,
           std::unique_ptr<IndexArray> indices,
           IndexerType idxType) {
  size_t num_vertices = indices->size();
  if (num_vertices % 3 != 0 || normals->size() != positions->size())
    throw std::runtime_error("Bad mesh data in Mesh constuctor!");

  for (size_t i = 0; i < num_vertices; i += 3) {
    triangles_.emplace_back(
        positions->at(indices->at(i)), positions->at(indices->at(i + 1)),
        positions->at(indices->at(i + 2)), normals->at(indices->at(i)),
        normals->at(indices->at(i + 1)), normals->at(indices->at(i + 2)));
  }
  // Let mesh data destruct.

  clock_t tStart = clock();
  // Build Octree/BVH/Kd-tree.
  idx_type_ = idxType;
  if (idxType == IndexerType::Octree) {
    octree_ = make_unique<Octree>();
    octree_->Build(*this);
  } else if (idxType == IndexerType::BVH) {
    bvh_ = make_unique<BVH>(SplittingHeuristic::None);
    bvh_->Build(*this);
  } else if (idxType == IndexerType::BVH_SAH) {
    bvh_ = make_unique<BVH>(SplittingHeuristic::SAH);
    bvh_->Build(*this);
  } else {
    throw std::runtime_error("[Mesh] constructor doesn't recognize indexer type...");
  }
  printf("Build Time: %.2fs\n", (double)(clock() - tStart)/CLOCKS_PER_SEC);
}

AABB Mesh::CreateBoundingBox() const {
  const Mesh *mesh = this;
  auto& triangles = mesh->GetTriangles();
  const Triangle& firstTriangle = triangles[0];
  AABB bbox(firstTriangle.CreateBoundingBox());
  for (size_t i = 1; i < triangles.size(); i++) {
    bbox.UnionWith(triangles[i].CreateBoundingBox());
  }
  return bbox;
}


 
bool Mesh::Intersect(const Ray& ray, float t_min, HitRecord& record) const {
  if (idx_type_ == IndexerType::BVH_SAH) {
    return bvh_->Intersect(ray, t_min, record);
  } else if (idx_type_ == IndexerType::BVH) {
    return bvh_->Intersect(ray, t_min, record);
  } else if (idx_type_ == IndexerType::Octree) {
    return octree_->Intersect(ray, t_min, record);
  } else {
    throw std::runtime_error("[Mesh] intersect doesn't recognize indexer type...");
  }
}
}  // namespace GLOO
