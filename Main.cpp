#include "Event_Controller.h"
#include "Shader.h"
#include "Camera_2D.h"
#include "Picture_Manager.h"

#include "Object_System/Text_Field.h"

#include <Object_System/Object_2D.h>

#include "Physics/Physical_Model_3D.h"
#include "Physics/Physical_Model_2D.h"

#include "Physics/Collision_Detector_2D.h"
#include "Physics/Collision_Resolver.h"
#include "Physics/Collision_Resolution__Rigid_Body_2D.h"

#include "Physics/Space_Hasher_2D.h"
#include "Physics/Default_Narrow_CD.h"
#include "Physics/Default_Narrowest_CD.h"
#include "Physics/SAT_Narrowest_CD.h"

#include "Timer.h"

#include "Object_System/Debug_Drawable_Frame.h"

#include <sstream>
#include <iomanip>

#include <chrono>
#include <thread>


#include <Object_System/Rigid_Body_2D.h>


#define DT LEti::Event_Controller::get_dt()

//class LEti::Rigid_Body_2D : public LEti::Object_2D
//{
//public:
//	glm::vec3 velocity;
//	float angular_velocity = 0.0f;
//	float mass = 1.0f;
//	std::string name;

//public:
//	void update(float _ratio = 1.0f) override
//	{
//		move(velocity * DT * _ratio);

//		rotate(angular_velocity * DT * (_ratio));

//		LEti::Object_2D::update();
//	}

//};

class CameraControll
{
private:
	bool m_grabbed = false;

public:
	void update()
	{
		if(LEti::Event_Controller::mouse_button_was_pressed(GLFW_MOUSE_BUTTON_3))
			m_grabbed = true;
		if(LEti::Event_Controller::mouse_button_was_released(GLFW_MOUSE_BUTTON_3))
			m_grabbed = false;

		if(!m_grabbed)
			return;

		LEti::Camera_2D::set_position(LEti::Camera_2D::position() + (glm::vec3(LEti::Window_Controller::get_cursor_stride().x, LEti::Window_Controller::get_cursor_stride().y, 0.0f) * LEti::Camera_2D::view_scale()));
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
	LEti::Rigid_Body_2D* grabbed_object = nullptr;
	glm::vec3 cursor_pos;

	glm::vec3 launch_from{0.0f, 0.0f, 0.0f};

	Type type = Type::none;

public:
	void update()
	{
		cursor_pos = {LEti::Window_Controller::get_cursor_position().x, LEti::Window_Controller::get_cursor_position().y, 0.0f};
		cursor_pos = LEti::Camera_2D::convert_window_coords(cursor_pos);

		if(!grabbed_object) return;
//		grabbed_object->velocity = {0.0f, 0.0f, 0.0f};
		grabbed_object->set_velocity(grabbed_object->get_pos() - grabbed_object->get_pos_prev());

		if(type == Type::drag)
			grabbed_object->set_pos(cursor_pos);
	}

	void grab(LEti::Rigid_Body_2D* _obj, Type _type)
	{
		if(type != Type::none) return;
		type = _type;
		grabbed_object = _obj;

		grabbed_object->set_velocity({0.0f, 0.0f, 0.0f});
		grabbed_object->set_angular_velocity(0.0f);

		if(_type == Type::drag)
			grabbed_object->set_pos(cursor_pos);

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

			grabbed_object->set_velocity(stride);
		}


		grabbed_object = nullptr;
		type = Type::none;
	}

};

#include "MDL_Reader.h"

int main()
{
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

//											*((bool*&)_variable_vptr) = _values_as_string[0] == "true" ? true : false;
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
										[](const std::string& _val) { return true; },
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

	CameraControll camera_controll;


	LV::MDL_Reader reader;
	reader.parse_file("Resources/Models/quad_new");
	reader.parse_file("Resources/Models/circleish");
	reader.parse_file("Resources/Models/sandclock");
	reader.parse_file("Resources/Models/text_field_new");

	LEti::Object_2D_Stub quad;
	quad.assign_values(reader.get_stub("quad"));
	LEti::Object_2D_Stub circleish;
	circleish.assign_values(reader.get_stub("circleish"));
	LEti::Object_2D_Stub sandclock;
	sandclock.assign_values(reader.get_stub("sandclock"));
	LEti::Text_Field_Stub tf_stub;
	tf_stub.assign_values(reader.get_stub("text_field"));

	LEti::Window_Controller::create_window(1200, 800, "Collision Test");

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_DEPTH_TEST);

	glEnable(GL_CULL_FACE);
	glCullFace(GL_CW);

	LEti::Shader::init_shader("Resources/Shaders/vertex_shader.shader", "Resources/Shaders/fragment_shader.shader");
	L_ASSERT(!(!LEti::Shader::is_valid()));
	LEti::Shader::set_texture_uniform_name("input_texture");
	LEti::Shader::set_transform_matrix_uniform_name("transform_matrix");
	LEti::Shader::set_projection_matrix_uniform_name("projection_matrix");

	LEti::Camera_2D::set_position({600, 400, 0.0f});
//	LEti::Camera_2D::set_view_scale(2.0f);

	LEti::Collision_Detector_2D collision_detector;

	collision_detector.set_broad_phase(new LEti::Space_Hasher_2D, 10);
	collision_detector.set_narrow_phase(new LEti::Default_Narrow_CD, 10);
	collision_detector.set_narrowest_phase(new LEti::SAT_Narrowest_CD);

	LEti::Collision_Resolver Collision_Resolver;
	Collision_Resolver.add_resolution(new LEti::Collision_Resolution__Rigid_Body_2D);

	reader.parse_file("Resources/Textures/textures");
	LEti::Picture_Manager::Picture_Autoload_Stub texture_autoload;
	texture_autoload.assign_values(reader.get_stub("textures"));


	LEti::Event_Controller::set_max_dt(60.0f / 1000.0f);


	L_CREATE_LOG_LEVEL("MOUSE_POS_LL");
	

	///////////////// 2d collision test

	LEti::Object_2D indicator;
	indicator.init(quad);
	indicator.draw_module()->set_texture(LEti::Picture_Manager::get_picture("purple_texture"));
	indicator.set_scale({3, 3, 1});
	indicator.set_pos({30, 30, 0});
	indicator.set_rotation_angle(LEti::Math::QUARTER_PI);

	LEti::Rigid_Body_2D circleish_co;
	circleish_co.init(circleish);
	circleish_co.set_mass(4.0f);
	circleish_co.set_pos({200, 600, 0});
	circleish_co.draw_module()->set_texture(LEti::Picture_Manager::get_picture("pyramid_texture"));

	LEti::Rigid_Body_2D sandclock_co;
	sandclock_co.init(sandclock);
	sandclock_co.set_mass(6.0f);
	sandclock_co.set_pos({200, 200, 0});
	sandclock_co.set_scale(20.0f);
	sandclock_co.draw_module()->set_texture(LEti::Picture_Manager::get_picture("white_texture"));

	LEti::Rigid_Body_2D flat_co;

	const unsigned int small_quads_amount = 4;

	LEti::Rigid_Body_2D small_quads[small_quads_amount];
	for(unsigned int i=0; i<small_quads_amount; ++i)
	{
		small_quads[i].init(quad);
		small_quads[i].set_mass(1.0f);
		small_quads[i].set_scale({25, 25, 1});
		small_quads[i].set_pos({1000, 100 + (70 * i), 0});
	}

//	flat_co.init("flat_co_model");
	flat_co.init(quad);
	flat_co.set_mass(4.0f);
	flat_co.set_pos({800, 400, 0});
	flat_co.draw_module()->set_texture(LEti::Picture_Manager::get_picture("white_texture"));

	auto reset_func = [&]()
	{
		//	TEST 2

		flat_co.set_pos({558, 386, 0});

		flat_co.set_velocity({0.0f, 0.0f, 0.0f});

		flat_co.set_scale(50);

		flat_co.set_rotation_angle(LEti::Math::QUARTER_PI);

		flat_co.set_rotation_axis({0.0f, 0.0f, 1.0f});

		flat_co.set_angular_velocity(0.0f);


		flat_co.update(0.0f);
		flat_co.update_previous_state();

		circleish_co.set_pos({200, 600, 0});
		circleish_co.set_angular_velocity(0.0f);
		circleish_co.set_velocity({0.0f, 0.0f, 0.0f});
		circleish_co.update(0.0f);
		circleish_co.update_previous_state();

		sandclock_co.set_pos({200, 200, 0});
		sandclock_co.set_angular_velocity(0.0f);
		sandclock_co.set_velocity({0.0f, 0.0f, 0.0f});
		sandclock_co.update(0.0f);
		sandclock_co.update_previous_state();


		for(unsigned int i=0; i<small_quads_amount; ++i)
		{
			small_quads[i].set_pos({1000, 100 + (70 * i), 0});
			small_quads[i].set_velocity({0.0f, 0.0f, 0.0f});
			small_quads[i].set_angular_velocity(0.0f);
			small_quads[i].set_rotation_angle(0.0f);

			small_quads[i].update(0.0f);
			small_quads[i].update_previous_state();
		}
	};
	reset_func();

	auto launch_func = [&]()
	{
//		small_quads[8].velocity = {20, -30, 0};
//		small_quads[9].velocity = {-20, -30, 0};

//		flat_co.set_velocity({-200, 0.0f, 0.0f});
		flat_co.set_angular_velocity(LEti::Math::PI);

//		small_quads[0].velocity = {197, -56, 0};

//		small_quads[8].set_angular_velocity = -LEti::Math::HALF_PI;
//		small_quads[9].set_angular_velocity = LEti::Math::HALF_PI;

//		small_quads[7].set_velocity({0, -1000, 0});
//		small_quads[5].set_velocity({0, 1000, 0});

//		small_quads[3].velocity = {0, -100, 0};
	};

	Grab grab;

	std::map<const LEti::Object_2D*, LEti::Rigid_Body_2D*> objects_map;
	objects_map.emplace(&flat_co, &flat_co);
	objects_map.emplace(&circleish_co, &circleish_co);
	objects_map.emplace(&sandclock_co, &sandclock_co);
	for(unsigned int i=0; i<small_quads_amount; ++i)
		objects_map.emplace(small_quads + i, small_quads + i);

//	LEti::Resource_Loader::load_object("flat_indicator_red", "Resources/Models/flat_indicator_red.mdl");
//	LEti::Resource_Loader::load_object("debug_frame", "Resources/Models/debug_frame.mdl");
//	LEti::Resource_Loader::load_object("debug_frame_red", "Resources/Models/debug_frame_red.mdl");

//	LEti::Resource_Loader::load_object("ind", "Resources/Models/intersection_point_indicator.mdl");
//	LEti::Object_2D ind;
//	ind.init("ind");

	LEti::Timer fps_timer;

//	LEti::Resource_Loader::load_object("text_field", "Resources/Models/text_field.mdl");
	//	LEti::Text_Field intersection_info_block;
	//	intersection_info_block.init("text_field");
	//	LEti::Text_Field tf_flat_co_speed;
	//	tf_flat_co_speed.init("text_field");
	//	tf_flat_co_speed.set_pos(0, 760, 0);

	LEti::Text_Field fps_info_block;
	fps_info_block.init(tf_stub);
	//	fps_info_block.set_pos(1150, 770, 0);
	fps_info_block.set_pos({10, 770, 0});

	LEti::Text_Field misc_info_block;
	misc_info_block.init(tf_stub);
	misc_info_block.set_pos({10, 10, 0});
	misc_info_block.set_text("");

	for(auto& co : objects_map)
	{
		co.second->update();
	}

	glm::vec3 cursor_position(0.0f, 0.0f, 0.0f);

	collision_detector.register_point(&cursor_position);

	for(auto& co : objects_map)
	{
		co.second->physics_module()->set_is_dynamic(true);
		collision_detector.register_object(co.second);
	}

	//	LEti::Collision_Detector_2D::register_point(&cursor_position);

	bool flat_co_enabled = true;

	unsigned int fps_counter = 0;

	LEti::Debug_Drawable_Frame frame;
	frame.init(quad);
	frame.set_pos({0, 0, 0});
	frame.set_scale({1, 1, 1});
	frame.draw_module()->set_texture(LEti::Picture_Manager::get_picture("ugly_color"));

	LEti::Debug_Drawable_Frame border_frame;
	border_frame.init(quad);
	border_frame.set_pos({0, 0, 0});
	border_frame.set_scale({1, 1, 1});
	border_frame.draw_module()->set_texture(LEti::Picture_Manager::get_picture("ugly_color"));
	border_frame.set_point(0, {1.0f, 1.0f, 0.0f}).set_sequence_element(0, 0)
			.set_point(1, {1200.0f, 1.0f, 0.0f}).set_sequence_element(1, 1)
			.set_point(2, {1200.0f, 800.0f, 0.0f}).set_sequence_element(2, 2)
			.set_point(3, {1.0f, 800.0f, 0.0f}).set_sequence_element(3, 3);

	bool intersection_on_prev_frame = false;

	while (!LEti::Window_Controller::window_should_close())
	{
		LEti::Event_Controller::update();
		LEti::Window_Controller::update();

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

//		if (LEti::Event_Controller::key_was_pressed(GLFW_KEY_TAB))
//			LEti::Camera::toggle_controll(LEti::Camera::get_controllable() ? false : true);
//		LEti::Camera::update(false, true);

		for(auto& co : objects_map)
		{
			co.second->update_previous_state();
		}

		if (LEti::Event_Controller::key_was_pressed(GLFW_KEY_K))
		{
		}
		if (LEti::Event_Controller::is_key_down(GLFW_KEY_J))
		{
			//			flat_co.rotate_impulse(LEti::Math::HALF_PI * LEti::Event_Controller::get_dt());
		}
		if (LEti::Event_Controller::is_key_down(GLFW_KEY_L))
		{
			//			flat_co.rotate_impulse(-LEti::Math::HALF_PI * LEti::Event_Controller::get_dt());
		}

		if(LEti::Event_Controller::key_was_pressed(GLFW_KEY_R))
		{
			reset_func();
		}

		if(LEti::Event_Controller::key_was_pressed(GLFW_KEY_SPACE))
		{
//			reset_func();
			launch_func();
		}

		if (LEti::Event_Controller::is_key_down(GLFW_KEY_W))
			sandclock_co.apply_linear_impulse({0.0f, 10.0f, 0.0f});
		if (LEti::Event_Controller::is_key_down(GLFW_KEY_S))
			sandclock_co.apply_linear_impulse({0.0f, -10.0f, 0.0f});
		if (LEti::Event_Controller::is_key_down(GLFW_KEY_A))
			sandclock_co.apply_linear_impulse({-10.0f, 0.0f, 0.0f});
		if (LEti::Event_Controller::is_key_down(GLFW_KEY_D))
			sandclock_co.apply_linear_impulse({10.0f, 0.0f, 0.0f});
		if (LEti::Event_Controller::is_key_down(GLFW_KEY_Q))
			sandclock_co.apply_rotation(LEti::Math::QUARTER_PI);
		if (LEti::Event_Controller::is_key_down(GLFW_KEY_E))
			sandclock_co.apply_rotation(-LEti::Math::QUARTER_PI);

		if(LEti::Event_Controller::mouse_wheel_rotation() != 0)
		{
			float additional_scale_per_rotation = 0.2f;

			additional_scale_per_rotation *= -(float)(LEti::Event_Controller::mouse_wheel_rotation());

			LEti::Camera_2D::set_view_scale(LEti::Camera_2D::view_scale() + additional_scale_per_rotation);
		}

		for(auto& co : objects_map)
		{
			LEti::Rigid_Body_2D& cco = *co.second;
			glm::vec3 vel = cco.velocity();

			if(cco.get_pos().y >= 800.0f)
			{
				cco.set_pos({cco.get_pos().x, 799.0f, 0.0f});
				vel.y *= -1;
			}
			else if(cco.get_pos().y <= 0.0f)
			{
				cco.set_pos({cco.get_pos().x, 1.0f, 0.0f});
				vel.y *= -1;
			}

			if(cco.get_pos().x >= 1200.0f)
			{
				cco.set_pos({1199.0f, cco.get_pos().y, 0.0f});
				vel.x *= -1;
			}
			else if(cco.get_pos().x <= 0.0f)
			{
				cco.set_pos({1.0f, cco.get_pos().y, 0.0f});
				vel.x *= -1;
			}

			cco.set_velocity(vel);
		}

		for(auto& co : objects_map)
		{
			co.second->update();
		}

//		LEti::Camera_2D::set_position(flat_co.get_pos());

		if(LEti::Event_Controller::mouse_button_was_pressed(GLFW_MOUSE_BUTTON_1))
		{
			cursor_position.z = 0.0f;
			cursor_position.x = LEti::Window_Controller::get_cursor_position().x;
			cursor_position.y = LEti::Window_Controller::get_cursor_position().y;

			L_LOG("MOUSE_POS_LL", "Raw coords: " + std::to_string(cursor_position.x) + " " + std::to_string(cursor_position.y));

			cursor_position = LEti::Camera_2D::convert_window_coords(cursor_position);

			L_LOG("MOUSE_POS_LL", "Processed coords: " + std::to_string(cursor_position.x) + " " + std::to_string(cursor_position.y));
		}
		if(LEti::Event_Controller::mouse_button_was_released(GLFW_MOUSE_BUTTON_1))
		{
			cursor_position.z = 0.0f;
			cursor_position.x = LEti::Window_Controller::get_cursor_position().x;
			cursor_position.y = LEti::Window_Controller::get_cursor_position().y;

			L_LOG("MOUSE_POS_LL", "Raw coords: " + std::to_string(cursor_position.x) + " " + std::to_string(cursor_position.y));

			cursor_position = LEti::Camera_2D::convert_window_coords(cursor_position);

			L_LOG("MOUSE_POS_LL", "Processed coords: " + std::to_string(cursor_position.x) + " " + std::to_string(cursor_position.y));
		}
		if(LEti::Event_Controller::mouse_button_was_pressed(GLFW_MOUSE_BUTTON_2))
		{
			cursor_position.z = 0.0f;
			cursor_position.x = LEti::Window_Controller::get_cursor_position().x;
			cursor_position.y = LEti::Window_Controller::get_cursor_position().y;

			cursor_position = LEti::Camera_2D::convert_window_coords(cursor_position);
		}

		auto draw_frame = [](LEti::Debug_Drawable_Frame& _frame, const LEti::Physical_Model_2D::Imprint& _pm)->void
		{
			_frame.clear_points().clear_sequence();

			unsigned int counter = 0;
			for(unsigned int i=0; i<_pm.get_polygons_count(); ++i)
			{
				for(unsigned int j=0; j<3; ++j)
				{
					_frame.set_point(counter, _pm[i][j]).set_sequence_element(counter, counter);
					++counter;
				}
			}
			_frame.update();
			_frame.draw();
		};

		for(auto& co : objects_map)
		{
//			co.second->apply_linear_impulse(glm::vec3(0.0f, -9.8f, 0.0f));
		}

		collision_detector.update();

//		LEti::Default_Narrow_CD::Collision_Data_List__Models list = collision_detector.get_collisions__models();

		Collision_Resolver.resolve_all(collision_detector.get_collisions__models());



		std::string points_str;

		grab.update();

		camera_controll.update();

//		collision_detector.unregister_point(&cursor_position);

		if(LEti::Event_Controller::mouse_button_was_pressed(GLFW_MOUSE_BUTTON_1))
		{
			auto plist = collision_detector.get_collisions__points();

			if(plist.size() != 0)
				grab.grab(objects_map.at(plist.begin()->first), Grab::Type::drag);
		}
		if(LEti::Event_Controller::mouse_button_was_pressed(GLFW_MOUSE_BUTTON_2))
		{
			auto plist = collision_detector.get_collisions__points();

			if(plist.size() != 0)
				grab.grab(objects_map.at(plist.begin()->first), Grab::Type::launch);
		}
		if(LEti::Event_Controller::mouse_button_was_released(GLFW_MOUSE_BUTTON_1))
		{
			grab.release();
		}
		if(LEti::Event_Controller::mouse_button_was_released(GLFW_MOUSE_BUTTON_2))
		{
			grab.release();
		}

		for(auto& co : objects_map)
		{
//			draw_frame(frame, co.second->physics_module()->get_physical_model()->create_imprint());
		}
		border_frame.update();
		border_frame.draw();

		if(intersection_on_prev_frame)
		{
			misc_info_block.set_text(points_str.c_str());
			misc_info_block.draw();
		}

		for(auto& co : objects_map)
		{
			co.second->draw();
		}

		//		draw_frame(frame, flat_co.physics_module()->get_physical_model()->create_imprint());
		//		draw_frame(frame, flat_co_2.physics_module()->get_physical_model()->create_imprint());
		//		draw_frame(frame, flat_co_3.physics_module()->get_physical_model()->create_imprint());


		//		intersection_info_block.set_text(std::to_string(list.size()).c_str());
		//		intersection_info_block.draw();

		++fps_counter;
		fps_timer.update();
		if(!fps_timer.is_active())
		{
			fps_timer.start(1.0f);
			fps_info_block.set_text((std::to_string(fps_counter)).c_str());
			fps_counter = 0;
		}
		fps_info_block.draw();

		LEti::Window_Controller::swap_buffers();
	}

	return 0;
}














































