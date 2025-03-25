#include <Collision_Resolution.h>

#include <Maybe_Immovable_Physics_Module.h>


void Collision_Resolution::resolve(const LPhys::Intersection_Data &_id, float _dt)
{
    Maybe_Immovable_Physics_Module* pm_1 = LV::cast_variable<Maybe_Immovable_Physics_Module>(_id.first);
    Maybe_Immovable_Physics_Module* pm_2 = LV::cast_variable<Maybe_Immovable_Physics_Module>(_id.second);

    L_ASSERT(pm_1 && pm_2);

    if(!pm_1->movable() && !pm_2->movable())
        return;

    if(pm_1->movable() && pm_2->movable())
        return LPhys::Collision_Resolution__Physics_Module_2D::resolve(_id, _dt);

    Maybe_Immovable_Physics_Module* movable = pm_1->movable() ? pm_1 : pm_2;
    Maybe_Immovable_Physics_Module* immovable = pm_1->movable() ? pm_2 : pm_1;

    glm::vec3 normal = _id.normal;
    if(movable == _id.second)
        normal *= -1.0f;

    LEti::Transformation_Data transformation_after_collision = *movable->transformation_data();
    transformation_after_collision.set_position( LEti::Transformation_Data::get_position_for_ratio(*movable->transformation_data_prev_state(), *movable->transformation_data(), _id.time_of_intersection_ratio) );

    std::cout << "normal:\nx: " << normal.x << ";   y: " << normal.y << "\n" << std::endl;

    float time_left_after_intersection = 1.0f - _id.time_of_intersection_ratio;

    glm::vec3 stride_vec = movable->transformation_data()->position() - movable->transformation_data_prev_state()->position();
    float stride_to_normal_cos = LEti::Math::angle_cos_between_vectors(stride_vec, normal);
    if( stride_to_normal_cos < 0.0f && stride_to_normal_cos > -0.9999f )      //  if entity goes into terrain
    {
        glm::vec3 rotated_normal = LEti::Math::rotate_vector(normal, {0.0f, 0.0f, 1.0f}, LEti::Math::HALF_PI);
        glm::vec3 projection = LEti::Math::calculate_projection(stride_vec * time_left_after_intersection, rotated_normal);

        float length_multiplier = LEti::Math::angle_cos_between_vectors(projection, stride_vec);
        projection *= length_multiplier;

        transformation_after_collision.move(projection);
    }

    glm::vec3 separation_vec = normal * _id.depth * 1.01f;
    transformation_after_collision.move(separation_vec);

    movable->add_transformation_after_collision(transformation_after_collision);
}
