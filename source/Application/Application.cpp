#include <Application/Application.h>

#include <glew.h>
#include <glfw3.h>

#include <LEti_Registration.h>
#include <LR_Registration.h>
#include <LPhys_Registration.h>
#include <LMD_Registration.h>
#include <LGui_Registration.h>

#include <Collision_Detection/Narrow_Phase/Dynamic_Narrow_CD__Mobile_Vs_Static.h>

#include <Misc_Modules/Expiration_Module.h>
#include <Physics/Rigid_Body/Collision_Resolution__Rigid_Body_3D.h>
#include <Tools/Objects_Controller/Objects_Controller_Extension__Physics.h>
#include <Tools/Objects_Controller/Objects_Controller_Extension__Removal.h>

#include <Messages/Messages.h>
#include <Graphics/Fragment_Shader_Light_Component.h>
#include <Physics/Collision_Resolution.h>
#include <Modules/Light_Source_Module.h>
#include <Modules/Velocity_Module.h>
#include <Modules/Type_Module.h>
#include <Modules/Health_Module.h>
#include <Modules/Attack_Module.h>
#include <Modules/Player_Control_Module.h>
#include <Modules/On_Death_Effect_Module.h>
#include <Modules/On_Damaged_Effect_Module.h>
#include <Modules/Entity_AI/Crystalis_AI.h>
#include <Modules/Misc_Modules/Health_Bar_Module.h>
#include <Modules/Misc_Modules/Damage_Indicator_Module.h>
#include <Weapons/Weapon.h>
#include <Weapons/Generic_Weapon.h>
#include <Controllers/Objects_Controller_Extensions/Objects_Controller_Extension__On_Death_Notification.h>
#include <Controllers/Objects_Controller_Extensions/Objects_Controller_Extension__Entity_Stabilizer.h>
#include <Controllers/Objects_Controller_Extensions/Objects_Controller_Extension__Entity_Proximity_Checker.h>
#include <Controllers/Objects_Controller_Extensions/Objects_Controller_Extension__Gravity_Applier.h>

using namespace Shardis;

// LEti::Object* terrain_obj = nullptr;


Application::Application()
{
    srand(time(nullptr));

    L_CREATE_LOG_LEVEL("Terrain Generation");
    // L_CREATE_LOG_LEVEL(LR::Window_Controller::log_level());

    LR::Window_Controller::instance().create_window(1920, 1080, "Shardis");

    m_renderer_helper = new Renderer_Helper;

    LV::Type_Manager::register_basic_types();
    LEti::register_types(m_object_constructor);
    LR::register_types(m_object_constructor, [this](){ return m_resources_manager; }, m_renderer, &m_renderer_helper->draw_order_controller(), [this](){ return m_shader_manager; }, [this](){ return m_dt; });
    LPhys::register_types(m_object_constructor);
    LMD::register_types(m_object_constructor, [this](){ return m_resources_manager; }, m_renderer, &m_renderer_helper->draw_order_controller());
    // LGui::register_types(m_object_constructor, m_gui_camera, m_actions_controller);

    M_register_messages();
    M_init_objects_controller();
    M_register_types();
    M_init_renderer();
    M_init_resources();
    M_init_weapons();
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
    LST::Message_Translator::instance().register_message_type<Message__Player_Evades>();
    LST::Message_Translator::instance().register_message_type<Message__Find_Player>();
    LST::Message_Translator::instance().register_message_type<Message__Update_Camera>();

    LST::Message_Translator::instance().subscribe<Message__Find_Player>([this](Message__Find_Player& _msg)
    {
        _msg.player_object = m_objects_controller->get_suitable_object([](LEti::Object* _object)
        {
            Type_Module* type_module = _object->get_module_of_type<Type_Module>();
            if(!type_module)
                return false;

            return type_module->object_type() == Object_Type::Player;
        });
    });
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

        LPhys::Binary_Space_Partitioner* bp = new LPhys::Binary_Space_Partitioner;
        bp->set_precision(2);
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
        intersection_detector->set_plane_contact_priority_ratio(10.0f);
        narrow->set_intersection_detector(intersection_detector);

        ext_physics_terrain->collision_detector().set_narrow_phase(narrow);

        LMD::Collision_Resolution__Rigid_Body_3D* rigid_body_resolution = new LMD::Collision_Resolution__Rigid_Body_3D;
        // rigid_body_resolution->set_impulse_ratio_after_collision(0.9f);
        ext_physics_terrain->collision_resolver().add_resolution(rigid_body_resolution);

        Collision_Resolution* resolution = new Collision_Resolution;
        resolution->set_additional_offset_between_objects(0.05f);
        ext_physics_terrain->collision_resolver().add_resolution(resolution);

        m_objects_controller->add_extension(ext_physics_terrain);
    }

    {
        LMD::Objects_Controller_Extension__Physics* ext_physics_entities = new LMD::Objects_Controller_Extension__Physics;

        ext_physics_entities->set_registration_filter([](LPhys::Physics_Module* _module)
        {
            return !_module->is_static();
        });

        LPhys::Binary_Space_Partitioner* bsp = new LPhys::Binary_Space_Partitioner;
        ext_physics_entities->collision_detector().set_broad_phase(bsp);

        LPhys::Dynamic_Narrow_CD* narrow = new LPhys::Dynamic_Narrow_CD;
        narrow->set_precision(5);

        LPhys::SAT_Models_Intersection_3D* intersection_detector = new LPhys::SAT_Models_Intersection_3D;
        intersection_detector->set_min_polygons_for_optimization(8);
        intersection_detector->set_plane_contact_priority_ratio(1.0f);
        narrow->set_intersection_detector(intersection_detector);

        ext_physics_entities->collision_detector().set_narrow_phase(narrow);

        ext_physics_entities->collision_resolver().add_resolution(new Collision_Resolution);

        m_objects_controller->add_extension(ext_physics_entities);
    }

    m_objects_controller->add_extension(new Objects_Controller_Extension__On_Death_Notification);

    {
        Objects_Controller_Extension__Entity_Stabilizer* ext_stabilizer = new Objects_Controller_Extension__Entity_Stabilizer;
        ext_stabilizer->set_min_stride_per_frame(0.01f);
        m_objects_controller->add_extension(ext_stabilizer);
    }

    {
        Objects_Controller_Extension__Entity_Proximity_Checker* ext_proximity_checker = new Objects_Controller_Extension__Entity_Proximity_Checker;
        ext_proximity_checker->inject_camera(&m_ingame_camera);
        ext_proximity_checker->set_max_distance(200);
        m_objects_controller->add_extension(ext_proximity_checker);
    }
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


    m_object_constructor.register_type<Fragment_Shader_Light_Component_Stub>();


    m_object_constructor.register_type<Light_Source_Module_Stub>().override_initialization_func([this](LV::Variable_Base* _product)
    {
        Light_Source_Module_Stub* product = (Light_Source_Module_Stub*)_product;

        product->light_controller = m_renderer_helper->light_controller();
    });

    m_object_constructor.register_type<Velocity_Module_Stub>();

    m_object_constructor.register_type<Type_Module_Stub>();

    m_object_constructor.register_type<Health_Module_Stub>();

    m_object_constructor.register_type<Attack_Module_Stub>();

    m_object_constructor.register_type<On_Death_Effect_Module_Stub>().override_initialization_func([this](LV::Variable_Base* _product)
    {
        On_Death_Effect_Module_Stub* product = (On_Death_Effect_Module_Stub*)_product;

        product->objects_controller = m_objects_controller;
    });

    m_object_constructor.register_type<On_Damaged_Effect_Module_Stub>().override_initialization_func([this](LV::Variable_Base* _product)
    {
        On_Damaged_Effect_Module_Stub* product = (On_Damaged_Effect_Module_Stub*)_product;

        product->objects_controller = m_objects_controller;
    });

    m_object_constructor.register_type<Player_Control_Module_Stub>().override_initialization_func([this](LV::Variable_Base* _product)
    {
        Player_Control_Module_Stub* product = (Player_Control_Module_Stub*)_product;

        product->camera = &m_ingame_camera;
        product->objects_controller = m_objects_controller;
        product->weapons_provider = m_weapons_stubs;
    });


    m_object_constructor.register_type<Weapon_Stub>().override_initialization_func([this](LV::Variable_Base* _product)
    {
        Weapon_Stub* product = (Weapon_Stub*)_product;

        product->objects_controller = m_objects_controller;
        product->main_game_camera = &m_ingame_camera;
    });

    m_object_constructor.register_type<Generic_Weapon_Stub>();


    m_object_constructor.register_type<Health_Bar_Module_Stub>();

    m_object_constructor.register_type<Damage_Indicator_Module_Stub>().override_initialization_func([this](LV::Variable_Base* _product)
    {
        Damage_Indicator_Module_Stub* product = (Damage_Indicator_Module_Stub*)_product;

        product->camera = &m_ingame_camera;
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
    camera_settings.field_of_view = LEti::Math::HALF_PI;
    camera_settings.max_distance = 100000.0f;

    m_ingame_camera.set_settings(camera_settings);

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

    LEti::Resources_Manager_Stub* resources_manager_stub = (LEti::Resources_Manager_Stub*)m_object_constructor.construct(reader.get_stub("Resources"));
    m_resources_manager = (LEti::Resources_Manager*)resources_manager_stub->construct();
    delete resources_manager_stub;
}

void Application::M_init_weapons()
{
    LV::MDL_Reader reader;
    reader.parse_file("../Resources/Weapons/Weapons");

    m_weapons_stubs = (LMD::Objects_Storage*)m_object_constructor.construct(reader.get_stub("Weapons"));
    L_ASSERT(LV::cast_variable<LMD::Objects_Storage>(m_weapons_stubs));
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

    // LEti::Object_Stub* test_object_stub = m_object_stubs->get_object<LEti::Object_Stub>("Crystalis");
    LEti::Object_Stub* test_object_stub = m_object_stubs->get_object<LEti::Object_Stub>("Model_Test_Entity");
    L_ASSERT(test_object_stub);
    LEti::Object_Stub* terrain_stub = m_object_stubs->get_object<LEti::Object_Stub>("Ground");
    L_ASSERT(terrain_stub);

    LEti::Object* terrain_obj = LEti::Object_Stub::construct_from(terrain_stub);
    m_objects_controller->add_object(terrain_obj);

    // for(unsigned int i = 0; i < 0; ++i)
    // {
    //     LEti::Object* test_object = LEti::Object_Stub::construct_from(test_object_stub);
    //     // test_object->current_state().set_position(initial_position);
    //     test_object->current_state().move({ 5.0f * (float)(i + 1), 2.0f, 2.0f });
    //     test_object->update(0.0f);
    //     test_object->update_previous_state();
    //     m_objects_controller->add_object(test_object);
    // }

    LR::Shader_Program* sp = m_shader_manager->get_shader_program("Shader_Program__Final_Shader");
    LR::Shader* shader = sp->get_shader_of_type(LR::Shader_Type::Fragment);
    Fragment_Shader_Light_Component* light_controller = shader->get_shader_component_of_type<Fragment_Shader_Light_Component>();
    light_controller->add_light_source( {0.0f, 10.0f, 0.0f}, {1.0f, 1.0f, 1.0f}, 500.0f );

    // LEti::Object_Stub* fx_object_stub = m_object_stubs->get_object<LEti::Object_Stub>("Shards_Splash_FX");
    // LEti::Object* fx_object = LEti::Object_Stub::construct_from(fx_object_stub);
    // fx_object->current_state().set_position(player_object->current_state().position() + glm::vec3(-5, 0, 0));
    // fx_object->update(0.0f);
    // fx_object->update_previous_state();
    // m_objects_controller->add_object(fx_object);
}

void Application::M_init_update_logic()
{
    m_fps_timer.set_target_fps(120);

    m_fps_timer.set_on_tick(
    [this](float _dt)
    {
        m_dt = _dt;

        LR::Window_Controller::instance().update();

        M_update_game(_dt);
    });
}



void Application::M_update_game(float _dt)
{
    if(LR::Window_Controller::instance().key_was_pressed(GLFW_KEY_ESCAPE))
    {
        LR::Window_Controller::instance().set_window_should_close(true);
    }

    m_objects_controller->update_previous_state();

    if(LR::Window_Controller::instance().key_was_pressed(GLFW_KEY_O))
    {
        const LEti::Object_Stub* stub = m_object_stubs->get_object<LEti::Object_Stub>("Model_Test_Entity");
        LEti::Object* object = LEti::Object_Stub::construct_from(stub);

        LMD::Physics_Module__Rigid_Body* pm = object->get_module_of_type<LMD::Physics_Module__Rigid_Body>();
        if(pm)
        {
            pm->set_velocity( m_ingame_camera.settings().direction * 3.0f );
            pm->set_angular_velocity( m_ingame_camera.settings().top * 10.0f );
            pm->set_center_of_mass_position( m_ingame_camera.settings().position + ( m_ingame_camera.settings().direction * 3.0f ) );
        }

        object->update(0.0f);
        object->update_previous_state();
        m_objects_controller->add_object(object);
    }

    m_camera_controller.update(_dt);

    // terrain_obj->update(_dt);
    m_objects_controller->update(_dt);

    m_renderer_helper->update(_dt);
}



void Application::run()
{
    while (!LR::Window_Controller::instance().window_should_close())
    {
        m_fps_timer.tick();
    }
}
