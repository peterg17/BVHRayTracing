#ifndef BVH_H_
#define BVH_H_

#include <memory>
#include <glm/glm.hpp>
#include "HitRecord.hpp"
#include "hittable/HittableBase.hpp"
#include "AABB.hpp"
#include "hittable/Triangle.hpp"
#include "hittable/Mesh.hpp"
#include "hittable/Plane.hpp"
#include "hittable/Sphere.hpp"

namespace GLOO{

enum class SplittingHeuristic {
    None,
    SAH,    
};

class BVH {
  
  public:
    BVH(SplittingHeuristic splitType, int max_level=10) {
      split_type_ = splitType;
      max_level = max_level;
    }
    // void Build(std::vector<HittableBase*> hitObjects);
    void Build(const Mesh& mesh);
    bool Intersect(const Ray& ray, float t_min, HitRecord& record);

  private:
    struct BVHNode {
      bool IsTerminal() const {
        // not sure if this makes sense, because 
        // couldn't one of them be a nullptr but the other keeps going?
        return (left == nullptr && right == nullptr);
      }

      // void SetBvol(AABB bvol) const {
      //   bvol_ = bvol;
      // }

      // AABB GetBvol() const {
      //   return bvol_;
      // }

      std::unique_ptr<BVHNode> left;
      std::unique_ptr<BVHNode> right;
      // std::vector<HittableBase*> objects;
      // const HittableBase* object;
      // std::vector<const Triangle*> triangles;
      const Triangle* triangle;
      AABB bvol_;
    };

    // can we separate triangles from mesh object?
    // like intersect them without having to keep track of where they come from?
    void BuildNode(BVHNode& node,
                   const AABB& bbox,
                  //  std::vector<HittableBase*> objects,
                   const std::vector<const Triangle*>& triangles,
                   int axis,
                   int level);
    
    bool IntersectSubVol(const BVHNode& node,
                        const Ray& ray,
                        float t_min,
                        HitRecord& record,
                        int level);


    int max_level_;
    std::unique_ptr<BVHNode> root_;
    int axis_;
    float divs_ [3];
    SplittingHeuristic split_type_;
};
} // namespace GLOO

#endif