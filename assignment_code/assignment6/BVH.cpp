#include "BVH.hpp"

#include <algorithm>
#include "cassert"
#include <iostream>
#include "gloo/utils.hpp"
#include "hittable/HittableBase.hpp"
#include "hittable/Mesh.hpp"
#include "hittable/Plane.hpp"
#include "hittable/Sphere.hpp"
#include "hittable/Triangle.hpp"

using namespace std;

namespace {

// this is kind of arbitrary for now
static const int objectCapacityPerLevel = 6;
int axis = 0;

// helper function for bbox/bvol intersection
bool IntervalIntersect(float* a, float* b) {
  if (a[0] > b[1]) {
    return a[0] <= b[1];
  } else {
    return b[0] <= a[1];
  }
}

}

namespace GLOO {
AABB FromTriangle(const Triangle& triangle) {
  AABB bbox;
  bbox.mn = bbox.mx = triangle.GetPosition(0);
  for (int i = 1; i < 3; i++) {
    for (int dim = 0; dim < 3; dim++) {
      bbox.mn[dim] = std::min(bbox.mn[dim], triangle.GetPosition(i)[dim]);
      bbox.mx[dim] = std::max(bbox.mx[dim], triangle.GetPosition(i)[dim]);
    }
  }
  return bbox;
}

AABB FromMesh(const Mesh& mesh) {
  auto& triangles = mesh.GetTriangles();
  AABB bbox(FromTriangle(triangles[0]));
  for (size_t i = 1; i < triangles.size(); i++) {
    AABB objBbox = FromTriangle(triangles[i]);
    // std::cout << "[BVH] in build, bbox " << i << std::endl;
    // objBbox.PrintBox();
    bbox.UnionWith(objBbox);
  }
  return bbox;
}

bool CompareHittable(const Triangle* h1, const Triangle* h2) {
  AABB bbox1 = h1->CreateBoundingBox();
  AABB bbox2 = h2->CreateBoundingBox();
  // glm::vec3 center1 = bbox1.GetCenter();
  // glm::vec3 center2 = bbox2.GetCenter();
  // Unclear if this is exactly what we want, has to be lt in all components
  // https://stackoverflow.com/questions/46636721/how-do-i-use-glm-vector-relational-functions
  // return (glm::all(glm::lessThan(center1, center2)));
  // return 
  return (bbox1.mx[axis] < bbox2.mx[axis]);
}

float CalcSurfaceArea(AABB vol) {
  float width = abs(vol.mx[0] - vol.mn[0]);
  float height = abs(vol.mx[1] - vol.mn[1]);
  float depth = abs(vol.mx[2] - vol.mn[2]);
  float SA = 2 * (width*depth + width*height + depth*height);
  return SA;
}

// assume that the triangle vector is pre-sorted by center
int SplitAxis(std::vector<const Triangle*> triangles, int axis, AABB vol) {
  // assert(index >= 1 && index <= (triangles.size()-2));
  assert(triangles.size() > 2);
  float minCost = std::numeric_limits<float>::max();
  float minIdx = 1;
  float SA_V = CalcSurfaceArea(vol);
  for (int idx=1; idx < (triangles.size() - 1); idx++) {
    // evaluate a split at idx
    std::vector<const Triangle*> left;
    std::vector<const Triangle*> right;
    float SA_VL;
    float SA_VR;
    float N_L;
    float N_R;
    // calculate left volume bounds
    bool init = false;
    AABB leftVol;
    for (int i=0; i < idx; i++) {
      auto tri = triangles[i];
      AABB bbox = FromTriangle(*triangles[i]);
      if (init == false) {
        leftVol = bbox;
        init = true;
      } else {
        leftVol.UnionWith(bbox);
      }
      left.push_back(tri);
    }

    // calculate right volume bounds
    init = false;
    AABB rightVol;
    for (int i=idx; i < triangles.size(); i++) {
      auto tri = triangles[i];
      AABB bbox = FromTriangle(*triangles[i]);
      if (init == false) {
        rightVol = bbox;
        init = true;
      } else {
        rightVol.UnionWith(bbox);
      }
      right.push_back(tri);
    }

    SA_VL = CalcSurfaceArea(leftVol);
    SA_VR = CalcSurfaceArea(rightVol);
    N_L = left.size();
    N_R = right.size();
    float P_VL = SA_VL / SA_V;
    float P_VR = SA_VR / SA_V;
    float splitCost = 1 + ((2 * P_VL * N_L) + (2 * P_VR * N_R));
    if (splitCost < minCost) {
      minCost = splitCost;
      minIdx = idx;
    }

  }
  // std::cout << "returning split with split idx of:  " << minIdx << std::endl; 
  assert(minIdx >= 1 && minIdx <= (triangles.size() - 2));
  return minIdx;
}


bool HitBox(AABB bbox, const Ray& ray, float t_min, float t_start, float t_end) {
  // return true;
  glm::vec3 ray_dir = ray.GetDirection();
  glm::vec3 ray_origin = ray.GetOrigin();

  if (t_start > t_end) {
    // std::cout << "[BVH] intersectSubVol returns false b/c of ray missing box,  t_start: " << t_start << " and t_end: " << t_end << std::endl;
    return false;
  } 

  if (t_end < t_min) {
    // std::cout << "[BVH] intersectSubVol returns false b/c of box behind ray" << std::endl;
    return false;
  }

  if (t_start < 0 || t_end < 0)  {
    return false;
  }

  return true;
}

void BVH::BuildNode(BVHNode& node, const AABB& bbox, 
                const std::vector<const Triangle*>& triangles, int axis, int level) {
  // std::cout << "[BVH] at level: " << level << std::endl;                  
  // bbox.PrintBox();
  if (triangles.size() == 1) {
    // node.objects = objects;
    // std::cout << "[BVH] build node reaches leaf" << std::endl;
    // node.object = objects[0];
    node.bvol_ = FromTriangle(*triangles[0]);
    // std::cout << "[BVH] constructing leaf node, bbox is:  \n";
    // node.bvol_.PrintBox();
    node.triangle = triangles[0];
    node.left = nullptr;
    node.right = nullptr;
    return;
  } else if (triangles.size() == 2) {
    node.left = make_unique<BVHNode>();
    node.right = make_unique<BVHNode>();
    AABB leftBox = FromTriangle(*triangles[0]);
    AABB rightBox = FromTriangle(*triangles[1]);
    node.left->triangle = triangles[0];
    node.left->bvol_ = leftBox;
    node.right->triangle = triangles[1];
    node.right->bvol_ = rightBox;
    AABB combined = leftBox;
    combined.UnionWith(rightBox);
    node.bvol_ = combined;
    return;
  }

  node.left = make_unique<BVHNode>();
  node.right = make_unique<BVHNode>();
  // std::vector<HittableBase*> allObjects = std::vector<HittableBase*>();
  // std::vector<HittableBase*> left = std::vector<HittableBase*>();
  // std::vector<HittableBase*> right = std::vector<HittableBase*>();
  std::vector<const Triangle*> allObjects;
  std::vector<const Triangle*> left;
  std::vector<const Triangle*> right;
  
  AABB totalVol = FromTriangle(*triangles[0]);
  // copy objects into new array that can be sorted
  for (int i=0; i < triangles.size(); i++) {
    const Triangle* tri = triangles[i];
    AABB triBox = FromTriangle(*triangles[i]);
    totalVol.UnionWith(triBox);
    allObjects.push_back(tri);
  }

  // axis_ = axis;
  // axis = (axis+1) % 3;


  // pick longest axis
  float width = abs(bbox.mx[0] - bbox.mn[0]);
  float height = abs(bbox.mx[1] - bbox.mn[1]);
  float depth = abs(bbox.mx[2] - bbox.mn[2]);
  float max_axis = std::max(std::max(width, height), depth);
  if (max_axis == width) {
    axis = 0;
  } else if (max_axis == height) {
    axis = 1;
  } else {
    axis = 2;
  }


  std::sort(allObjects.begin(), allObjects.end(), CompareHittable);
  // int splitIdx = SplitAxis(allObjects, axis, totalVol);

  int splitIdx;
  if (split_type_ == SplittingHeuristic::SAH) {
    splitIdx = SplitAxis(allObjects, axis, totalVol);
  } else {
    splitIdx = allObjects.size() / 2;
  }
  
  // int halfIdx = allObjects.size() / 2;
  int halfIdx = splitIdx;

  // make left bbox
  // bool init = false;
  AABB leftVol(FromTriangle(*allObjects[0]));
  for (int i=0; i < halfIdx; i++) {
    auto& obj = allObjects[i];
    left.push_back(obj);
    // AABB objBbox = obj->CreateBoundingBox();
    AABB objBbox = FromTriangle(*obj);
    leftVol.UnionWith(objBbox);
  }

  AABB rightVol(FromTriangle(*allObjects[halfIdx]));
  // make right bbox
  // init = false;
  for (int i=halfIdx; i < allObjects.size(); i++) {
    auto& obj = allObjects[i];
    right.push_back(obj);
    AABB objBbox = FromTriangle(*obj);
    // AABB objBbox = obj->CreateBoundingBox();
    // if (objBbox.isInf) {
    //   // std::cout << "[BVH] indexing a plane " << std::endl;
    //   continue;
    // }
    rightVol.UnionWith(objBbox);
    // if (init == false) {
    //   rightVol = objBbox;
    //   init = true;
    // } else {
    //   rightVol.UnionWith(objBbox);
    // }
  }


  node.left->bvol_ = leftVol;
  // node.left->objects = left;
  node.right->bvol_ = rightVol;
  AABB combined = AABB(leftVol);
  combined.UnionWith(rightVol);
  node.bvol_ = combined;
  // node.right->objects = right;

  // std::cout << "[BVH] in build sub vol, left has: " << left.size() << "  objects\n"; 
  // std::cout << "[BVH] in build sub vol, and right has: " << right.size() << "  objects\n"; 
  BuildNode(*(node.left), leftVol, left, axis + 1, level + 1);
  BuildNode(*(node.right), rightVol, right, axis + 1, level + 1);
}

void BVH::Build(const Mesh& mesh) {
  // std::cout << "[BVH] in build, size of hitObjects is: " << hitObjects.size() << std::endl;
  // std::cout << "[BVH] in Build function" << std::endl;

  auto& triangles = mesh.GetTriangles();
  AABB bbox = FromMesh(mesh);


  std::vector<const Triangle*> triangle_ptrs;
  for (size_t i = 0; i < triangles.size(); i++)
    triangle_ptrs.push_back(&triangles[i]);

  root_ = make_unique<BVHNode>();
  root_->bvol_ = bbox;
  BuildNode(*root_, bbox, triangle_ptrs, 0, 0);
  // std::cout << "[BVH] finished building!"  << std::endl;
}


bool BVH::IntersectSubVol(const BVHNode& node, const Ray& ray,
                          float t_min, HitRecord& record, int level) {
  // bool intersected = false;

  // // if (level > 0)
  AABB bvol = node.bvol_;
  // std::cout << "[BVH] in IntersectSubVol, level: " << level << std::endl;
  // bvol.PrintBox();


  if (node.IsTerminal()) {
    // for (auto& o : node.objects) {
    //   bool result = o->Intersect(ray, t_min, record);
    //   intersected |= result;
    // }
    // std::cout << "[BVH] intersect gets to leaf node!" << std::endl;
    bool result = node.triangle->Intersect(ray, t_min, record);
    // intersected |= result;
    // return intersected;
    return result;
  }
  
  // do all of the checks for misses
  
  glm::vec3 ray_dir = ray.GetDirection();
  glm::vec3 ray_origin = ray.GetOrigin();

  // parallel ray test
  for (int dim=0; dim < 3; dim++) {
    if (ray_dir[dim] == 0) {
      float r_o = ray_origin[dim];
      if (r_o < bvol.mn[dim] || r_o > bvol.mx[dim]) {
        // std::cout << "ray is parallel!" << std::endl;
        return false;
      }
    }
  }


   

  float t_start, t_end;
  if (divs_[0] > 0) {
    t_start = (bvol.mn[0] - ray_origin[0]) * divs_[0];
    t_end = (bvol.mx[0] - ray_origin[0]) * divs_[0];
  } else {
    t_start = (bvol.mx[0] - ray_origin[0]) * divs_[0];
    t_end = (bvol.mn[0] - ray_origin[0]) * divs_[0];
  }
  float t1, t2;
  for (int i=1; i < 3; i++) {
    if (divs_[i] > 0) {
      t1 = (bvol.mn[i] - ray_origin[i]) * divs_[i];
      t2 = (bvol.mx[i] - ray_origin[i]) * divs_[i];
    } else {
      t1 = (bvol.mx[i] - ray_origin[i]) * divs_[i];
      t2 = (bvol.mn[i] - ray_origin[i]) * divs_[i];
    }
    if (t1 > t2) std::swap(t1, t2); 
    if (t1 > t_start) t_start = t1;
    if (t2 < t_end) t_end = t2;
  }
  

  // std::cout << "[BVH] in intersect subvol, t_start: " << t_start << " and t_end: " << t_end << std::endl;

  // bool intersect = false;
  // intersect |= IntersectSubVol(*(node.left), ray, t_min, record, level + 1);
  // intersect |= IntersectSubVol(*(node.left), ray, t_min, record, level + 1);
  // return intersect;

  if (HitBox(node.bvol_, ray, t_min, t_start, t_end)) {
    HitRecord lrec, rrec;
    // bool intersected = false;
    bool leftHit = (node.left != nullptr) && (IntersectSubVol(*(node.left), ray, t_min, lrec, level + 1));
    bool rightHit = (node.right != nullptr) && (IntersectSubVol(*(node.right), ray, t_min, rrec, level + 1));
    if (leftHit && rightHit) {
      if (lrec.time < rrec.time) {
        record = lrec;
      } else {
        record = rrec;
      }
      return true;
    } else if (leftHit) {
      record = lrec;
      return true;
    } else if (rightHit) {
      record = rrec;
      return true;
    } else {
      return false;
    }
  } else {
    return false;
  }
  
}

bool BVH::Intersect(const Ray& ray, float t_min, HitRecord& record) {
  AABB bvol = root_->bvol_;
  // std::vector<float> divs = std::vector<float>();
  glm::vec3 ray_dir = ray.GetDirection();
  float divx = 1.0 / (ray_dir[0] + 1e-8f);
  float divy = 1.0 / (ray_dir[1] + 1e-8f);
  float divz = 1.0 / (ray_dir[2] + 1e-8f);
  // divs.push_back(divx);
  // divs.push_back(divy);
  // divs.push_back(divz); 
  divs_[0] = divx;
  divs_[1] = divy;
  divs_[2] = divz;
  return IntersectSubVol(*root_, ray, t_min, record, 0);
}
}