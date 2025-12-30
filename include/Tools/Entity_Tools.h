#pragma once

#include <vec3.hpp>


namespace Shardis
{

    glm::vec3 calculate_impulse(const glm::vec3& _initial_impulse, const glm::vec3& _additional_impulse, float _max_impulse, float _acceleration_multiplier);

    float calculate_rotation_angle_around_y(const glm::vec3& _front_vector);

}
