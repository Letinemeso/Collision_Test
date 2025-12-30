#pragma once

#include <Object_Constructor.h>

#include <Object.h>

#include <Draw_Order_Controller/Draw_Order_Controller.h>
#include <Shader/Shader_Program.h>
#include <Shader/Shader_Manager.h>
#include <Texture/Texture__Frame_Buffer.h>

#include <Screen.h>

#include <Graphics/Fragment_Shader_Light_Component.h>


namespace Shardis
{

    class Renderer_Helper
    {
    private:
        LR::Renderer* m_renderer = nullptr;
        LR::Camera_Base* m_base_game_camera = nullptr;

        LR::Draw_Order_Controller m_draw_order_controller;
        LEti::Object* m_base_game_rendering_object = nullptr;

        Fragment_Shader_Light_Component* m_light_controller = nullptr;

        LR::Texture__Frame_Buffer* m_base_game_frame_buffer = nullptr;

    public:
        Renderer_Helper();
        ~Renderer_Helper();

    public:
        inline LR::Draw_Order_Controller& draw_order_controller() { return m_draw_order_controller; }
        inline Fragment_Shader_Light_Component* light_controller() const { return m_light_controller; }

    public:
        void inject_renderer(LR::Renderer* _ptr);
        void inject_base_game_camera(LR::Camera_Base* _ptr);

        void init(const LV::Object_Constructor& _object_constructor, const LR::Shader_Manager* _shader_manager);

    public:
        void update(float _dt);

    };

}
