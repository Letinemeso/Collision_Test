#pragma once

#include <Object_Constructor.h>

#include <Resources_Manager.h>
#include <FPS_Timer.h>

#include <Renderer/Renderer.h>
#include <Draw_Order_Controller/Draw_Order_Controller.h>
#include <Shader/Shader_Manager.h>
#include <Camera/Camera_2D.h>
#include <Camera/Camera_3D.h>

#include <Collision_Detection/Collision_Detector.h>
#include <Collision_Detection/Broad_Phase/Binary_Space_Partitioner.h>
#include <Collision_Detection/Narrow_Phase/Dynamic_Narrow_CD.h>
#include <Collision_Detection/Primitives/SAT_Models_Intersection_3D.h>
#include <Collision_Resolution/Collision_Resolver.h>
#include <Collision_Resolution/Collision_Resolution__Physics_Module__Mesh.h>
#include <Modules/Physics_Module__Mesh.h>

#include <Tools/Objects_Storage.h>
#include <Tools/Objects_Controller/Objects_Controller.h>

#include <Actions_Controller.h>
#include <Screen.h>

#include <Controllers/Camera_Controller.h>
#include <Controllers/Spawn_Controller.h>
#include <Graphics/Renderer_Helper.h>
#include <Terrain/Terrain_Controller.h>


namespace Shardis
{

    class Application
    {
    private:
        LV::Object_Constructor m_object_constructor;

        LEti::Resources_Manager* m_resources_manager = nullptr;

        LR::Renderer m_renderer;
        LR::Shader_Manager* m_shader_manager = nullptr;
        Renderer_Helper* m_renderer_helper = nullptr;

        LR::Camera_3D m_ingame_camera;

        LEti::FPS_Timer m_fps_timer;
        float m_dt = 0.0f;

        LMD::Objects_Storage* m_object_stubs = nullptr;
        LMD::Objects_Storage* m_weapons_stubs = nullptr;

        Camera_Controller m_camera_controller;

        LMD::Objects_Controller* m_objects_controller = nullptr;

    public:
        Application();
        ~Application();

    private:
        void M_register_messages();
        void M_init_objects_controller();
        void M_register_types();
        void M_init_renderer();
        void M_init_resources();
        void M_init_weapons();
        void M_init_objects();
        void M_on_components_initialized();
        void M_init_update_logic();

    private:
        void M_update_game(float _dt);

    public:
        void run();

    };

}
