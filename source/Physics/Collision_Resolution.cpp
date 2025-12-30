#include <Physics/Collision_Resolution.h>

#include <Object.h>

#include <Modules/Physics_Module__Mesh.h>

#include <Modules/Type_Module.h>
#include <Modules/Health_Module.h>
#include <Modules/Attack_Module.h>

using namespace Shardis;

namespace Shardis
{
    constexpr float Grounded_Dot_Threshold = 0.65f;
    constexpr float Small_Movements_Tolerance = 0.01f;
}


Collision_Resolution::Collision_Type Collision_Resolution::M_calculate_collision_type(const LPhys::Intersection_Data& _id) const
{
    LEti::Object* parent_1 = _id.first->parent_object();
    LEti::Object* parent_2 = _id.second->parent_object();

    const Type_Module* type_module_1 = parent_1->get_module_of_type<Type_Module>();
    const Type_Module* type_module_2 = parent_2->get_module_of_type<Type_Module>();

    if(!type_module_1 || !type_module_2)
        return Collision_Type::Unknown;

    Object_Type type_1 = type_module_1->object_type();
    Object_Type type_2 = type_module_2->object_type();

    if(type_1 == Object_Type::Terrain && type_2 == Object_Type::Enemy)
        return Collision_Type::Entity_Vs_Terrain;
    if(type_1 == Object_Type::Enemy && type_2 == Object_Type::Terrain)
        return Collision_Type::Entity_Vs_Terrain;

    if(type_1 == Object_Type::Terrain && type_2 == Object_Type::Player)
        return Collision_Type::Entity_Vs_Terrain;
    if(type_1 == Object_Type::Player && type_2 == Object_Type::Terrain)
        return Collision_Type::Entity_Vs_Terrain;

    if(type_1 == Object_Type::Enemy && type_2 == Object_Type::Enemy)
        return Collision_Type::Entity_Vs_Entity;

    if(type_1 == Object_Type::Enemy && type_2 == Object_Type::Player)
        return Collision_Type::Entity_Vs_Entity;
    if(type_1 == Object_Type::Player && type_2 == Object_Type::Enemy)
        return Collision_Type::Entity_Vs_Entity;

    if(type_1 == Object_Type::Enemy && type_2 == Object_Type::Player_Attack)
        return Collision_Type::Attack_Vs_Entity;
    if(type_1 == Object_Type::Player_Attack && type_2 == Object_Type::Enemy)
        return Collision_Type::Attack_Vs_Entity;

    if(type_1 == Object_Type::Player && type_2 == Object_Type::Enemy_Attack)
        return Collision_Type::Attack_Vs_Entity;
    if(type_1 == Object_Type::Enemy_Attack && type_2 == Object_Type::Player)
        return Collision_Type::Attack_Vs_Entity;

    return Collision_Type::Unknown;
}



float Collision_Resolution::M_calculate_traction_ratio(const glm::vec3& _impulse, const glm::vec3& _normal)
{
    if(LEti::Math::vector_length_squared(_normal) < 1e-6f)
        return -1.0f;

    glm::vec3 impulse_direction = _impulse;
    LEti::Math::shrink_vector_to_1(impulse_direction);

    if(LEti::Math::vector_length_squared(impulse_direction) < 1e-6f)
        return -1.0f;

    float traction_ratio = -LEti::Math::dot_product(impulse_direction, _normal);
    if(traction_ratio < 0.0f)
        return -1.0f;

    constexpr float Traction_Ratio_Multiplier = 0.7f;
    float result = 1.0f - (traction_ratio * Traction_Ratio_Multiplier);
    if(result < 0.0f)
        result = 0.0f;

    return result;
}

void Collision_Resolution::M_apply_traction(Velocity_Module* _velocity_module, const glm::vec3& _normal, float _traction_ratio, float _dt)
{
    L_ASSERT(_velocity_module);
    L_ASSERT(_traction_ratio >= 0.0f);
    L_ASSERT(_traction_ratio <= 1.0f);

    const glm::vec3& impulse = _velocity_module->impulse();

    float projection = LEti::Math::dot_product(impulse, _normal);

    glm::vec3 new_impulse = impulse + (_normal * -projection);
    float new_impulse_initial_force = LEti::Math::vector_length(impulse);
    float new_impulse_initial_force_inversed = 0.0f;
    if(new_impulse_initial_force > 0.0f)
        new_impulse_initial_force_inversed = 1.0f / new_impulse_initial_force;

    float traction = 1.0f - ((1.0f - _traction_ratio) * _velocity_module->traction_multiplier());
    float impulse_force = new_impulse_initial_force * traction;

    new_impulse = (new_impulse * new_impulse_initial_force_inversed) * impulse_force;

    _velocity_module->set_impulse(new_impulse);
}



bool Collision_Resolution::M_resolve_dynamic_vs_dynamic(const LPhys::Intersection_Data &_id, float _dt)
{
    L_ASSERT(LV::cast_variable<LPhys::Physics_Module__Mesh>(_id.first));
    L_ASSERT(LV::cast_variable<LPhys::Physics_Module__Mesh>(_id.second));

    LPhys::Physics_Module__Mesh* pm1 = (LPhys::Physics_Module__Mesh*)_id.first;
    LPhys::Physics_Module__Mesh* pm2 = (LPhys::Physics_Module__Mesh*)_id.second;

    LEti::Transformation_Data transformation_data_after_collision_1 = *pm1->transformation_data();
    transformation_data_after_collision_1.set_position( LEti::Transformation_Data::get_position_for_ratio(*pm1->transformation_data_prev_state(), *pm1->transformation_data(), _id.time_of_intersection_ratio) );
    LEti::Transformation_Data transformation_data_after_collision_2 = *pm2->transformation_data();
    transformation_data_after_collision_2.set_position( LEti::Transformation_Data::get_position_for_ratio(*pm2->transformation_data_prev_state(), *pm2->transformation_data(), _id.time_of_intersection_ratio) );

    glm::vec3 separation_vec = _id.normal * (_id.depth + m_additional_offset_between_objects);
    transformation_data_after_collision_1.move(  (separation_vec * 0.5f) );
    transformation_data_after_collision_2.move( -(separation_vec * 0.5f) );

    float time_after_intersection_ratio = 1.0f - _id.time_of_intersection_ratio;

    {
        Velocity_Module* velocity_module = pm1->parent_object()->get_module_of_type<Velocity_Module>();

        float traction_ratio = 0.0f;
        if(velocity_module)
        {
            const glm::vec3& impulse = velocity_module->impulse();
            traction_ratio = M_calculate_traction_ratio(impulse, _id.normal);

            if(traction_ratio >= 0.0f)
                M_apply_traction(velocity_module, _id.normal, traction_ratio, _dt);
        }
    }
    {
        Velocity_Module* velocity_module = pm2->parent_object()->get_module_of_type<Velocity_Module>();

        float traction_ratio = 0.0f;
        if(velocity_module)
        {
            const glm::vec3& impulse = velocity_module->impulse();
            traction_ratio = M_calculate_traction_ratio(impulse, -_id.normal);

            if(traction_ratio >= 0.0f)
                M_apply_traction(velocity_module, _id.normal, traction_ratio, _dt);
        }
    }

    pm1->add_transformation_after_collision(transformation_data_after_collision_1);
    pm2->add_transformation_after_collision(transformation_data_after_collision_2);

    return true;
}

bool Collision_Resolution::M_resolve_dynamic_vs_static(const LPhys::Intersection_Data &_id, float _dt)
{
    L_ASSERT(_id.first->is_static() ^ _id.second->is_static());

    L_ASSERT(LV::cast_variable<LPhys::Physics_Module__Mesh>(_id.first));
    L_ASSERT(LV::cast_variable<LPhys::Physics_Module__Mesh>(_id.second));

    LPhys::Physics_Module__Mesh* pm1 = (LPhys::Physics_Module__Mesh*)_id.first;
    LPhys::Physics_Module__Mesh* pm2 = (LPhys::Physics_Module__Mesh*)_id.second;

    glm::vec3 normal = _id.normal;

    if(pm1->is_static())
    {
        LPhys::Physics_Module__Mesh* temp = pm1;
        pm1 = pm2;
        pm2 = temp;

        normal *= -1.0f;
    }

    constexpr glm::vec3 Up_Vector = {0.0f, 1.0f, 0.0f};
    float vertical_dot = LEti::Math::dot_product(normal, Up_Vector);
    bool grounded = vertical_dot > Grounded_Dot_Threshold;

    LEti::Transformation_Data transformation_data_after_collision_1 = *pm1->transformation_data();
    transformation_data_after_collision_1.set_position( LEti::Transformation_Data::get_position_for_ratio(*pm1->transformation_data_prev_state(), *pm1->transformation_data(), _id.time_of_intersection_ratio) );

    glm::vec3 separation_vec = normal * (_id.depth + m_additional_offset_between_objects);
    transformation_data_after_collision_1.move(separation_vec);

    float time_after_intersection_ratio = 1.0f - _id.time_of_intersection_ratio;

    glm::vec3 stride_vec = pm1->transformation_data()->position() - pm1->transformation_data_prev_state()->position();
    stride_vec *= time_after_intersection_ratio;
    float stride_on_normal_projection = -LEti::Math::dot_product(stride_vec, normal);
    stride_vec += normal * stride_on_normal_projection;

    Velocity_Module* velocity_module = pm1->parent_object()->get_module_of_type<Velocity_Module>();

    float traction_ratio = 0.0f;
    if(velocity_module)
    {
        const glm::vec3& impulse = velocity_module->impulse();
        traction_ratio = M_calculate_traction_ratio(impulse, _id.normal);
        if(!grounded)
            traction_ratio = 1.0f;

        if(traction_ratio >= 0.0f)
            M_apply_traction(velocity_module, _id.normal, traction_ratio, _dt);
    }

    transformation_data_after_collision_1.move(stride_vec);

    glm::vec3 total_stride = transformation_data_after_collision_1.position() - pm1->transformation_data_prev_state()->position();
    if(LEti::Math::vector_length_squared(total_stride) < Small_Movements_Tolerance)
        transformation_data_after_collision_1.set_position(pm1->transformation_data_prev_state()->position());

    pm1->add_transformation_after_collision(transformation_data_after_collision_1);

    if(grounded)
        velocity_module->mark_grounded(true);

    return true;
}

bool Collision_Resolution::M_resolve_attack_vs_entity(const LPhys::Intersection_Data& _id, float _dt)
{
    LEti::Object* attack = _id.first->parent_object();
    LEti::Object* entity = _id.second->parent_object();

    {
        const Type_Module* type_module_1 = attack->get_module_of_type<Type_Module>();
        L_ASSERT(type_module_1);

        Object_Type type_1 = type_module_1->object_type();

        if(type_1 == Object_Type::Enemy || type_1 == Object_Type::Player)
            std::swap(attack, entity);
    }

    Attack_Module* attack_module = attack->get_module_of_type<Attack_Module>();
    Health_Module* health_module = entity->get_module_of_type<Health_Module>();
    Velocity_Module* velocity_module = entity->get_module_of_type<Velocity_Module>();

    L_ASSERT(attack_module);
    L_ASSERT(health_module);
    L_ASSERT(velocity_module);

    if(health_module->is_invulnerable_to(attack))
        return true;

    health_module->receive_damage(attack_module->damage(), attack_module->stagger(), attack_module->owner(), _id.point);
    health_module->add_invulnerability(attack_module->damage_frequency(), attack);

    glm::vec3 knockback = entity->current_state().position() - attack->current_state().position();
    knockback.y = 0.0f;
    LEti::Math::extend_vector_to_length(knockback, attack_module->knockback());

    velocity_module->add_impulse(knockback);

    return true;
}



bool Collision_Resolution::resolve(const LPhys::Intersection_Data& _id, float _dt)
{
    Collision_Type collision_type = M_calculate_collision_type(_id);

    if(collision_type == Collision_Type::Entity_Vs_Entity)
        return M_resolve_dynamic_vs_dynamic(_id, _dt);

    if(collision_type == Collision_Type::Entity_Vs_Terrain)
        return M_resolve_dynamic_vs_static(_id, _dt);

    if(collision_type == Collision_Type::Attack_Vs_Entity)
        return M_resolve_attack_vs_entity(_id, _dt);

    return false;
}
