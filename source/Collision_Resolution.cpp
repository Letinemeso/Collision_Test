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

    glm::vec3 stride_vec = movable->transformation_data()->position() - movable->transformation_data_prev_state()->position();
    if( LEti::Math::angle_cos_between_vectors(stride_vec, normal) < 0.0f )      //  if entity goes into terrain
    {
        glm::vec3 stride_on_normal_projection = LEti::Math::dot_product(stride_vec, normal) * normal;
        stride_vec -= stride_on_normal_projection;
    }

    LEti::Transformation_Data transformation_after_collision = *movable->transformation_data();

    transformation_after_collision.set_position( LEti::Transformation_Data::get_position_for_ratio(*movable->transformation_data_prev_state(), *movable->transformation_data(), _id.time_of_intersection_ratio) );

    glm::vec3 separation_vec = normal * _id.depth * 1.01f;
    transformation_after_collision.move(separation_vec);

    stride_vec *= 1.0f - _id.time_of_intersection_ratio;
    transformation_after_collision.move(stride_vec);

    movable->add_transformation_after_collision(transformation_after_collision);
}
