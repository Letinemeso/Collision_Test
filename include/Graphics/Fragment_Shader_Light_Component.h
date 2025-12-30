#pragma once

#include <Data_Structures/Vector.h>

#include <Shader/Shader_Components/Shader_Component.h>
#include <Camera/Camera_2D.h>


namespace Shardis
{

    class Fragment_Shader_Light_Component : public LR::Shader_Component
    {
    public:
        INIT_VARIABLE(Shardis::Fragment_Shader_Light_Component, LR::Shader_Component)

    private:
        static constexpr unsigned int Max_Light_Sources = 64;

    public:
        struct Light_Source_Data
        {
            glm::vec3 color = {-1.0f, -1.0f, -1.0f};
            glm::vec3 point = {0.0f, 0.0f, 0.0f};
            float max_light_spread_distance = -1.0f;
            bool enabled = true;

            inline bool initialized() const { return max_light_spread_distance > 0.0f; }
            inline void reset()
            {
                color = {-1.0f, -1.0f, -1.0f};
                point = {0.0f, 0.0f, 0.0f};
                max_light_spread_distance = -1.0f;
                enabled = true;
            }
        };

    private:
        struct Light_Source_Data_Uniforms
        {
            int color = -1;
            int point = -1;
            int max_light_spread_distance = -1;
        };

    private:
        float m_min_light_ratio = 0.00f;

    private:
        using Light_Sources_Container = LDS::List<Light_Source_Data>;
        Light_Sources_Container m_light_sources;

        int m_relevant_light_sources_location = -1;
        Light_Source_Data_Uniforms m_light_sources_uniforms[Max_Light_Sources];

        int m_min_light_ratio_uniform = -1;

    public:
        Fragment_Shader_Light_Component();
        ~Fragment_Shader_Light_Component();

    public:
        inline void set_min_light_ratio(float _value) { m_min_light_ratio = _value; }

        inline float min_light_ratio() const { return m_min_light_ratio; }

    public:
        Light_Source_Data* add_light_source();
        Light_Source_Data* add_light_source(const glm::vec3& _point, const glm::vec3& _color, float _max_light_spread_distance);
        void remove_light_source(Light_Source_Data*& _light_source);

    public:
        void init(unsigned int _opengl_program_handle) override;

    private:
        Light_Sources_Container::Iterator M_find_light_source(Light_Source_Data* _light_source);

    public:
        void update(const LR::Draw_Module* _draw_module) override;

    };


    class Fragment_Shader_Light_Component_Stub : public LR::Shader_Component_Stub
    {
    public:
        INIT_VARIABLE(Shardis::Fragment_Shader_Light_Component_Stub, LR::Shader_Component_Stub)

    public:
        INIT_BUILDER_STUB(Fragment_Shader_Light_Component)

    };

}
