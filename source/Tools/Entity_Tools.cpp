#include <Tools/Entity_Tools.h>

#include <Math_Stuff.h>

using namespace Shardis;


glm::vec3 Shardis::calculate_impulse(const glm::vec3& _initial_impulse, const glm::vec3& _additional_impulse, float _max_impulse, float _acceleration_multiplier)
{
    float impulse_length = LEti::Math::vector_length(_initial_impulse);

    glm::vec3 result = _initial_impulse + (_additional_impulse * _acceleration_multiplier);
    float new_impulse_length = LEti::Math::vector_length(result);

    if(new_impulse_length > _max_impulse && new_impulse_length > impulse_length)
        result *= impulse_length / new_impulse_length;

    return result;
}

float Shardis::calculate_rotation_angle_around_y(const glm::vec3& _front_vector)
{
    constexpr glm::vec3 Reference_Direction = {1.0f, 0.0f, 0.0f};

    glm::vec3 front = _front_vector;
    if( !LEti::Math::floats_are_equal(front.y, 0.0f) )
    {
        front.y = 0.0f;
        LEti::Math::shrink_vector_to_1(front);
    }

    float angle_cos = LEti::Math::angle_cos_between_vectors(_front_vector, Reference_Direction);
    float angle = acos(angle_cos);
    if(_front_vector.z > 0.0f)
        angle = LEti::Math::DOUBLE_PI - angle;

    return angle;
}
