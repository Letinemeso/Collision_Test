#pragma once

#include <Collision_Resolution/Collision_Resolution_Interface.h>

#include <Modules/Velocity_Module.h>


namespace Shardis
{

    class Collision_Resolution : public LPhys::Collision_Resolution_Interface
    {
    private:
        enum class Collision_Type
        {
            Unknown,
            Entity_Vs_Terrain,
            Entity_Vs_Entity,
            Attack_Vs_Entity
        };

    private:
        float m_additional_offset_between_objects = 0.01f;

    public:
        inline void set_additional_offset_between_objects(float _value) { m_additional_offset_between_objects = _value; }

    private:
        Collision_Type M_calculate_collision_type(const LPhys::Intersection_Data& _id) const;

        float M_calculate_traction_ratio(const glm::vec3& _impulse, const glm::vec3& _normal);
        void M_apply_traction(Velocity_Module* _velocity_module, const glm::vec3& _normal, float _traction_ratio, float _dt);

        bool M_resolve_dynamic_vs_dynamic(const LPhys::Intersection_Data& _id, float _dt);
        bool M_resolve_dynamic_vs_static(const LPhys::Intersection_Data& _id, float _dt);
        bool M_resolve_attack_vs_entity(const LPhys::Intersection_Data& _id, float _dt);

    public:
        bool resolve(const LPhys::Intersection_Data &_id, float _dt = 0.0f) override;

    };

}
