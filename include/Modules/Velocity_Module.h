#pragma once

#include <Stuff/Timer.h>

#include <Misc_Modules/Directional_Movement_Module.h>


namespace Shardis
{

    class Velocity_Module : public LMD::Directional_Movement_Module
    {
    public:
        INIT_VARIABLE(Shardis::Velocity_Module, LMD::Directional_Movement_Module)

    private:
        glm::vec3 m_impulse = { 0.0f, 0.0f, 0.0f };

        float m_max_impulse = 0.0f;
        float m_max_impulse_squared = 0.0f;

        float m_traction_multiplier = 1.0f;

        LST::Timer m_grounded_timer;

    public:
        inline void set_impulse(const glm::vec3& _value) { m_impulse = _value; }
        inline void add_impulse(const glm::vec3& _value) { m_impulse += _value; }
        inline void set_max_impulse(float _value) { m_max_impulse = _value; m_max_impulse_squared = _value * _value; }
        inline void set_traction_multiplier(float _value) { m_traction_multiplier = _value; }

        inline const glm::vec3& impulse() const { return m_impulse; }
        inline float max_impulse() const { return m_max_impulse; }
        inline float traction_multiplier() const { return m_traction_multiplier; }

        inline bool is_grounded() const { return m_grounded_timer.is_active(); }

    public:
        void mark_grounded(bool _grounded);

    public:
        void update(float _dt) override;

    };


    class Velocity_Module_Stub : public LMD::Directional_Movement_Module_Stub
    {
    public:
        INIT_VARIABLE(Shardis::Velocity_Module_Stub, LMD::Directional_Movement_Module_Stub)

        INIT_FIELDS
        ADD_FIELD(float, max_impulse)
        FIELDS_END

    public:
        float max_impulse = 0.0f;

    public:
        INIT_BUILDER_STUB(Velocity_Module)

    };

}
