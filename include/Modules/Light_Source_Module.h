#pragma once

#include <Module.h>

#include <Graphics/Fragment_Shader_Light_Component.h>


namespace Shardis
{

    class Light_Source_Module : public LEti::Module
    {
    public:
        INIT_VARIABLE(Shardis::Light_Source_Module, LEti::Module)

    private:
        Fragment_Shader_Light_Component* m_light_controller = nullptr;

        glm::vec3 m_light_color;
        float m_max_light_distance = 0.0f;
        glm::vec3 m_additional_offset;

    private:
        Fragment_Shader_Light_Component::Light_Source_Data* m_light_source = nullptr;

    public:
        Light_Source_Module();
        ~Light_Source_Module();

    public:
        inline void inject_light_controller(Fragment_Shader_Light_Component* _ptr) { m_light_controller = _ptr; }
        inline void set_additional_offset(const glm::vec3& _value) { m_additional_offset = _value; }

        void set_light_color(const glm::vec3& _color);
        void set_max_light_distance(float _max_distance);

    private:
        void M_on_parent_object_set() override;

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
        FIELDS_END

    public:
        glm::vec3 light_color = {1.0f, 1.0f, 1.0f};
        float max_light_distance = 0.0f;
        glm::vec3 additional_offset = {0.0f, 0.0f, 0.0f};

    public:
        Fragment_Shader_Light_Component* light_controller = nullptr;

    public:
        INIT_BUILDER_STUB(Light_Source_Module)

    };

}
