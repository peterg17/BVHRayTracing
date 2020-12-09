#include "Illuminator.hpp"

#include <limits>
#include <stdexcept>

#include <glm/geometric.hpp>
#include <glm/gtx/string_cast.hpp>


#include "gloo/lights/DirectionalLight.hpp"
#include "gloo/lights/PointLight.hpp"
#include "gloo/SceneNode.hpp"

namespace GLOO {
void Illuminator::GetIllumination(const LightComponent& light_component,
                                  const glm::vec3& hit_pos,
                                  glm::vec3& dir_to_light,
                                  glm::vec3& intensity,
                                  float& dist_to_light) {
  // Calculation will be done in world space.

  auto light_ptr = light_component.GetLightPtr();
  if (light_ptr->GetType() == LightType::Directional) {
    auto directional_light_ptr = static_cast<DirectionalLight*>(light_ptr);
    dir_to_light = -directional_light_ptr->GetDirection();
    intensity = directional_light_ptr->GetDiffuseColor();
    dist_to_light = std::numeric_limits<float>::max();
  } else if (light_ptr->GetType() == LightType::Point) {
    auto point_light_ptr = static_cast<PointLight*>(light_ptr);
    glm::vec3 light_pos = light_component.GetNodePtr()->GetTransform().GetPosition();
    glm::vec3 pos_to_light = light_pos - hit_pos;
    dir_to_light = glm::normalize(pos_to_light);
    // float distance = sqrtf(glm::dot(pos_to_light, pos_to_light));
    float distance = glm::distance(light_pos, hit_pos);
    dist_to_light = distance;
    glm::vec3 denom = point_light_ptr->GetAttenuation() * distance * distance;
    // if (denom[0] > 0.0 || denom[1] > 0.0 || denom[2] || 0.0) {
    //   throw std::runtime_error("Diving by zero!");
    // }
    intensity = point_light_ptr->GetDiffuseColor() / denom;
  } else {  // TODO: Implement point light.
    throw std::runtime_error(
        "Unrecognized light type when computing "
        "illumination");
  }
}
}  // namespace GLOO
