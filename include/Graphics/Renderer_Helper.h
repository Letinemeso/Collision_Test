#pragma once

#include <Object_Constructor.h>

#include <Object.h>

#include <Draw_Order_Controller/Draw_Order_Controller.h>
#include <Shader/Shader_Program.h>
#include <Shader/Shader_Manager.h>
#include <Shader/Shader_Components/Shader_Component__Uniforms.h>
#include <Texture/Texture__Frame_Buffer.h>

#include <Screen.h>

#include <Graphics/Fragment_Shader_Light_Component.h>
#include <Graphics/Camera.h>


namespace Shardis
{

    class Renderer_Helper
    {
    private:
        LR::Renderer* m_renderer = nullptr;
        Camera* m_base_game_camera = nullptr;

        LR::Draw_Order_Controller m_draw_order_controller;
        LEti::Object* m_base_game_rendering_object = nullptr;
        LR::Draw_Module* m_default_dm = nullptr;
        LR::Draw_Module* m_fractured_dm = nullptr;

        Fragment_Shader_Light_Component* m_light_controller = nullptr;
        LR::Shader_Component__Uniform__Float* m_brightness_controller = nullptr;

        LST::Key_Frame_Animation<float> m_brightness_kfa;

    public:
        Renderer_Helper();
        ~Renderer_Helper();

    public:
        inline LR::Draw_Order_Controller& draw_order_controller() { return m_draw_order_controller; }
        inline Fragment_Shader_Light_Component* light_controller() const { return m_light_controller; }

    public:
        void inject_renderer(LR::Renderer* _ptr);
        void inject_base_game_camera(Camera* _ptr);

        void init(const LV::Object_Constructor& _object_constructor, const LR::Shader_Manager* _shader_manager);

    private:
        void M_init_final_dm(LR::Draw_Module* _module) const;

    public:
        void start_brightness_fade(float _start_value, float _end_value, float _duration);
        void stop_brightness_fade();
        void set_brightness(float _value);

        bool brightness_fade_completed() const;

        bool is_default_view() const;
        void set_default_view();
        void set_fractured_view();

    public:
        void update(float _dt);

    };

}
