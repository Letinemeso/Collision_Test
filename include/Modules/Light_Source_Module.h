#pragma once

#include <Stuff/Key_Frame_Animation.h>
#include <Stuff/Command_Queue.h>

#include <Module.h>

#include <Graphics/Fragment_Shader_Light_Component.h>


namespace Shardis
{

    class Light_Source_Module : public LEti::Module
    {
    public:
        INIT_VARIABLE(Shardis::Light_Source_Module, LEti::Module)

    public:
        struct Fade_Parameters
        {
            float start_max_light_distance = 0.0f;
            float target_max_light_distance = 0.0f;
            float duration = 0.0f;
            float initial_pause = 0.0f;
        };

        struct Pulse_Parameters
        {
            float min_distance_ratio = -1.0f;
            float max_distance_ratio = -1.0f;
            float duration = -1.0f;
        };

    private:
        Fragment_Shader_Light_Component* m_light_controller = nullptr;

        glm::vec3 m_light_color;
        float m_max_light_distance = 0.0f;
        glm::vec3 m_additional_offset = {0.0f, 0.0f, 0.0f};

        Pulse_Parameters m_pulse_parameters;

    private:
        Fragment_Shader_Light_Component::Light_Source_Data* m_light_source = nullptr;

        LST::Key_Frame_Animation<float> m_fade_kfa;
        LST::Command_Queue m_queue;

    public:
        Light_Source_Module();
        ~Light_Source_Module();

    public:
        inline void inject_light_controller(Fragment_Shader_Light_Component* _ptr) { m_light_controller = _ptr; }
        inline void set_additional_offset(const glm::vec3& _value) { m_additional_offset = _value; }
        inline void set_pulse_parameters(const Pulse_Parameters& _parameters) { m_pulse_parameters = _parameters; }

        inline const Pulse_Parameters& pulse_parameters() const { return m_pulse_parameters; }

        inline const glm::vec3& light_color() const { return m_light_color; }
        inline float max_light_distance() const { return m_max_light_distance; }

        void set_light_color(const glm::vec3& _color);
        void set_max_light_distance(float _max_distance);

        void start_fade(const Fade_Parameters& _parameters);
        void start_pulse(const Pulse_Parameters& _parameters);

    private:
        void M_on_parent_object_set() override;

        void M_construct_pulse_kfa(const Pulse_Parameters& _parameters);
        void M_activate_pulse_if_needed();

    public:
        void update(float _dt) override;

    };


    class Light_Source_Module_Stub : public LEti::Module_Stub
    {
    public:
        INIT_VARIABLE(Shardis::Light_Source_Module_Stub, LEti::Module_Stub)

        INIT_FIELDS
        ADD_FIELD(glm::vec3, light_color)
        ADD_FIELD(float, max_light_distance)
        ADD_FIELD(glm::vec3, additional_offset)
        ADD_FIELD(float, smooth_appearance_duration)
        ADD_FIELD(float, smooth_appearance_initial_pause)
        ADD_FIELD(float, pulse_min_distance_ratio)
        ADD_FIELD(float, pulse_max_distance_ratio)
        ADD_FIELD(float, pulse_duration)
        FIELDS_END

    public:
        glm::vec3 light_color = {1.0f, 1.0f, 1.0f};
        float max_light_distance = 0.0f;
        glm::vec3 additional_offset = {0.0f, 0.0f, 0.0f};

        float smooth_appearance_duration = -1.0f;
        float smooth_appearance_initial_pause = -1.0f;

        float pulse_min_distance_ratio = -1.0f;
        float pulse_max_distance_ratio = -1.0f;
        float pulse_duration = -1.0f;

    public:
        Fragment_Shader_Light_Component* light_controller = nullptr;

    public:
        INIT_BUILDER_STUB(Light_Source_Module)

    };

}
