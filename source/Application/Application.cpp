#include <Application/Application.h>

#include <glew.h>
#include <glfw3.h>

#include <LV_Registration.h>
#include <LEti_Registration.h>
#include <LR_Registration.h>
#include <LPhys_Registration.h>
#include <LMD_Registration.h>
#include <LGui_Registration.h>

#include <Collision_Detection/Narrow_Phase/Dynamic_Narrow_CD__Mobile_Vs_Static.h>

#include <LSound_Registration.h>
#include <Sound_Engine.h>

#include <Misc_Modules/Expiration_Module.h>
#include <Physics/Rigid_Body/Collision_Resolution__Rigid_Body_3D.h>
#include <Tools/Objects_Controller/Objects_Controller_Extension__Physics.h>
#include <Tools/Objects_Controller/Objects_Controller_Extension__Removal.h>

#include <Messages/Messages.h>
#include <Graphics/Fragment_Shader_Light_Component.h>
#include <Modules/Light_Source_Module.h>
#include <Controllers/Objects_Controller_Extensions/Objects_Controller_Extension__On_Death_Notification.h>
#include <Controllers/Objects_Controller_Extensions/Objects_Controller_Extension__Gravity_Applier.h>

using namespace Shardis;

namespace Shardis
{
    constexpr float Max_Proximity_Distance = 200.0f;
    constexpr float Max_Proximity_Distance_Squared = Max_Proximity_Distance * Max_Proximity_Distance;
}


Application::Application()
{
    srand(time(nullptr));

    L_CREATE_LOG_LEVEL("Terrain Generation");
    // L_CREATE_LOG_LEVEL(LR::Window_Controller::log_level());

    LR::Window_Controller::instance().create_window(1920, 1080, "Shardis");

    m_renderer_helper = new Renderer_Helper;

    LV::register_types(m_object_constructor);
    LEti::register_types(m_object_constructor);
    LR::register_types(m_object_constructor, [this](){ return m_resources_manager; }, m_renderer, &m_renderer_helper->draw_order_controller(), [this](){ return m_shader_manager; }, [this](){ return m_main_game_dt; });
    LPhys::register_types(m_object_constructor);
    LSound::register_types(m_object_constructor, [this](){ return m_resources_manager; });
    LMD::register_types(m_object_constructor, [this](){ return m_resources_manager; }, m_renderer, &m_renderer_helper->draw_order_controller());
    // LGui::register_types(m_object_constructor, m_gui_camera, m_actions_controller);

    LSound::Sound_Engine::instance();

    M_register_messages();
    M_init_objects_controller();
    M_register_types();
    M_init_renderer();
    M_init_resources();
    M_init_objects();
    M_on_components_initialized();
    M_init_update_logic();
}

Application::~Application()
{
    delete m_object_stubs;

    delete m_objects_controller;

    delete m_renderer_helper;

    delete m_shader_manager;
    delete m_resources_manager;
}



void Application::M_register_messages()
{
    LST::Message_Translator::instance().register_message_type<Message__On_Entity_Death>();
    LST::Message_Translator::instance().register_message_type<Message__On_Entity_Damaged>();
    LST::Message_Translator::instance().register_message_type<Message__On_Entity_Healed>();
    LST::Message_Translator::instance().register_message_type<Message__On_Natural_Entity_Death>();
    LST::Message_Translator::instance().register_message_type<Message__Player_Evades>();
    LST::Message_Translator::instance().register_message_type<Message__Update_Camera>();
    LST::Message_Translator::instance().register_message_type<Message__Pause_Game>();
}

void Application::M_init_objects_controller()
{
    m_objects_controller = new LMD::Objects_Controller;

    {
        LMD::Objects_Controller_Extension__Removal* ext_removal = new LMD::Objects_Controller_Extension__Removal;
        m_objects_controller->add_extension(ext_removal);
    }

    m_objects_controller->add_extension(new Objects_Controller_Extension__Gravity_Applier);

    {
        LMD::Objects_Controller_Extension__Physics* ext_physics_terrain = new LMD::Objects_Controller_Extension__Physics;

        ext_physics_terrain->set_registration_filter([](const LPhys::Physics_Module* _module)
        {
            if(LV::cast_variable<LPhys::Physics_Module__Mesh>(_module))
                return true;
            if(LV::cast_variable<LMD::Physics_Module__Rigid_Body>(_module))
                return true;

            return _module->is_static();
        });

        LPhys::Binary_Space_Partitioner* bp = new LPhys::Binary_Space_Partitioner;
        bp->set_precision(2);
        bp->set_max_recursion_level(20);
        bp->add_filter([](const LPhys::Physics_Module* _first, const LPhys::Physics_Module* _second)->bool
        {
            return _first->is_static() ^ _second->is_static();
        });
        ext_physics_terrain->collision_detector().set_broad_phase(bp);

        LPhys::Dynamic_Narrow_CD__Mobile_Vs_Static* narrow = new LPhys::Dynamic_Narrow_CD__Mobile_Vs_Static;
        narrow->set_interpolation_precision(3);
        narrow->set_segments_division_precision(7);

        LPhys::SAT_Models_Intersection_3D* intersection_detector = new LPhys::SAT_Models_Intersection_3D;
        intersection_detector->set_min_polygons_for_optimization(8);
        intersection_detector->set_plane_contact_priority_ratio(50.0f);
        narrow->set_intersection_detector(intersection_detector);

        ext_physics_terrain->collision_detector().set_narrow_phase(narrow);

        LMD::Collision_Resolution__Rigid_Body_3D* rigid_body_resolution = new LMD::Collision_Resolution__Rigid_Body_3D;
        rigid_body_resolution->set_soft_damping_multiplier(0.9f);
        rigid_body_resolution->set_soft_damping_min_velocity(0.1f);
        rigid_body_resolution->set_soft_damping_min_angular_velocity(0.3f);
        // rigid_body_resolution->set_soft_damping_min_angular_velocity(0.00005f);
        rigid_body_resolution->set_hard_damping_velocity_threshold(0.03f);
        rigid_body_resolution->set_hard_damping_angular_velocity_threshold(0.08f);
        // rigid_body_resolution->set_hard_damping_angular_velocity_threshold(0.000005f);
        ext_physics_terrain->collision_resolver().add_resolution(rigid_body_resolution);

        m_objects_controller->add_extension(ext_physics_terrain);
    }

    m_objects_controller->add_extension(new Objects_Controller_Extension__On_Death_Notification);
}

void Application::M_register_types()
{
    m_object_constructor.register_type<LMD::Expiration_Module_Stub>().override_initialization_func([this](LV::Variable_Base* _product)
    {
        LMD::Expiration_Module_Stub* product = (LMD::Expiration_Module_Stub*)_product;

        product->on_expired_function = [](LEti::Object* _object)
        {
            LMD::Message__Request_Object_Deletion msg;
            msg.object = _object;
            LST::Message_Translator::instance().translate(msg);
        };
    });


    m_object_constructor.register_type<Fragment_Shader_Light_Component_Stub>().override_initialization_func([this](LV::Variable_Base* _product)
    {
        Fragment_Shader_Light_Component_Stub* product = (Fragment_Shader_Light_Component_Stub*)_product;

        product->camera = &m_ingame_camera;
    });


    m_object_constructor.register_type<Light_Source_Module_Stub>().override_initialization_func([this](LV::Variable_Base* _product)
    {
        Light_Source_Module_Stub* product = (Light_Source_Module_Stub*)_product;

        product->light_controller = m_renderer_helper->light_controller();
    });
}

void Application::M_init_renderer()
{
    LV::MDL_Reader reader;
    reader.parse_file("../Resources/Shaders/Shader_Manager");

    LR::Shader_Manager_Stub* shader_manager_stub = (LR::Shader_Manager_Stub*)m_object_constructor.construct(reader.get_stub("Shader_Manager"));
    m_shader_manager = LR::Shader_Manager_Stub::construct_from(shader_manager_stub);
    delete shader_manager_stub;

    // LR::Shader_Program* shader_program = m_shader_manager->get_shader_program("Shader_Program__Default");

    LR::Camera_3D::Settings camera_settings;
    camera_settings.position = { 0.0f, 0.0f, 0.0f };
    camera_settings.direction = {0.0f, 0.0f, 1.0f};
    camera_settings.top = { 0.0f, 1.0f, 0.0f };
    camera_settings.field_of_view = LST::Math::HALF_PI;
    camera_settings.min_distance = 0.1f;
    camera_settings.max_distance = 3000.0f;

    m_ingame_camera.set_settings(camera_settings);
    m_ingame_camera.apply_settings_forcefully();

    m_renderer.set_camera(&m_ingame_camera);

    LST::Message_Translator::instance().subscribe<LR::Message__Window_Resized>([this](LR::Message__Window_Resized& _msg)
    {
        m_ingame_camera.reconfigure();
    });

    glEnable(GL_CULL_FACE);
    glCullFace(GL_CCW);

    m_renderer_helper->inject_renderer(&m_renderer);
    m_renderer_helper->inject_base_game_camera(&m_ingame_camera);
    m_renderer_helper->init(m_object_constructor, m_shader_manager);
}

void Application::M_init_resources()
{
    LV::MDL_Reader reader;
    reader.parse_file("../Resources/Resources/Resources");

    LV::Variable_Loader_Stub* resources_manager_stub = (LV::Variable_Loader_Stub*)m_object_constructor.construct(reader.get_stub("Resources"));
    m_resources_manager = (LV::Variable_Loader*)resources_manager_stub->construct();
    delete resources_manager_stub;
}

void Application::M_init_objects()
{
    LV::MDL_Reader reader;
    reader.parse_file("../Resources/Objects/Objects");

    m_object_stubs = (LMD::Objects_Storage*)m_object_constructor.construct(reader.get_stub("Objects"));
    L_ASSERT(LV::cast_variable<LMD::Objects_Storage>(m_object_stubs));
}

void Application::M_on_components_initialized()
{
    m_camera_controller.inject_camera(&m_ingame_camera);

    LR::Window_Controller::instance().set_cursor_visibility(false);
    LR::Window_Controller::instance().set_cursor_pos(LR::Window_Controller::instance().get_window_size() * 0.5f);

    LEti::Object* light_source = new LEti::Object;
    Light_Source_Module* ls_module = new Light_Source_Module;
    ls_module->inject_light_controller(m_renderer_helper->light_controller());
    ls_module->set_light_color({1.0f, 1.0f, 1.0f});
    ls_module->set_max_light_distance(1000);
    light_source->add_module(ls_module);
    light_source->update(0.0f);
    light_source->update_previous_state();
    m_objects_controller->add_object(light_source);

    const LEti::Object_Stub* ground_object_stub = m_object_stubs->get_object<LEti::Object_Stub>("Ground");
    LEti::Object* ground_object = LEti::Object_Stub::construct_from(ground_object_stub);
    ground_object->current_state().set_position({0.0f, -5.0f, 0.0f});
    ground_object->update(0.0f);
    ground_object->update_previous_state();
    m_objects_controller->add_object(ground_object);
}

void Application::M_init_update_logic()
{
    m_fps_timer.set_target_fps(120);

    m_fps_timer.set_on_tick(
    [this](float _dt)
    {
        m_main_game_dt = _dt * m_dt_multiplier;

        LR::Window_Controller::instance().update();

        M_update_game();
    });
}



#ifdef L_DEBUG

void Application::M_create_entity()
{
    if(LR::Window_Controller::instance().key_was_pressed(GLFW_KEY_O))
    {
        // const LEti::Object_Stub* stub = m_object_stubs->get_object<LEti::Object_Stub>("Physics_Test_Object");
        const LEti::Object_Stub* stub = m_object_stubs->get_object<LEti::Object_Stub>("Physics_Test_Object__Pyramid");

        LEti::Object* object = LEti::Object_Stub::construct_from(stub);

        object->current_state().set_position( m_ingame_camera.settings().position + ( m_ingame_camera.settings().direction * 10.0f ) );

        LMD::Physics_Module__Rigid_Body* pm = object->get_module_of_type<LMD::Physics_Module__Rigid_Body>();
        if(pm)
        {
            // pm->set_velocity( m_ingame_camera.settings().direction * 10.0f );
            // pm->set_angular_velocity( m_ingame_camera.settings().top * 10.0f );

            pm->set_velocity( {LST::Math::random_number_float(-2.0f, 2.0f),
                              LST::Math::random_number_float(-2.0f, 2.0f),
                              LST::Math::random_number_float(-2.0f, 2.0f)} );
            pm->set_angular_velocity( {LST::Math::random_number_float(-LST::Math::DOUBLE_PI, LST::Math::DOUBLE_PI),
                                      LST::Math::random_number_float(-LST::Math::DOUBLE_PI, LST::Math::DOUBLE_PI),
                                      LST::Math::random_number_float(-LST::Math::DOUBLE_PI, LST::Math::DOUBLE_PI)} );
        }

        object->update(0.0f);
        object->update_previous_state();
        m_objects_controller->add_object(object);
    }
}

void Application::M_spawn_controller_debug_control()
{

}

void Application::M_process_force_pause()
{
    if(!LR::Window_Controller::instance().key_was_pressed(GLFW_KEY_F1))
        return;

    bool should_pause = m_dt_multiplier > LST::Math::Float_Precision_Tolerance;
    M_pause_dt(should_pause);
}

#endif



void Application::M_pause_dt(bool _pause)
{
    if(_pause)
        m_dt_multiplier = 0.0f;
    else
        m_dt_multiplier = 1.0f;
}


void Application::M_update_game()
{
    if(LR::Window_Controller::instance().key_was_pressed(GLFW_KEY_ESCAPE))
    {
        LR::Window_Controller::instance().set_window_should_close(true);
    }

#ifdef L_DEBUG
    M_process_force_pause();
#endif

    m_objects_controller->update_previous_state();

#ifdef L_DEBUG
    M_create_entity();
    M_spawn_controller_debug_control();

#endif

    m_camera_controller.update(m_main_game_dt);

    m_objects_controller->update(m_main_game_dt);

    LR::Window_Controller::instance().clear(true, true);
    glEnable(GL_DEPTH_TEST);
    m_renderer_helper->update(m_main_game_dt);
    glDisable(GL_DEPTH_TEST);
    LR::Window_Controller::instance().swap_buffers();
}



void Application::run()
{
    while (!m_should_terminate && !LR::Window_Controller::instance().window_should_close())
    {
        m_fps_timer.tick();
    }
}
