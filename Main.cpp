#include <L_Debug/L_Debug.h>

#include <Type_Manager.h>
#include <Variable_Base.h>
#include <Builder_Stub.h>
#include <Object_Constructor.h>

#include <Object.h>
#include <FPS_Timer.h>

#include <Collision_Detection/Collision_Detector.h>
#include <Collision_Detection/Broad_Phase/Binary_Space_Partitioner.h>
#include <Collision_Detection/Narrow_Phase/Dynamic_Narrow_CD.h>
#include <Collision_Resolution/Collision_Resolver.h>
#include <Collision_Resolution/Collision_Resolution__Physics_Module_2D.h>

#include <Shader/Shader_Manager.h>
#include <Shader/Shader_Program.h>
#include <Shader/Shader.h>
#include <Shader/Shader_Components/Shader_Transform_Component.h>
#include <Renderer/Renderer.h>
#include <Camera/Camera_2D.h>
#include <Draw_Modules/Draw_Module.h>
#include <Window/Window_Controller.h>
#include <glew.h>
#include <glfw3.h>

#include <Click_Controller.h>


void register_basic_types()
{
    //  registration of basic types for model parser

    LV::Type_Manager::register_basic_types();

    LV::Type_Manager::register_type("glm::vec3", {
        [](const std::string& _val)
        {
            if(_val == ".")
                return false;

            unsigned int dots_count = 0;
            unsigned int i=0;
            if(_val[0] == '+' || _val[0] == '-')
                ++i;
            for(; i<_val.size(); ++i)
            {
                if(_val[i] == '.')
                {
                    ++dots_count;
                    continue;
                }
                if(_val[i] < '0' || _val[i] > '9')
                    return false;
            }

            if(dots_count > 1)
                return false;

            return true;
        },
            [](void* _variable_vptr, const LDS::Vector<std::string>& _values_as_string)
        {
            L_ASSERT(_values_as_string.size() == 3);

            glm::vec3& vec = *((glm::vec3*)_variable_vptr);
            for(unsigned int i=0; i<3; ++i)
                vec[i] = std::stof(_values_as_string[i]);
        },
            [](void* _variable_vptr)
        {
            const glm::vec3& variable = *((glm::vec3*)_variable_vptr);
            LDS::Vector<std::string> result;
            result.resize(3);
            result.push(std::to_string(variable[0]));
            result.push(std::to_string(variable[1]));
            result.push(std::to_string(variable[2]));
            return result;
        }
    });
    LV::Type_Manager::register_type("LDS::List<glm::vec3>", {
        [](const std::string& _val)
        {
            if(_val == ".")
                return false;

            unsigned int dots_count = 0;
            unsigned int i=0;
            if(_val[0] == '+' || _val[0] == '-')
                ++i;
            for(; i<_val.size(); ++i)
            {
                if(_val[i] == '.')
                {
                    ++dots_count;
                    continue;
                }
                if(_val[i] < '0' || _val[i] > '9')
                    return false;
            }

            if(dots_count > 1)
                return false;

            return true;
        },
            [](void* _variable_vptr, const LDS::Vector<std::string>& _values_as_string)
        {
            LDS::List<glm::vec3>& list = *((LDS::List<glm::vec3>*)_variable_vptr);
            list.clear();

            L_ASSERT(_values_as_string.size() % 3 == 0);

            for(unsigned int i=0; i<_values_as_string.size(); i += 3)
            {
                glm::vec3 vector;
                vector[0] = std::stof(_values_as_string[i]);
                vector[1] = std::stof(_values_as_string[i + 1]);
                vector[2] = std::stof(_values_as_string[i + 2]);

                list.push_back(vector);
            }
        },
            [](void* _variable_vptr)
        {
            const LDS::List<glm::vec3>& variable = *((LDS::List<glm::vec3>*)_variable_vptr);
            LDS::Vector<std::string> result;
            result.resize(variable.size() * 3);
            for(LDS::List<glm::vec3>::Const_Iterator it = variable.begin(); !it.end_reached(); ++it)
            {
                const glm::vec3& vec = *it;
                result.push(std::to_string(vec[0]));
                result.push(std::to_string(vec[1]));
                result.push(std::to_string(vec[2]));
            }
            return result;
        }
    });

    LV::Type_Manager::register_type("glm::vec2", {
        [](const std::string& _val)
        {
            if(_val == ".")
                return false;

            unsigned int dots_count = 0;
            unsigned int i=0;
            if(_val[0] == '+' || _val[0] == '-')
                ++i;
            for(; i<_val.size(); ++i)
            {
                if(_val[i] == '.')
                {
                    ++dots_count;
                    continue;
                }
                if(_val[i] < '0' || _val[i] > '9')
                    return false;
            }

            if(dots_count > 1)
                return false;

            return true;
        },
            [](void* _variable_vptr, const LDS::Vector<std::string>& _values_as_string)
        {
            L_ASSERT(_values_as_string.size() == 2);

            glm::vec2& vec = *((glm::vec2*)_variable_vptr);
            for(unsigned int i=0; i<2; ++i)
                vec[i] = std::stof(_values_as_string[i]);
        },
            [](void* _variable_vptr)
        {
            const glm::vec2& variable = *((glm::vec2*)_variable_vptr);
            LDS::Vector<std::string> result;
            result.resize(2);
            result.push(std::to_string(variable[0]));
            result.push(std::to_string(variable[1]));
            return result;
        }
    });
    LV::Type_Manager::register_type("LDS::List<glm::vec2>", {
        [](const std::string& _val)
        {
            if(_val == ".")
                return false;

            unsigned int dots_count = 0;
            unsigned int i=0;
            if(_val[0] == '+' || _val[0] == '-')
                ++i;
            for(; i<_val.size(); ++i)
            {
                if(_val[i] == '.')
                {
                    ++dots_count;
                    continue;
                }
                if(_val[i] < '0' || _val[i] > '9')
                    return false;
            }

            if(dots_count > 1)
                return false;

            return true;
        },
            [](void* _variable_vptr, const LDS::Vector<std::string>& _values_as_string)
        {
            LDS::List<glm::vec2>& list = *((LDS::List<glm::vec2>*)_variable_vptr);
            list.clear();

            L_ASSERT(_values_as_string.size() % 2 == 0);

            for(unsigned int i=0; i<_values_as_string.size(); i += 2)
            {
                glm::vec2 vector;
                vector[0] = std::stof(_values_as_string[i]);
                vector[1] = std::stof(_values_as_string[i + 1]);

                list.push_back(vector);
            }
        },
            [](void* _variable_vptr)
        {
            const LDS::List<glm::vec2>& variable = *((LDS::List<glm::vec2>*)_variable_vptr);
            LDS::Vector<std::string> result;
            result.resize(variable.size() * 2);
            for(LDS::List<glm::vec2>::Const_Iterator it = variable.begin(); !it.end_reached(); ++it)
            {
                const glm::vec2& vec = *it;
                result.push(std::to_string(vec[0]));
                result.push(std::to_string(vec[1]));
            }
            return result;
        }
    });
    LV::Type_Manager::register_type("LDS::Vector<glm::vec2>", {
        [](const std::string& _val)
        {
            if(_val == ".")
                return false;

            unsigned int dots_count = 0;
            unsigned int i=0;
            if(_val[0] == '+' || _val[0] == '-')
                ++i;
            for(; i<_val.size(); ++i)
            {
                if(_val[i] == '.')
                {
                    ++dots_count;
                    continue;
                }
                if(_val[i] < '0' || _val[i] > '9')
                    return false;
            }

            if(dots_count > 1)
                return false;

            return true;
        },
            [](void* _variable_vptr, const LDS::Vector<std::string>& _values_as_string)
        {
            LDS::Vector<glm::vec2>& vector = *((LDS::Vector<glm::vec2>*)_variable_vptr);
            vector.clear();

            L_ASSERT(_values_as_string.size() % 2 == 0);

            vector.resize(_values_as_string.size() / 2);

            for(unsigned int i=0; i<_values_as_string.size(); i += 2)
            {
                glm::vec2 vec;
                vec[0] = std::stof(_values_as_string[i]);
                vec[1] = std::stof(_values_as_string[i + 1]);

                vector.push(vec);
            }
        },
            [](void* _variable_vptr)
        {
            const LDS::Vector<glm::vec2>& variable = *((LDS::Vector<glm::vec2>*)_variable_vptr);
            LDS::Vector<std::string> result;
            result.resize(variable.size() * 2);
            for(unsigned int i=0; i<variable.size(); ++i)
            {
                result.push(std::to_string(variable[i][0]));
                result.push(std::to_string(variable[i][1]));
            }
            return result;
        }
    });
}

int main()
{
    LV::Object_Constructor object_constructor;

    LR::Shader_Manager* shader_manager = nullptr;
    LR::Renderer renderer;
    LR::Camera_2D camera;

    LPhys::Collision_Detector collision_detector;
    LPhys::Collision_Resolver collision_resolver;

    LEti::FPS_Timer fps_timer;

    //  initialization

    register_basic_types();

    object_constructor.register_type<LEti::Object_Stub>();

    object_constructor.register_type<LPhys::Physics_Module_2D_Stub>();

    object_constructor.register_type<LR::Draw_Module_Stub>().override_initialization_func([&](LV::Variable_Base* _product)
    {
        LR::Draw_Module_Stub* product = (LR::Draw_Module_Stub*)_product;
        product->renderer = &renderer;
        product->shader_manager = shader_manager;
    });
    object_constructor.register_type<LR::Graphics_Component_Stub>();
    object_constructor.register_type<LR::Shader_Manager_Stub>();
    object_constructor.register_type<LR::Shader_Program_Stub>();
    object_constructor.register_type<LR::Shader_Stub>();
    object_constructor.register_type<LR::Shader_Component_Stub>();
    object_constructor.register_type<LR::Shader_Transform_Component_Stub>();

    //  ~initialization

    //  Physics stuff
    {
        LPhys::Binary_Space_Partitioner* bp = new LPhys::Binary_Space_Partitioner;
        bp->set_precision(2);
        collision_detector.set_broad_phase(bp);
        LPhys::Dynamic_Narrow_CD* np = new LPhys::Dynamic_Narrow_CD;
        np->set_precision(5);
        collision_detector.set_narrow_phase(np);

        collision_resolver.set_resolution(new LPhys::Collision_Resolution__Physics_Module_2D);
    }
    //  ~Physics stuff


    //  Other rendering stuff
    {
        LR::Window_Controller::create_window(1200, 800, "Collision_Test");

        camera.set_position({0.0f, 0.0f, 0.0f});
        camera.set_view_scale(1.0f);

        renderer.set_camera(&camera);
    }
    //  ~Other rendering stuff

    //  Shader Manager
    {
        LV::MDL_Reader reader;
        reader.parse_file("Resources/Shaders/Shader_Manager");
        LR::Shader_Manager_Stub* stub = (LR::Shader_Manager_Stub*)object_constructor.construct(reader.get_stub("Shader_Manager"));
        shader_manager = LR::Shader_Manager_Stub::construct_from(stub);
        delete stub;
    }
    //  ~Shader Manager

    //  Objects

    Click_Controller click_controller;
    click_controller.inject_camera(&camera);

    {
        LV::MDL_Reader reader;
        reader.parse_file("Resources/Models/Objects");

        click_controller.set_object_stub((LEti::Object_Stub*)object_constructor.construct(reader.get_stub("Boring_Square")));
    }

    //  ~Objects

    //  App logic

    fps_timer.set_target_fps(72);

    fps_timer.set_on_tick([&](float _dt)
    {
        LR::Window_Controller::update();

        glClear(GL_COLOR_BUFFER_BIT);

        click_controller.update(_dt);

        LR::Window_Controller::swap_buffers();
    });

    while(!LR::Window_Controller::window_should_close())
    {
        fps_timer.tick();

        if(LR::Window_Controller::key_was_pressed(GLFW_KEY_ESCAPE))
            break;
    }

    //  ~App logic


    delete shader_manager;

    return 0;
}
