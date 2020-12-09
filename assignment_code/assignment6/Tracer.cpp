#include "Tracer.hpp"

#include <glm/gtx/string_cast.hpp>
#include <stdexcept>
#include <algorithm>

#include "gloo/Transform.hpp"
#include "gloo/components/MaterialComponent.hpp"
#include "gloo/lights/AmbientLight.hpp"

#include "gloo/Image.hpp"
#include "Illuminator.hpp"
#include "Ray.hpp"

namespace GLOO
{
  void Tracer::Render(const Scene &scene,
                      const std::string &output_file)
  {
    scene_ptr_ = &scene;

    auto &root = scene_ptr_->GetRootNode();
    scene_node_ = &root;
    tracing_components_ = root.GetComponentPtrsInChildren<TracingComponent>();
    light_components_ = root.GetComponentPtrsInChildren<LightComponent>();

    Image image(image_size_.x, image_size_.y);

    for (size_t y = 0; y < image_size_.y; y++)
    {
      for (size_t x = 0; x < image_size_.x; x++)
      {
        // TODO: For each pixel, cast a ray, and update its value in the image.
        // cast ray using camera
        float x_scaled = -1.0f + (float)((float)x / (image_size_.x)) * 2.0f;
        float y_scaled = -1.0f + (float)((float)y / (image_size_.y)) * 2.0f;
        glm::vec2 point(x_scaled, y_scaled);
        Ray ray = camera_.GenerateRay(point);
        HitRecord hr = HitRecord();
        glm::vec3 ray_color = TraceRay(ray, max_bounces_, hr, shadows_enabled_);
        image.SetPixel(x, y, ray_color);
      }
    }

    // For debugging purposes...
    // glm::vec2 point(0.0, 0.0);
    // Ray ray = camera_.GenerateRay(point);
    // Ray ray = Ray(glm::vec3(0.0,0.0,0.0), glm::vec3(-1.0,-1.0,-1.0));
    // HitRecord hr = HitRecord();
    // glm::vec3 ray_color = TraceRay(ray, max_bounces_, hr, shadows_enabled_);

    if (output_file.size())
      image.SavePNG(output_file);
  }

  glm::vec3 Tracer::TraceRay(const Ray &ray,
                             size_t bounces,
                             HitRecord &record, bool shadows) const
  {
    // std::cout << "[Tracer] ray at origin: " << glm::to_string(ray.GetOrigin());
    // std::cout << " and direction: " << glm::to_string(ray.GetDirection()) << std::endl;
    // std::cout << "Intersects at time: " << record.time << std::endl;
    glm::vec3 total_intensity = GetBackgroundColor(ray.GetDirection());
    // TODO: Compute the color for the cast ray.
    for (int i = 0; i < tracing_components_.size(); i++)
    {
      TracingComponent *tc = tracing_components_[i];
      glm::mat4x4 T = tc->GetNodePtr()->GetTransform().GetLocalToWorldMatrix();
      glm::mat4x4 T_inv = glm::inverse(T);
      Ray new_ray = Ray(ray.GetOrigin(), ray.GetDirection());
      new_ray.ApplyTransform(T_inv);

      bool res;
      // if (tc->GetHittable().IsMesh()) {
      //   res = tc->GetHittable().Intersect(new_ray, 0.0, record);
      // } else {
      //   res = scene_node_->GetBVH().Intersect(new_ray, 0.0, record);
      // }
      res = tc->GetHittable().Intersect(new_ray, 0.0, record);
      if (res)
      { 
        // std::cout << "[Tracer] ray at origin: " << glm::to_string(new_ray.GetOrigin());
        // std::cout << " and direction: " << glm::to_string(new_ray.GetDirection()) << std::endl;
        // std::cout << "Intersects at time: " << record.time << std::endl;
        total_intensity = glm::vec3(0.0,0.0,0.0);
        glm::vec3 old_normal = record.normal;
        glm::vec4 new_normal = glm::transpose(T_inv) * glm::vec4(record.normal, 1.0);
        // record.normal = glm::normalize(glm::vec3(new_normal / new_normal.w));
        record.normal = glm::normalize(glm::vec3(new_normal[0], new_normal[1], new_normal[2]));
        new_ray.ApplyTransform(T);
        
        Material material_color = tc->GetNodePtr()->GetComponentPtr<MaterialComponent>()->GetMaterial();
        float shininess = material_color.GetShininess();
        glm::vec3 k_specular = material_color.GetSpecularColor();
        glm::vec3 k_diffuse = material_color.GetDiffuseColor();
        glm::vec3 k_ambient = material_color.GetAmbientColor();

        glm::vec3 dir_to_light = glm::vec3(0.0,0.0,0.0);
        glm::vec3& dir_light_ref = dir_to_light;
        glm::vec3 intensity = glm::vec3(0.0,0.0,0.0);
        glm::vec3& intensity_ref = intensity;
        float dist_to_light = 0.0f;
        float& dist_ref = dist_to_light;
        const glm::vec3 hit_pos = new_ray.At(record.time);
        const glm::vec3& hit_ref = hit_pos;
        glm::vec3 reflected_eye_vector = glm::normalize(new_ray.GetDirection() - 2.0f * glm::dot(record.normal, new_ray.GetDirection()) * record.normal); 
        // glm::vec3 eye_vector = new_ray.GetOrigin() - hit_pos;
        // glm::vec3 eye_vector = new_ray.GetDirection() * glm::vec3(-1.0,-1.0,-1.0);
        // glm::vec4 new_ev = glm::transpose(T_inv) * glm::vec4(eye_vector, 1.0);
        // eye_vector = glm::normalize(glm::vec3(new_ev[0], new_ev[1], new_ev[2]));
        for (int j=0; j < light_components_.size(); j++) {
          const LightComponent *lc = light_components_[j];
          auto light_type = lc->GetLightPtr()->GetType();
          glm::vec3 other_light = glm::vec3(0.0,0.0,0.0);
          // calculate Clamp(L,N) where L is direectional vector to light and N is the normal vector
          // note: L bolded in pset is the directional vector to light whereas L not bolded is the intensity vector
          if (light_type == LightType::Directional || light_type == LightType::Point) {
            // diffuse shading
            Illuminator::GetIllumination(*lc, hit_ref, dir_light_ref, intensity_ref, dist_ref);
            float clamped_value = 0.0f;
            float clamped_spec_value = 0.0f;
            // Ray reflected_eye_ray = Ray(hit_pos, glm::normalize(record.normal));
            // glm::vec3 reflected_eye_vector = reflected_eye_ray.GetDirection();
            if (glm::dot(dir_to_light, record.normal) > 0.0) {
              clamped_value = glm::dot(dir_to_light, record.normal);
            }
            if (glm::dot(dir_to_light, reflected_eye_vector) > 0.0) {
              clamped_spec_value = glm::dot(dir_to_light, reflected_eye_vector);
            }
            // std::cout << "intensity of point/directional light is: " << glm::to_string(intensity) << std::endl;
            glm::vec3 diffuse_intensity = clamped_value * intensity * k_diffuse;
            glm::vec3 specular_intensity = powf(clamped_spec_value, shininess) * intensity * k_specular;
            // if (k_specular[0] > 0.0) {
            //   std::cout << "clamped spec value is: " << clamped_spec_value << std::endl;
            //   std::cout << "k specular: " << glm::to_string(k_specular) << std::endl;
            //   std::cout << "intensity: " << glm::to_string(intensity) << std::endl;
            //   std::cout << "total specular intensity is: " << glm::to_string(specular_intensity) << std::endl;
            // }
            other_light += diffuse_intensity + specular_intensity;
          } else if (light_type == LightType::Ambient) {
            // AmbientLight *ambient = static_cast<AmbientLight*>(lc->GetLightPtr());
            auto ambient = (AmbientLight*)light_components_[j]->GetLightPtr();
            glm::vec3 ambient_intensity = ambient->GetAmbientColor() * k_diffuse;
            // total_intensity = total_intensity + ambient_intensity;
            other_light += ambient_intensity;
          }
          // shadows
          // send a ray out to each light source, if something blocking there is shadow
          // std::cout << "Just before calling shadows" << std::endl;
          if (shadows) {
              float epsilon = 0.05;
              glm::vec3 shadow_origin = hit_pos + epsilon*dir_to_light;
              Ray shadow_ray = Ray(shadow_origin, dir_to_light);
              shadow_ray.ApplyTransform(T);
              HitRecord hr = HitRecord();
              // bool res = tc->GetHittable().Intersect(shadow_ray, 0.0, hr);
              glm::vec3 shadow_trace = TraceRay(shadow_ray, 0, hr, false);
              bool did_intersect = hr.time < std::numeric_limits<float>::max();
              // bool did_intersect = hr.time < 10.0;
              // glm::vec3 shadow_vec_diff = shadow_ray.At(hr.time) - shadow_origin;
              if (did_intersect) {
                float shadow_dist = glm::distance(shadow_ray.At(hr.time), shadow_origin);
                // std::cout << "Shadow dist is: " << shadow_dist << " and dist to light is: " << dist_to_light << std::endl;
                if (shadow_dist < dist_to_light) {
                  other_light = glm::vec3(0.0,0.0,0.0);
                }
              }
            // }
          }
          total_intensity += other_light;
        }
        

        // reflections
        if (bounces > 0) {
          glm::vec3 N = record.normal;
          HitRecord hr = HitRecord();
          float epsilon = 0.01;
          Ray reflect_ray = Ray(ray.At(record.time) + epsilon*reflected_eye_vector, reflected_eye_vector);
          glm::vec3 indirect_intensity = TraceRay(reflect_ray, bounces - 1, hr, shadows_enabled_);
          total_intensity += k_specular * indirect_intensity;
        }
      }
    }
    return total_intensity;
  }

  glm::vec3 Tracer::GetBackgroundColor(const glm::vec3 &direction) const
  {
    if (cube_map_ != nullptr)
    {
      return cube_map_->GetTexel(direction);
    }
    else
      return background_color_;
  }
} // namespace GLOO
