#include <chrono>
#include <thread>
#include <map>

#include <MDL_Reader.h>

#include <Stuff/Timer.h>
#include <Stuff/File.h>

#include <FPS_Timer.h>
#include <Object_System/Object_2D.h>

#include <Shader/Shader.h>
#include <Camera/Camera_2D.h>
#include <Picture/Graphic_Resources_Manager.h>

#include <Collision_Detection/Collision_Detector_2D.h>
#include <Collision_Detection/Space_Hasher_2D.h>
#include <Collision_Detection/Dynamic_Narrow_CD.h>
#include <Collision_Detection/Default_Narrowest_CD.h>
#include <Collision_Detection/SAT_Narrowest_CD.h>
#include <Collision_Resolution/Collision_Resolver.h>
#include <Collision_Resolution/Collision_Resolution__Rigid_Body_2D.h>
#include <Draw_Modules/Draw_Module__Animation.h>
#include <Draw_Modules/Draw_Module__Text_Field.h>
#include <Renderer/Renderer.h>



#define DT LR::Window_Controller::get_dt()


class Test_Object : public LEti::Object_2D
{
public:
    DECLARE_VARIABLE;

public:
    LPhys::Rigid_Body_2D* physics_module = nullptr;

};

INIT_FIELDS(Test_Object, LEti::Object_2D)
FIELDS_END



class Test_Object_Stub : public LEti::Object_2D_Stub
{
public:
    DECLARE_VARIABLE;

public:
    LPhys::Rigid_Body_2D__Stub* physics_module = nullptr;
    LR::Default_Draw_Module_2D_Stub* draw_module = nullptr;

protected:
    LV::Variable_Base* M_construct_product() const override;
    void M_init_constructed_product(LV::Variable_Base* _product) const override;

public:
    ~Test_Object_Stub();

};



INIT_FIELDS(Test_Object_Stub, LEti::Object_2D_Stub)

ADD_CHILD("draw_module", *draw_module)
ADD_CHILD("physics_module", *physics_module)

FIELDS_END


LV::Variable_Base* Test_Object_Stub::M_construct_product() const
{
    return new Test_Object;
}

void Test_Object_Stub::M_init_constructed_product(LV::Variable_Base* _product) const
{
    LEti::Object_2D_Stub::M_init_constructed_product(_product);

    Test_Object* result = (Test_Object*)_product;

    LR::Default_Draw_Module_2D* dm = (LR::Default_Draw_Module_2D*)draw_module->construct();

    LPhys::Rigid_Body_2D* pm = (LPhys::Rigid_Body_2D*)physics_module->construct();
    pm->set_on_alignment_func([pm, dm]()
    {
        dm->move_raw(-pm->calculate_raw_center_of_mass());
    });
    pm->align_to_center_of_mass();

    result->add_module(pm);
    result->add_module(dm);

    result->physics_module = pm;
}



Test_Object_Stub::~Test_Object_Stub()
{
    delete physics_module;
    delete draw_module;
}






class Color_Controll
{
private:
    glm::vec3 initial_direction = {1.0f, 1.0f, 1.0f};
    glm::vec3 axis = {-1.0f, 1.0f, -1.0f};
    glm::mat4x4 matrix;
    float angle = 0.0f;

    glm::vec3 result_direction;

public:
    Color_Controll()
    {
        matrix = glm::rotate(angle, axis);
        result_direction = matrix * glm::vec4(initial_direction, 1.0f);
    }

    void update()
    {
//        angle += LEti::Math::PI * LR::Window_Controller::get_dt();

        matrix = glm::rotate(angle, axis);
        result_direction = matrix * glm::vec4(initial_direction, 1.0f);
    }

    float r_ratio() const
    {
        return fabs(result_direction.x);
    }
    float g_ratio() const
    {
        return fabs(result_direction.y);
    }
    float b_ratio() const
    {
        return fabs(result_direction.z);
    }

    void change_color(LEti::Object_2D& _obj)
    {
//        for(unsigned int i=0; i < _obj.draw_module()->colors().size(); i += 4)
//            _obj.draw_module()->colors()[i] = r_ratio();
//        for(unsigned int i=1; i < _obj.draw_module()->colors().size(); i += 4)
//            _obj.draw_module()->colors()[i] = g_ratio();
//        for(unsigned int i=2; i < _obj.draw_module()->colors().size(); i += 4)
//            _obj.draw_module()->colors()[i] = b_ratio();
    }

};


class Grab
{
public:
	enum class Type
	{
		none = 0,
		drag,
		launch
	};

private:
    Test_Object* grabbed_object = nullptr;
	glm::vec3 cursor_pos;

	glm::vec3 launch_from{0.0f, 0.0f, 0.0f};

	Type type = Type::none;

public:
    const LR::Camera_2D* camera = nullptr;

public:
	void update()
	{
        cursor_pos = {LR::Window_Controller::get_cursor_position().x, LR::Window_Controller::get_cursor_position().y, 0.0f};
        cursor_pos = camera->convert_window_coords(cursor_pos);

        if(!grabbed_object)
            return;

        grabbed_object->physics_module->set_velocity(grabbed_object->current_state().position() - grabbed_object->previous_state().position());

        if(type == Type::drag)
            grabbed_object->current_state().set_position(cursor_pos);
	}

    void grab(Test_Object* _obj, Type _type)
	{
		if(type != Type::none) return;
		type = _type;
		grabbed_object = _obj;

        grabbed_object->physics_module->set_velocity({0.0f, 0.0f, 0.0f});
        grabbed_object->physics_module->set_angular_velocity(0.0f);

        if(_type == Type::drag)
            grabbed_object->current_state().set_position(cursor_pos);

        if(_type == Type::launch)
            launch_from = cursor_pos;
	}

	void release()
	{
		if(!grabbed_object) return;

		if(type == Type::drag)
		{
//			glm::vec3 stride = grabbed_object->get_pos() - grabbed_object->get_pos_prev();

//			grabbed_object->velocity = stride / DT;
		}
		else if(type == Type::launch)
		{
			glm::vec3 stride = (cursor_pos - launch_from);

//			grabbed_object->set_velocity(stride);
		}


		grabbed_object = nullptr;
		type = Type::none;
	}

};




int main()
{
    //  engine setup

    LV::Type_Manager::register_type("int", {
                                        [](const std::string& _val)
                                        {
                                            unsigned int i=0;
                                            if(_val[0] == '+' || _val[0] == '-')
                                            ++i;
                                            for(; i<_val.size(); ++i)
                                            if(_val[i] < '0' || _val[i] > '9')
                                            return false;
                                            return true;
                                        },
                                        [](void* _variable_vptr, const LDS::Vector<std::string>& _values_as_string) { *((int*)_variable_vptr) = std::stoi(_values_as_string[0]); }
                                    });
    LV::Type_Manager::register_type("int*", {
                                        [](const std::string& _val)
                                        {
                                            unsigned int i=0;
                                            if(_val[0] == '+' || _val[0] == '-')
                                            ++i;
                                            for(; i<_val.size(); ++i)
                                            if(_val[i] < '0' || _val[i] > '9')
                                            return false;
                                            return true;
                                        },
                                        [](void* _variable_vptr, const LDS::Vector<std::string>& _values_as_string)
                                        {
                                            int** var_ptr_ptr = (int**)_variable_vptr;

                                            if(*var_ptr_ptr == nullptr)
                                                *var_ptr_ptr = new int[_values_as_string.size()];

                                            int* var_ptr = *var_ptr_ptr;

                                            for(unsigned int i=0; i<_values_as_string.size(); ++i)
                                                var_ptr[i] = std::stoi(_values_as_string[i]);
                                        }
                                    });
    LV::Type_Manager::register_type("unsigned char*", {
                                        [](const std::string& _val)
                                        {
                                            if(_val.size() != 1)
                                                return false;
                                            return true;
                                        },
                                        [](void* _variable_vptr, const LDS::Vector<std::string>& _values_as_string)
                                        {
                                            unsigned char** var_ptr_ptr = (unsigned char**)_variable_vptr;

                                            if(*var_ptr_ptr == nullptr)
                                                *var_ptr_ptr = new unsigned char[_values_as_string.size()];

                                            unsigned char* var_ptr = *var_ptr_ptr;

                                            for(unsigned int i=0; i<_values_as_string.size(); ++i)
                                                var_ptr[i] = _values_as_string[i][0];
                                        }
                                    });
    LV::Type_Manager::register_type("unsigned int", {
                                        [](const std::string& _val)
                                        {
                                            for(unsigned int i=0; i<_val.size(); ++i)
                                            if(_val[i] < '0' || _val[i] > '9')
                                            return false;
                                            return true;
                                        },
                                        [](void* _variable_vptr, const LDS::Vector<std::string>& _values_as_string) { *((int*)_variable_vptr) = std::stoi(_values_as_string[0]); }
                                    });
    LV::Type_Manager::register_type("unsigned int*", {
                                        [](const std::string& _val)
                                        {
                                            for(unsigned int i=0; i<_val.size(); ++i)
                                            if(_val[i] < '0' || _val[i] > '9')
                                            return false;
                                            return true;
                                        },
                                        [](void* _variable_vptr, const LDS::Vector<std::string>& _values_as_string)
                                        {
                                            unsigned int** var_ptr_ptr = (unsigned int**)_variable_vptr;

                                            if(*var_ptr_ptr == nullptr)
                                                *var_ptr_ptr = new unsigned int[_values_as_string.size()];

                                            unsigned int* var_ptr = *var_ptr_ptr;

                                            for(unsigned int i=0; i<_values_as_string.size(); ++i)
                                                var_ptr[i] = std::stoi(_values_as_string[i]);
                                        }
                                    });
    LV::Type_Manager::register_type("bool", {
                                        [](const std::string& _val)
                                        {
                                            if(_val == "true" || _val == "false" || _val == "+" || _val == "-" || _val == "1" || _val == "0")
                                            return true;
                                            return false;
                                        },
                                        [](void* _variable_vptr, const LDS::Vector<std::string>& _values_as_string)
                                        {
                                            bool& var = *((bool*&)_variable_vptr);

                                            if(_values_as_string[0] == "true" || _values_as_string[0] == "+" || _values_as_string[0] == "1")
                                            var = true;
                                            else if(_values_as_string[0] == "false" || _values_as_string[0] == "-" || _values_as_string[0] == "0")
                                            var = false;
                                        }
                                    });
    LV::Type_Manager::register_type("bool*", {
                                        [](const std::string& _val)
                                        {
                                            if(_val == "true" || _val == "false" || _val == "+" || _val == "-" || _val == "1" || _val == "0")
                                            return true;
                                            return false;
                                        },
                                        [](void* _variable_vptr, const LDS::Vector<std::string>& _values_as_string)
                                        {
                                            bool** var_ptr_ptr = (bool**)_variable_vptr;

                                            if(*var_ptr_ptr == nullptr)
                                            *var_ptr_ptr = new bool[_values_as_string.size()];

                                            bool* var_ptr = *var_ptr_ptr;

                                            for(unsigned int i=0; i<_values_as_string.size(); ++i)
                                            {
                                                if(_values_as_string[i] == "true" || _values_as_string[i] == "+" || _values_as_string[i] == "1")
                                                var_ptr[i] = true;
                                                else if(_values_as_string[i] == "false" || _values_as_string[i] == "-" || _values_as_string[i] == "0")
                                                var_ptr[i] = false;
                                            }
                                        }
                                    });
    LV::Type_Manager::register_type("std::string", {
                                        [](const std::string& /*_val*/) { return true; },
                                        [](void* _variable_vptr, const LDS::Vector<std::string>& _values_as_string) {
                                            *((std::string*)_variable_vptr) = _values_as_string[0];
                                        }
                                    });
    LV::Type_Manager::register_type("float*", {
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
                                            float** var_ptr_ptr = (float**)_variable_vptr;

                                            if(*var_ptr_ptr == nullptr)
                                            *var_ptr_ptr = new float[_values_as_string.size()];

                                            float* var_ptr = *var_ptr_ptr;

                                            for(unsigned int i=0; i<_values_as_string.size(); ++i)
                                            var_ptr[i] = std::stof(_values_as_string[i]);
                                        }
                                    });
    LV::Type_Manager::register_type("std::string*", {
                                        [](const std::string& /*_val*/)
                                        {
                                            return true;
                                        },
                                        [](void* _variable_vptr, const LDS::Vector<std::string>& _values_as_string)
                                        {
                                            std::string** var_ptr_ptr = (std::string**)_variable_vptr;

                                            if(*var_ptr_ptr == nullptr)
                                            *var_ptr_ptr = new std::string[_values_as_string.size()];

                                            std::string* var_ptr = *var_ptr_ptr;

                                            for(unsigned int i=0; i<_values_as_string.size(); ++i)
                                            var_ptr[i] = _values_as_string[i];
                                        }
                                    });
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
                                        }
                                    });
    LV::Type_Manager::register_type("float", {
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
                                        [](void* _variable_vptr, const LDS::Vector<std::string>& _values_as_string) { *((float*)_variable_vptr) = std::stof(_values_as_string[0]); }
                                    });
    srand(time(nullptr));


    glm::vec3 cursor_position(0.0f, 0.0f, 0.0f);


    LST::File vertex_shader_file("Resources/Shaders/vertex_shader.shader");
    LST::File fragment_shader_file("Resources/Shaders/fragment_shader.shader");

    LV::MDL_Reader reader;

    LR::Window_Controller::create_window(1200, 800, "Generic Space Shooter Game");

    //    glEnable(GL_DEPTH_TEST);
    //    glClearColor(0.0,0.0,0.0,0.0);

    glEnable(GL_SCISSOR_TEST);
//    glScissor(20, 20, 200, 200);

    LR::Shader shader;

    LR::Camera_2D camera;
    camera.set_view_scale(1.0f);
    camera.set_position({600, 400, 0});
//    camera.set_position({0, 0, 0});

    LR::Renderer renderer;
    renderer.set_camera(&camera);
    renderer.set_shader(&shader);

    shader.init(vertex_shader_file.extract_block(), fragment_shader_file.extract_block());
    shader.set_texture_uniform("input_texture");
    shader.set_transform_matrix_uniform("transform_matrix");
    shader.set_projection_matrix_uniform("projection_matrix");

    LPhys::Collision_Detector_2D collision_detector;

    collision_detector.set_broad_phase(new LPhys::Space_Hasher_2D, 10);
    collision_detector.set_narrow_phase(new LPhys::Dynamic_Narrow_CD, 10);
    collision_detector.set_narrowest_phase(new LPhys::SAT_Narrowest_CD);

    LPhys::Collision_Resolver collision_resolver;
    collision_resolver.add_resolution(new LPhys::Collision_Resolution__Rigid_Body_2D);

    reader.parse_file("Resources/Textures/textures");
    reader.parse_file("Resources/Models/triangle");
    LR::Graphic_Resources_Manager graphics_resources_manager;

    graphics_resources_manager.load_resources(reader.get_stub("resources"));

    LEti::FPS_Timer timer;
    timer.set_max_dt(60.0f / 1000.0f);

    //  ~engine setup




    Test_Object_Stub test_object_stub;
    test_object_stub.draw_module = new LR::Default_Draw_Module_2D_Stub;
    test_object_stub.physics_module = new LPhys::Rigid_Body_2D__Stub;
    test_object_stub.assign_values(reader.get_stub("triangle"));
    test_object_stub.on_values_assigned();
    test_object_stub.draw_module->renderer = &renderer;
    test_object_stub.draw_module->graphic_resources_manager = &graphics_resources_manager;
    test_object_stub.scale = {20.0f, 20.0f, 1.0f};


    std::map<const LPhys::Rigid_Body_2D*, Test_Object*> objects_map;


    LDS::Vector<Test_Object*> test_objects;
    test_objects.resize(5);

    for(unsigned int i=0; i<5; ++i)
    {
        Test_Object* test_object = (Test_Object*)test_object_stub.construct();

        test_object->current_state().set_position( (glm::vec3(100.0f, 100.0f, 0.0f) * (float)i) + glm::vec3(400.0f, 200.0f, 0.0f));

        test_objects.push(test_object);

        objects_map.emplace(test_object->physics_module, test_object);

        collision_detector.register_object(test_object->physics_module);
    }


	///////////////// 2d collision test

	auto reset_func = [&]()
	{
        for(unsigned int i=0; i<5; ++i)
        {
            Test_Object* test_object = test_objects[i];
            test_object->current_state().set_position( (glm::vec3(100.0f, 100.0f, 0.0f) * (float)i) + glm::vec3(400.0f, 200.0f, 0.0f));

        }
	};
	reset_func();

	auto launch_func = [&]()
    {
        Test_Object* test_object = test_objects[0];
        test_object->physics_module->set_velocity({50.0f, 50.0f, 0.0f});
	};

	Grab grab;
    grab.camera = &camera;

//	LEti::Resource_Loader::load_object("flat_indicator_red", "Resources/Models/flat_indicator_red.mdl");
//	LEti::Resource_Loader::load_object("debug_frame", "Resources/Models/debug_frame.mdl");
//	LEti::Resource_Loader::load_object("debug_frame_red", "Resources/Models/debug_frame_red.mdl");

//	LEti::Resource_Loader::load_object("ind", "Resources/Models/intersection_point_indicator.mdl");
//	LEti::Object_2D ind;
//	ind.init("ind");

    LEti::FPS_Timer fps_timer;
    timer.set_max_dt(60.0f / 1000.0f);

//	LEti::Resource_Loader::load_object("text_field", "Resources/Models/text_field.mdl");
	//	LEti::Text_Field intersection_info_block;
	//	intersection_info_block.init("text_field");
	//	LEti::Text_Field tf_flat_co_speed;
	//	tf_flat_co_speed.init("text_field");
	//	tf_flat_co_speed.set_pos(0, 760, 0);

//	LEti::Text_Field fps_info_block;
//	fps_info_block.init(tf_stub);
//    //	fps_info_block.set_pos(1150, 770, 0);
//    fps_info_block.set_pos({10, 770, 0});
//    fps_info_block.set_scale(1);
//    fps_info_block.set_rotation_axis({0.0f, 0.0f, 1.0f});
//    fps_info_block.set_rotation_angle(0.0f);

//	LEti::Text_Field misc_info_block;
//	misc_info_block.init(tf_stub);
//	misc_info_block.set_pos({10, 10, 0});
//    misc_info_block.set_scale(1);
//    misc_info_block.set_rotation_axis({0.0f, 0.0f, 1.0f});
//    misc_info_block.set_rotation_angle(0.0f);
//    misc_info_block.set_text("abc");


	unsigned int fps_counter = 0;

	bool intersection_on_prev_frame = false;

    Color_Controll color_controll;

    while (!LR::Window_Controller::window_should_close())
    {
        timer.update();
        LR::Window_Controller::update();

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        for(unsigned int i=0; i<5; ++i)
        {
            Test_Object* object = test_objects[i];
            object->update_previous_state();
        }

        if (LR::Window_Controller::key_was_pressed(GLFW_KEY_K))
		{
		}
        if (LR::Window_Controller::is_key_down(GLFW_KEY_J))
		{
            //			flat_co.rotate_impulse(LEti::Math::HALF_PI * LR::Window_Controller::get_dt());
		}
        if (LR::Window_Controller::is_key_down(GLFW_KEY_L))
		{
            //			flat_co.rotate_impulse(-LEti::Math::HALF_PI * LR::Window_Controller::get_dt());
		}

        if(LR::Window_Controller::key_was_pressed(GLFW_KEY_R))
		{
			reset_func();
		}

        if(LR::Window_Controller::key_was_pressed(GLFW_KEY_SPACE))
		{
//			reset_func();
			launch_func();
		}

//		if (LR::Window_Controller::is_key_down(GLFW_KEY_W))
//			sandclock_co.apply_linear_impulse({0.0f, 10.0f, 0.0f});
//		if (LR::Window_Controller::is_key_down(GLFW_KEY_S))
//			sandclock_co.apply_linear_impulse({0.0f, -10.0f, 0.0f});
//		if (LR::Window_Controller::is_key_down(GLFW_KEY_A))
//			sandclock_co.apply_linear_impulse({-10.0f, 0.0f, 0.0f});
//		if (LR::Window_Controller::is_key_down(GLFW_KEY_D))
//			sandclock_co.apply_linear_impulse({10.0f, 0.0f, 0.0f});
//		if (LR::Window_Controller::is_key_down(GLFW_KEY_Q))
//			sandclock_co.apply_rotation(LEti::Math::QUARTER_PI);
//		if (LR::Window_Controller::is_key_down(GLFW_KEY_E))
//			sandclock_co.apply_rotation(-LEti::Math::QUARTER_PI);

//        if(LR::Window_Controller::mouse_wheel_rotation() != 0)
//		{
//			float additional_scale_per_rotation = 0.2f;

//            additional_scale_per_rotation *= -(float)(LR::Window_Controller::mouse_wheel_rotation());

//            camera.set_view_scale(camera.view_scale() + additional_scale_per_rotation);
//		}

        for(unsigned int i=0; i<5; ++i)
        {
            Test_Object* object = test_objects[i];
            object->update(timer.dt());
        }

//		LEti::Camera_2D::set_position(flat_co.get_pos());

        if(LR::Window_Controller::mouse_button_was_pressed(GLFW_MOUSE_BUTTON_1))
		{
			cursor_position.z = 0.0f;
            cursor_position.x = LR::Window_Controller::get_cursor_position().x;
            cursor_position.y = LR::Window_Controller::get_cursor_position().y;

			L_LOG("MOUSE_POS_LL", "Raw coords: " + std::to_string(cursor_position.x) + " " + std::to_string(cursor_position.y));

            cursor_position = camera.convert_window_coords(cursor_position);

			L_LOG("MOUSE_POS_LL", "Processed coords: " + std::to_string(cursor_position.x) + " " + std::to_string(cursor_position.y));
		}
        if(LR::Window_Controller::mouse_button_was_released(GLFW_MOUSE_BUTTON_1))
		{
			cursor_position.z = 0.0f;
            cursor_position.x = LR::Window_Controller::get_cursor_position().x;
            cursor_position.y = LR::Window_Controller::get_cursor_position().y;

			L_LOG("MOUSE_POS_LL", "Raw coords: " + std::to_string(cursor_position.x) + " " + std::to_string(cursor_position.y));

            cursor_position = camera.convert_window_coords(cursor_position);

			L_LOG("MOUSE_POS_LL", "Processed coords: " + std::to_string(cursor_position.x) + " " + std::to_string(cursor_position.y));
		}
        if(LR::Window_Controller::mouse_button_was_pressed(GLFW_MOUSE_BUTTON_2))
		{
			cursor_position.z = 0.0f;
            cursor_position.x = LR::Window_Controller::get_cursor_position().x;
            cursor_position.y = LR::Window_Controller::get_cursor_position().y;

            cursor_position = camera.convert_window_coords(cursor_position);
		}

		collision_detector.update();

        collision_resolver.resolve_all(collision_detector.get_collisions__models());



		std::string points_str;

		grab.update();

//        if(LR::Window_Controller::mouse_button_was_pressed(GLFW_MOUSE_BUTTON_1))
//		{
//			auto plist = collision_detector.get_collisions__points();

//			if(plist.size() != 0)
//				grab.grab(objects_map.at(plist.begin()->first), Grab::Type::drag);
//		}
//        if(LR::Window_Controller::mouse_button_was_pressed(GLFW_MOUSE_BUTTON_2))
//		{
//			auto plist = collision_detector.get_collisions__points();

//			if(plist.size() != 0)
//				grab.grab(objects_map.at(plist.begin()->first), Grab::Type::launch);
//		}
//        if(LR::Window_Controller::mouse_button_was_released(GLFW_MOUSE_BUTTON_1))
//		{
//			grab.release();
//		}
//        if(LR::Window_Controller::mouse_button_was_released(GLFW_MOUSE_BUTTON_2))
//		{
//			grab.release();
//		}



        LR::Window_Controller::swap_buffers();
	}

    for(unsigned int i=0; i<5; ++i)
    {
        Test_Object* object = test_objects[i];
        delete object;
    }

	return 0;
}














































