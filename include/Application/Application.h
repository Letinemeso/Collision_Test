#pragma once

#include <Stuff/Command_Queue.h>

#include <Object_Constructor.h>
#include <Variable_Loader.h>

#include <FPS_Timer.h>

#include <Renderer/Renderer.h>
#include <Draw_Order_Controller/Draw_Order_Controller.h>
#include <Shader/Shader_Manager.h>
#include <Camera/Camera_2D.h>

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
#include <Graphics/Renderer_Helper.h>
#include <Graphics/Camera.h>


namespace Shardis
{

    class Application
    {
    private:
        LV::Object_Constructor m_object_constructor;

        LV::Variable_Loader* m_resources_manager = nullptr;

        LR::Renderer m_renderer;
        LR::Shader_Manager* m_shader_manager = nullptr;
        Renderer_Helper* m_renderer_helper = nullptr;

        Camera m_ingame_camera;

        LEti::FPS_Timer m_fps_timer;
        float m_dt_multiplier = 1.0f;
        float m_main_game_dt = 0.0f;
        bool m_should_terminate = false;

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
        void M_init_objects();
        void M_on_components_initialized();
        void M_init_update_logic();

    private:    //  debug stuff
#ifdef L_DEBUG
        void M_create_entity();
        void M_spawn_controller_debug_control();
        void M_process_force_pause();
#endif

    private:
        void M_pause_dt(bool _pause);

        void M_update_game();

    public:
        void run();

    };

}
