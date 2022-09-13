#include "Event_Controller.h"
#include "Shader.h"
#include "Camera.h"
#include "Resource_Loader.h"

//#include "Object.h"
#include "Object_System/Text_Field.h"

#include <include/Object_System/Object_2D.h>

#include "Physics/Physical_Model_3D.h"
#include "Physics/Physical_Model_2D.h"

#include "Message_Translator.h"

#include "Physics/Space_Splitter_2D.h"
#include "Physics/Space_Hasher_2D.h"
#include "Physics/Default_Narrow_CD.h"
#include "Physics/Default_Narrowest_CD.h"

#include "Timer.h"

#include "Object_System/Debug_Drawable_Frame.h"

#include <sstream>
#include <iomanip>

#include <chrono>
#include <thread>


#define DT LEti::Event_Controller::get_dt()

struct On_Button_Pressed_Msg
{
	DEFINE_TYPE("obpf");
	unsigned int btn = 0;
	On_Button_Pressed_Msg(unsigned int _btn) : btn(_btn) { }
};

class Moving_Object : public LEti::Object_2D
{
public:
	float velocity = 0.0f;
	glm::vec3 movement_direction{1.0f, 0.0f, 0.0f};
	float angular_velocity = 0.0f;
	float mass = 1.0f;
	std::string name;

public:
	void rotate_impulse(float _angle)
	{
		glm::mat4x4 rm = glm::rotate(_angle, glm::vec3(0.0f, 0.0f, 1.0f));
		movement_direction = rm * glm::vec4(movement_direction, 1.0f);
	}

	glm::vec3 get_impulse_of_attached_point(const glm::vec3& _point) const
	{
		glm::vec3 movement_impulse = physics_module()->get_physical_model()->center_of_mass() - physics_module()->get_physical_model_prev_state()->center_of_mass();
		movement_impulse /= DT;

		glm::mat4x4 inversed_rotation_matrix = get_rotation_matrix_for_time_ratio(0.0f) / get_rotation_matrix_for_time_ratio(1.0f);
		glm::vec3 center_to_particle_vec = _point - physics_module()->get_physical_model()->center_of_mass();
		glm::vec3 center_to_particle_vec_prev = inversed_rotation_matrix * glm::vec4(center_to_particle_vec, 1.0f);

		glm::vec3 rotation_impulse = center_to_particle_vec - center_to_particle_vec_prev;

		float radius = LEti::Math::vector_length(center_to_particle_vec);
		float points_linear_velocity = LEti::Math::DOUBLE_PI * fabs(angular_velocity) * radius;
		LEti::Math::extend_vector_to_length(rotation_impulse, points_linear_velocity);

		return movement_impulse + rotation_impulse;
	}

	void apply_impulse(const glm::vec3& _impulse, const glm::vec3& _to_point)
	{
		if(LEti::Math::floats_are_equal(DT, 0.0f)) return;

		if(LEti::Math::floats_are_equal(LEti::Math::vector_length(_impulse), 0.0f))
			return;

		glm::vec3 prev_point_impulse = get_impulse_of_attached_point(_to_point);
		glm::vec3 result_impulse = prev_point_impulse + _impulse;

		glm::vec3 point_to_center_vec = physics_module()->get_physical_model()->center_of_mass() - _to_point;
		float movement_impulse_ratio = fabs(LEti::Math::angle_cos_between_vectors(point_to_center_vec, result_impulse));

		angular_velocity = LEti::Math::vector_length(result_impulse) / LEti::Math::vector_length(point_to_center_vec);

		if(LEti::Math::normalize(result_impulse, point_to_center_vec).z < 0.0f)
			angular_velocity *= -1;

		angular_velocity *= 1.0f - movement_impulse_ratio;

		movement_direction = result_impulse * movement_impulse_ratio;
		velocity = LEti::Math::vector_length(movement_direction);
		LEti::Math::shrink_vector_to_1(movement_direction);
	}

	void update(float _ratio = 1.0f) override
	{
		glm::vec3 trajectory{0.0f, 0.0f, 0.0f};
		trajectory.x = movement_direction.x * velocity * LEti::Event_Controller::get_dt() * (_ratio);
		trajectory.y = movement_direction.y * velocity * LEti::Event_Controller::get_dt() * (_ratio);
		move(trajectory);

		rotate(angular_velocity * DT * (_ratio));

		LEti::Object_2D::update();
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
	Moving_Object* grabbed_object = nullptr;
	glm::vec3 cursor_pos;

	glm::vec3 launch_from{0.0f, 0.0f, 0.0f};

	Type type = Type::none;

public:
	void update()
	{
		cursor_pos = {LEti::Window_Controller::get_cursor_position().x, LEti::Window_Controller::get_cursor_position().y, 0.0f};
		if(!grabbed_object) return;
		grabbed_object->velocity = 0.0f;

		if(type == Type::drag)
			grabbed_object->set_pos(cursor_pos);
	}

	void grab(Moving_Object* _obj, Type _type)
	{
		if(type != Type::none) return;
		type = _type;
		grabbed_object = _obj;

		grabbed_object->velocity = 0.0f;
		grabbed_object->angular_velocity = 0.0f;

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
			glm::vec3 stride = grabbed_object->get_pos() - grabbed_object->get_pos_prev();

			grabbed_object->velocity = LEti::Math::vector_length(stride) / DT;

			LEti::Math::shrink_vector_to_1(stride);
			grabbed_object->movement_direction = stride;
		}
		else if(type == Type::launch)
		{
			glm::vec3 stride = (cursor_pos - launch_from);
			grabbed_object->apply_impulse(stride, launch_from);
		}

		grabbed_object = nullptr;
		type = Type::none;
	}

};

int main()
{
	LEti::Window_Controller::create_window(1200, 800, "Collision Test");

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_DEPTH_TEST);

	glEnable(GL_CULL_FACE);
	glCullFace(GL_CW);

	LEti::Shader::init_shader("Resources/Shaders/vertex_shader.shader", "Resources/Shaders/fragment_shader.shader");
	ASSERT(!LEti::Shader::is_valid());
	LEti::Shader::set_texture_uniform_name("input_texture");
	LEti::Shader::set_transform_matrix_uniform_name("transform_matrix");
	LEti::Shader::set_projection_matrix_uniform_name("projection_matrix");

	LEti::Camera::setup_orthographic_matrix();

	LEti::Camera::set_fov_and_max_distance(LEti::Math::HALF_PI, 50.0f);
	LEti::Camera::set_camera_data({ 0.0f, 2.0f, 2.0f }, { 0.0f, -2.0f, -1.0f });

	LEti::Space_Splitter_2D::set_broad_phase<LEti::Space_Hasher_2D>();
	LEti::Space_Splitter_2D::set_narrow_phase<LEti::Default_Narrow_CD>();
	LEti::Space_Splitter_2D::set_narrowest_phase<LEti::Default_Narrowest_CD>();
//	LEti::Space_Splitter_2D::set_precision(10);
	LEti::Space_Splitter_2D::get_broad_phase()->set_precision(10);
	LEti::Space_Splitter_2D::get_narrow_phase()->set_precision(10);

	LEti::Resource_Loader::init();

	LEti::Resource_Loader::load_object("textures", "Resources/Textures/textures.mdl");

	LEti::Event_Controller::set_max_dt(60.0f / 1000.0f);

	

	///////////////// 2d collision test

	LEti::Resource_Loader::load_object("flat_co_model", "Resources/Models/quad.mdl");
//	LEti::Resource_Loader::load_object("flat_co_model", "Resources/Models/triangle.mdl");


//	while(true)
//	{
//		LEti::Object_2D* test = new LEti::Object_2D;
//		test->init("flat_co_model");
//		test->remove_physics_module();
//		test->create_physics_module();
//		test->physics_module();
//		test->remove_physics_module();
//		test->remove_draw_module();
//		test->
//		delete test;
//	}

	Moving_Object flat_co;
//	flat_co.speed = 200.0f;
//	flat_co.angle = LEti::Math::PI;

	Moving_Object flat_co_2;

	Moving_Object flat_co_3;

	flat_co.init("flat_co_model");
	flat_co.set_pos({800, 400, 0});
	flat_co.draw_module()->set_texture("white_texture");
	flat_co.name = "white";

//	LEti::Object_2D flat_co_foreshadow;
//	flat_co_foreshadow.init("flat_co_model");
//	flat_co_foreshadow.set_pos(50, 400, 0);

	flat_co_2.init("flat_co_model");
	flat_co_2.set_scale({20.0f, 20.0f, 1.0f});
	flat_co_2.set_pos({400, 600, 0});
//	flat_co_2.speed = 0.0f;
//	flat_co_2.angle = 2.53f;

	flat_co_3.init("flat_co_model");
	flat_co_3.set_scale({50.0f, 50.0f, 1.0f});
	flat_co_3.set_pos({400, 400, 0});
	flat_co_3.name = "red_big";
//	flat_co_3.speed = 200.0f;
//	flat_co_3.angle = 0.0f;

//	flat_co.remove_draw_module();

	int delay = 0;

	float co3_init_x = 560;
//	float co3_init_y = 340;
	float co3_init_y = 330;

	auto print_info_func = [&]()
	{
		auto print_co_data = [](const Moving_Object& _obj)
		{
			std::cout << _obj.name << ":\n"
					  << "\tnovement direction: " << _obj.movement_direction.x << "\t" << _obj.movement_direction.y << "\n"
					  << "\tvelocity: " << _obj.velocity << "\n"
					  << "\tangular velocity: " << _obj.angular_velocity << "\n\n";
		};

		print_co_data(flat_co);
		print_co_data(flat_co_3);
	};

	float co_spd= 150.0f;
	auto reset_func = [&]()
	{
		delay = 0;


		flat_co.set_pos({700, 400, 0});
		flat_co_3.set_pos({co3_init_x, co3_init_y, 0});
		flat_co_2.set_pos({800, 700, 0});

		flat_co.velocity = 0;
		flat_co_3.velocity = 0;
		flat_co_2.velocity = 0;

		flat_co.movement_direction = {-3.0f, 0.0f, 0.0f};
		LEti::Math::shrink_vector_to_1(flat_co.movement_direction);
		flat_co_3.movement_direction = {3.0f, 0.0f, 0.0f};
		LEti::Math::shrink_vector_to_1(flat_co_3.movement_direction);
		flat_co_2.movement_direction = {3.0f, -1.0f, 0.0f};
		LEti::Math::shrink_vector_to_1(flat_co_2.movement_direction);

		flat_co.set_scale(50);
		flat_co_3.set_scale(50);
		flat_co_3.set_scale({25.0f, 75.0f, 1.0f});
//		flat_co_2.set_scale(50);

		flat_co.set_rotation_angle(LEti::Math::QUARTER_PI);
		flat_co_3.set_rotation_angle(0.0f);
		flat_co_2.set_rotation_angle(0.0f);

		flat_co.set_rotation_axis({0.0f, 0.0f, 1.0f});
		flat_co_3.set_rotation_axis({0.0f, 0.0f, 1.0f});
		flat_co_2.set_rotation_axis({0.0f, 0.0f, 1.0f});

		flat_co.angular_velocity = 0.0f;
		flat_co_3.angular_velocity = 0.0f;
		flat_co_2.angular_velocity = 0.0f;


		flat_co.update(0.0f);
		flat_co_2.update(0.0f);
		flat_co_3.update(0.0f);
		flat_co.update_previous_state();
		flat_co_2.update_previous_state();
		flat_co_3.update_previous_state();
	};
	reset_func();

	auto launch_func = [&]()
	{
		//	TEST 1

//		flat_co_3.apply_impulse({200, 0, 0}, {co3_init_x + 10, co3_init_y + 50, 0});

		//	~TEST 1

		//	TEST 2

		flat_co_3.movement_direction = {1.0f, 0.0f, 0.0f};
		flat_co_3.velocity = 22.0f;
		flat_co_3.angular_velocity = -LEti::Math::HALF_PI;

		//	~TEST 2

		//	TEST 3

//		flat_co.velocity = 100.0f;

		//	~TEST 3

		std::cout << "-----  after launch movement info  -----\n";
		print_info_func();

		//
	};

	Grab grab;

	std::map<const LEti::Object_2D*, Moving_Object*> objects_map;
	objects_map.emplace(&flat_co, &flat_co);
	objects_map.emplace(&flat_co_2, &flat_co_2);
	objects_map.emplace(&flat_co_3, &flat_co_3);

	LEti::Resource_Loader::load_object("flat_indicator_red", "Resources/Models/flat_indicator_red.mdl");
	LEti::Resource_Loader::load_object("debug_frame", "Resources/Models/debug_frame.mdl");
	LEti::Resource_Loader::load_object("debug_frame_red", "Resources/Models/debug_frame_red.mdl");

	LEti::Resource_Loader::load_object("ind", "Resources/Models/intersection_point_indicator.mdl");
	LEti::Object_2D ind;
	ind.init("ind");

	LEti::Timer fps_timer;

	LEti::Resource_Loader::load_object("text_field", "Resources/Models/text_field.mdl");
//	LEti::Text_Field intersection_info_block;
//	intersection_info_block.init("text_field");
//	LEti::Text_Field tf_flat_co_speed;
//	tf_flat_co_speed.init("text_field");
//	tf_flat_co_speed.set_pos(0, 760, 0);

	LEti::Text_Field fps_info_block;
	fps_info_block.init("text_field");
//	fps_info_block.set_pos(1150, 770, 0);
	fps_info_block.set_pos({10, 770, 0});

	LEti::Text_Field impulses_summary_infoblock;
	impulses_summary_infoblock.init("text_field");
	impulses_summary_infoblock.set_pos({10, 10, 0});

	flat_co.update();
	flat_co_2.update();
	flat_co_3.update();

	glm::vec3 cursor_position;

	flat_co.physics_module()->set_is_dynamic(true);
	flat_co_2.physics_module()->set_is_dynamic(true);
	flat_co_3.physics_module()->set_is_dynamic(true);
	LEti::Space_Splitter_2D::register_object(&flat_co);
//	LEti::Space_Splitter_2D::register_object(&flat_co_2);
	LEti::Space_Splitter_2D::register_object(&flat_co_3);

//	LEti::Space_Splitter_2D::register_point(&cursor_position);

	bool flat_co_enabled = true;

	unsigned int fps_counter = 0;

	LEti::Debug_Drawable_Frame frame;
	frame.init("debug_frame");
	LEti::Debug_Drawable_Frame frame_red;
	frame_red.init("debug_frame_red");

	bool intersection_on_prev_frame = false;

	while (!LEti::Window_Controller::window_should_close())
	{
		LEti::Event_Controller::update();
		LEti::Window_Controller::update();

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		if (LEti::Event_Controller::key_was_pressed(GLFW_KEY_TAB))
			LEti::Camera::toggle_controll(LEti::Camera::get_controllable() ? false : true);
		LEti::Camera::update(false, true);

		flat_co.update_previous_state();
		flat_co_2.update_previous_state();
		flat_co_3.update_previous_state();

		if (LEti::Event_Controller::key_was_pressed(GLFW_KEY_I))
		{
			print_info_func();
		}
		if (LEti::Event_Controller::key_was_pressed(GLFW_KEY_K))
		{
		}
		if (LEti::Event_Controller::is_key_down(GLFW_KEY_J))
		{
			flat_co.rotate_impulse(LEti::Math::HALF_PI * LEti::Event_Controller::get_dt());
		}
		if (LEti::Event_Controller::is_key_down(GLFW_KEY_L))
		{
			flat_co.rotate_impulse(-LEti::Math::HALF_PI * LEti::Event_Controller::get_dt());
		}

		if(LEti::Event_Controller::key_was_pressed(GLFW_KEY_C))
		{
			flat_co.set_pos({0, 0, 0});
			flat_co_2.set_pos({0, 0, 0});
			flat_co_3.set_pos({0, 0, 0});
		}

		if(LEti::Event_Controller::key_was_pressed(GLFW_KEY_R))
		{
			reset_func();
		}
		if(LEti::Event_Controller::key_was_pressed(GLFW_KEY_SPACE))
		{
			reset_func();
			launch_func();
		}
		if(LEti::Event_Controller::key_was_pressed(GLFW_KEY_EQUAL))
		{
			delay += 100;
		}
		if(LEti::Event_Controller::key_was_pressed(GLFW_KEY_MINUS))
		{
			delay -= 100;
			if(delay < 0) delay = 0;
		}

		if(LEti::Event_Controller::is_key_down(GLFW_KEY_A))
		{
			flat_co.move({-(co_spd * DT), 0.0f, 0.0f});
		}
		if(LEti::Event_Controller::is_key_down(GLFW_KEY_D))
		{
			flat_co.move({ (co_spd * DT), 0.0f, 0.0f});
		}
		if(LEti::Event_Controller::is_key_down(GLFW_KEY_W))
		{
			flat_co.move({0.0f,  (co_spd * DT), 0.0f});
		}
		if(LEti::Event_Controller::is_key_down(GLFW_KEY_S))
		{
			flat_co.move({0.0f, -(co_spd * DT), 0.0f});
		}
		if(LEti::Event_Controller::is_key_down(GLFW_KEY_Q))
		{
//			for(auto& co : objects_map)
//			{
//				Moving_Object& cco = *co.second;
//				if(cco.angular_velocity < 0.0f)
//				{
//					cco.angular_velocity += LEti::Math::PI * DT;
//					if(cco.angular_velocity > 0.0f) cco.angular_velocity = 0.0f;
//				}
//				else
//				{
//					cco.angular_velocity -= LEti::Math::PI * DT;
//					if(cco.angular_velocity < 0.0f) cco.angular_velocity = 0.0f;
//				}
//			}
			if(flat_co.angular_velocity < 0.0f)
			{
				flat_co.angular_velocity += LEti::Math::PI * DT;
				if(flat_co.angular_velocity > 0.0f) flat_co.angular_velocity = 0.0f;
			}
			else
			{
				flat_co.angular_velocity -= LEti::Math::PI * DT;
				if(flat_co.angular_velocity < 0.0f) flat_co.angular_velocity = 0.0f;
			}
		}
		if(LEti::Event_Controller::is_key_down(GLFW_KEY_E))
		{
//			for(auto& co : objects_map)
//			{
//				Moving_Object& cco = *co.second;
//				if(cco.angular_velocity < 0.0f)
//				{
//					cco.angular_velocity -= LEti::Math::PI* DT;
//				}
//				else
//				{
//					cco.angular_velocity += LEti::Math::PI * DT;
//				}
//			}
			if(flat_co.angular_velocity < 0.0f)
			{
				flat_co.angular_velocity -= LEti::Math::PI* DT;
			}
			else
			{
				flat_co.angular_velocity += LEti::Math::PI * DT;
			}
		}

		for(auto& co : objects_map)
		{
			Moving_Object& cco = *co.second;
			if(cco.get_pos().y >= 800.0f)
			{
				cco.set_pos({cco.get_pos().x, 799.0f, 0.0f});
//				cco.angle = LEti::Math::DOUBLE_PI - cco.angle;
				cco.movement_direction.y *= -1;
			}
			else if(cco.get_pos().y <= 0.0f)
			{
				cco.set_pos({cco.get_pos().x, 1.0f, 0.0f});
//				cco.angle = LEti::Math::DOUBLE_PI - cco.angle;
				cco.movement_direction.y *= -1;
			}

			if(cco.get_pos().x >= 1200.0f)
			{
				cco.set_pos({1199.0f, cco.get_pos().y, 0.0f});
//				cco.angle = LEti::Math::DOUBLE_PI - cco.angle + LEti::Math::PI;
				cco.movement_direction.x *= -1;
			}
			else if(cco.get_pos().x <= 0.0f)
			{
				cco.set_pos({1.0f, cco.get_pos().y, 0.0f});
//				cco.angle = LEti::Math::DOUBLE_PI - cco.angle + LEti::Math::PI;
				cco.movement_direction.x *= -1;
			}
		}

		flat_co.update();
		flat_co_2.update();
		flat_co_3.update();

		if(LEti::Event_Controller::mouse_button_was_pressed(GLFW_MOUSE_BUTTON_1))
		{
			cursor_position.z = 0.0f;
			cursor_position.x = LEti::Window_Controller::get_cursor_position().x;
			cursor_position.y = LEti::Window_Controller::get_cursor_position().y;
			LEti::Space_Splitter_2D::register_point(&cursor_position);
		}
		if(LEti::Event_Controller::mouse_button_was_pressed(GLFW_MOUSE_BUTTON_2))
		{
			cursor_position.z = 0.0f;
			cursor_position.x = LEti::Window_Controller::get_cursor_position().x;
			cursor_position.y = LEti::Window_Controller::get_cursor_position().y;
			LEti::Space_Splitter_2D::register_point(&cursor_position);
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
		auto draw_frames_relative_to_other = [&](const LEti::Object_2D& _moving_1, const LEti::Object_2D& _moving_2)->void
		{
//				frame.clear_points().clear_sequence();

			LEti::Physical_Model_2D::Imprint pm = *_moving_1.physics_module()->get_physical_model_prev_state();

			draw_frame(frame, pm);


			glm::mat4x4 fake_movement_matrix{
				1.0f, 0.0f, 0.0f, 0.0f,
				0.0f, 1.0f, 0.0f, 0.0f,
				0.0f, 0.0f, 1.0f, 0.0f,
				900.0f, 600.0f, 0.0f, 1.0f
			};
			glm::mat4x4 fake_default_matrix{
				1.0f, 0.0f, 0.0f, 0.0f,
				0.0f, 1.0f, 0.0f, 0.0f,
				0.0f, 0.0f, 1.0f, 0.0f,
				0.0f, 0.0f, 0.0f, 1.0f
			};
			glm::mat4x4 fake_scale_matrix{
				30.0f, 0.0f, 0.0f, 0.0f,
				0.0f, 30.0f, 0.0f, 0.0f,
				0.0f, 0.0f, 1.0f, 0.0f,
				0.0f, 0.0f, 0.0f, 1.0f
			};

			glm::vec3 pos_diff_vector_prev = _moving_1.get_pos_prev() - _moving_2.get_pos_prev();
			pos_diff_vector_prev = _moving_2.get_rotation_matrix_inversed_for_time_ratio(0.0f) * glm::vec4(pos_diff_vector_prev, 1.0f);

			glm::mat4x4 diff_pos_prev = fake_default_matrix;
			diff_pos_prev *= fake_movement_matrix;
			diff_pos_prev[3][0] += pos_diff_vector_prev[0];
			diff_pos_prev[3][1] += pos_diff_vector_prev[1];
			glm::mat4x4 diff_rotation_prev = _moving_1.get_rotation_matrix_for_time_ratio(0.0f) / _moving_2.get_rotation_matrix_for_time_ratio(0.0f);
			glm::mat4x4 diff_scale_prev = _moving_1.get_scale_matrix_for_time_ratio(0.0f);


			glm::vec3 pos_diff_vector = _moving_1.get_pos() - _moving_2.get_pos();
			pos_diff_vector = _moving_2.get_rotation_matrix_inversed_for_time_ratio(1.0f) * glm::vec4(pos_diff_vector, 1.0f);

			glm::mat4x4 diff_pos = fake_default_matrix;
			diff_pos *= fake_movement_matrix;
			diff_pos[3][0] += pos_diff_vector[0];
			diff_pos[3][1] += pos_diff_vector[1];
			glm::mat4x4 diff_rotation = _moving_1.get_rotation_matrix_for_time_ratio(1.0f) / _moving_2.get_rotation_matrix_for_time_ratio(1.0f);
			glm::mat4x4 diff_scale = _moving_1.get_scale_matrix_for_time_ratio(1.0f) * (_moving_2.get_scale_matrix_for_time_ratio(1.0f) / _moving_2.get_scale_matrix_for_time_ratio(0.0f));

			LEti::Physical_Model_2D::Imprint initial_second_pm = *_moving_2.physics_module()->get_physical_model_prev_state();
			initial_second_pm.update(fake_movement_matrix, fake_default_matrix, _moving_2.get_scale_matrix_for_time_ratio(0.0f));


			draw_frame(frame_red, initial_second_pm);

			pm.update(diff_pos_prev, diff_rotation_prev, diff_scale_prev);
			draw_frame(frame, pm);

			pm.update(diff_pos, diff_rotation, diff_scale);
			draw_frame(frame_red, pm);

//			draw_frame(frame, *_moving_2.physics_module()->get_physical_model_prev_state());

		};
		draw_frames_relative_to_other(flat_co, flat_co_3);

		draw_frame(frame, flat_co.physics_module()->get_physical_model()->create_imprint());
		draw_frame(frame, flat_co_2.physics_module()->get_physical_model()->create_imprint());
		draw_frame(frame, flat_co_3.physics_module()->get_physical_model()->create_imprint());

		LEti::Space_Splitter_2D::update();

		glm::vec3 fcopos = flat_co.get_pos();
		glm::vec3 fco3pos = flat_co_3.get_pos();


		auto get_particle_velocity = [](const Moving_Object& _obj, const glm::vec3& _particle)->float
		{
			glm::vec3 movement_impulse = _obj.physics_module()->get_physical_model()->center_of_mass() - _obj.physics_module()->get_physical_model_prev_state()->center_of_mass();

			glm::mat4x4 inversed_rotation_matrix = _obj.get_rotation_matrix_for_time_ratio(0.0f) / _obj.get_rotation_matrix_for_time_ratio(1.0f);
			glm::vec3 center_to_particle_vec = _particle - _obj.physics_module()->get_physical_model()->center_of_mass();
			glm::vec3 center_to_particle_vec_prev = inversed_rotation_matrix * glm::vec4(center_to_particle_vec, 1.0f);

			glm::vec3 rotation_impulse = center_to_particle_vec - center_to_particle_vec_prev;

			return LEti::Math::vector_length(movement_impulse) + LEti::Math::vector_length(rotation_impulse);
//			return _obj.velocity;
		};


		const LEti::Default_Narrow_CD::Collision_Data_List__Models& list = LEti::Space_Splitter_2D::get_collisions__models();

		auto it = list.begin();
		while(it != list.end())
		{
			if(it->time_of_intersection_ratio < 0.0001f)
			{
				++it;
				continue;
			}

			Moving_Object& f = *(objects_map.at(it->first));
			Moving_Object& s = *(objects_map.at(it->second));

			std::cout << "-----  before collision movement info  -----\n";
			print_info_func();

			float dt_before_collision = DT * it->time_of_intersection_ratio;

			f.revert_to_previous_state();
			f.update(it->time_of_intersection_ratio);
			s.revert_to_previous_state();
			s.update(it->time_of_intersection_ratio);

			auto get_new_rotation_data_for_model = [&it](const Moving_Object& f, const Moving_Object& s, const glm::vec3& _f_normal, const glm::vec3& _s_normal, const glm::vec3& _particle)->std::pair<glm::vec3, float>
			{
				auto get_new_rotation_angle = [](const LEti::Object_2D& _for, const glm::vec3& _particle, const glm::vec3& _impulse)->float
				{
					glm::vec3 particle_to_center_vec = _for.physics_module()->get_physical_model()->center_of_mass() - _particle;
					float angle = acos(LEti::Math::angle_cos_between_vectors(_impulse, particle_to_center_vec));
					glm::vec3 axis = LEti::Math::normalize(_impulse, particle_to_center_vec);
					LEti::Math::shrink_vector_to_1(axis);
					angle *= axis.z;
					return angle;
				};

				glm::vec3 f_impulse_vector = f.get_impulse_of_attached_point(_particle);
				glm::vec3 s_impulse_vector = s.get_impulse_of_attached_point(_particle);

				glm::vec3 vec_f_particle_to_center = _particle - f.physics_module()->get_physical_model()->center_of_mass();
				glm::vec3 vec_s_particle_to_center = _particle - s.physics_module()->get_physical_model()->center_of_mass();

				float f_normal_angle_cos = LEti::Math::angle_cos_between_vectors({1.0f, 0.0f, 0.0f}, _f_normal);
				float f_normal_angle_sin = sqrt(1 - (f_normal_angle_cos * f_normal_angle_cos));
				s_impulse_vector.x *= fabs(f_normal_angle_cos);
				s_impulse_vector.y *= fabs(f_normal_angle_sin);

				float s_normal_ratio = 0.0f;
				if(!LEti::Math::floats_are_equal(LEti::Math::vector_length(f_impulse_vector), 0.0f))
					s_normal_ratio = fabs(cos(get_new_rotation_angle(f, _particle, f_impulse_vector)));

				glm::vec3 s_normal_scaled = _s_normal;
				LEti::Math::extend_vector_to_length(s_normal_scaled, LEti::Math::vector_length(f_impulse_vector));

				s_normal_scaled *= s_normal_ratio;

				glm::vec3 result_vector = f_impulse_vector + s_impulse_vector + s_normal_scaled;

				glm::vec3 force_to_particle_vector = s_impulse_vector + s_normal_scaled - (f_impulse_vector);

				float angle_cos_ctp_and_impulse = LEti::Math::angle_cos_between_vectors(vec_f_particle_to_center, force_to_particle_vector);		//ctp - center to particle
				float angle_ctp_and_impulse = acos(angle_cos_ctp_and_impulse);
				float force = LEti::Math::vector_length(force_to_particle_vector); // * mass
				float torque = LEti::Math::vector_length(vec_f_particle_to_center) * force;
				float angular_velocity = torque * (DT * (1 - it->time_of_intersection_ratio));
				glm::vec3 axis = LEti::Math::normalize(vec_f_particle_to_center, force_to_particle_vector);
//				LEti::Math::shrink_vector_to_1(axis);
				float f_new_rotation_angle = angular_velocity * sin(angle_ctp_and_impulse) * (axis.z * (DT * (1 - it->time_of_intersection_ratio)));

				result_vector *= fabs(cos(f_new_rotation_angle));

				return {result_vector , f_new_rotation_angle };
			};

			auto new_rd_v2 = [](const Moving_Object& _impulse_from, const glm::vec3& _intersection_point, const glm::vec3& _normal)->std::pair<glm::vec3, float>
			{
				glm::vec3 impulse = _impulse_from.get_impulse_of_attached_point(_intersection_point);

//				glm::vec3 reflected_normal = _normal/* * -1.0f*/;
//				glm::vec3 result_vector = LEti::Math::vector_length(impulse) * reflected_normal;

				glm::vec3 result_vector;
				for(unsigned int i=0; i<3; ++i)
					result_vector[i] = impulse[i] * fabs(_normal[i]);

//				 result_vector = LEti::Math::vector_length(impulse) * reflected_normal;

				return {result_vector , 0.0f };
			};

			auto project_onto_normal = [](const glm::vec3& _vec, const glm::vec3& _normal)->glm::vec3
			{
				glm::vec3 result = _normal * LEti::Math::vector_length(_vec);
				return result;
			};

			//	find new rotaion speed

			// f_particle_impulse.x
			// f_particle_impulse.y
			// s_particle_impulse.x
			// s_particle_impulse.y
			// f_compensating_impulse.x
			// f_compensating_impulse.y
			// s_compensating_impulse.x
			// s_compensating_impulse.y


			glm::vec3 f_particle_impulse = f.get_impulse_of_attached_point(it->point);
			glm::vec3 s_particle_impulse = s.get_impulse_of_attached_point(it->point);

			glm::vec3 f_compensating_impulse = project_onto_normal(s_particle_impulse, -it->first_normal) + project_onto_normal(f_particle_impulse, it->second_normal);
			glm::vec3 s_compensating_impulse = project_onto_normal(f_particle_impulse, -it->second_normal) + project_onto_normal(s_particle_impulse, it->first_normal);

			f.apply_impulse(f_compensating_impulse, it->point);
			s.apply_impulse(s_compensating_impulse, it->point);

			//

//			float f_imp_str_before = get_particle_velocity(f, (*f.physics_module()->get_physical_model())[0][0]) / it->time_of_intersection_ratio;
//			float s_imp_str_before = get_particle_velocity(s, (*s.physics_module()->get_physical_model())[0][0]) / it->time_of_intersection_ratio;
//			float imp_sum_before = f_imp_str_before + s_imp_str_before;
//			std::cout << "before: " << imp_sum_before << "\n";

//			f.update(1.0f - it->time_of_intersection_ratio);
//			s.update(1.0f - it->time_of_intersection_ratio);

//			float f_imp_str_after = get_particle_velocity(f, (*f.physics_module()->get_physical_model())[0][0])/* / (it->time_of_intersection_ratio)*/;
//			float s_imp_str_after = get_particle_velocity(s, (*s.physics_module()->get_physical_model())[0][0])/* / (it->time_of_intersection_ratio)*/;
//			float imp_sum_after = f_imp_str_after + s_imp_str_after;
//			std::cout << "after:  " << imp_sum_after << "\n\n";


			std::cout << "-----  after collision movement info  -----\n";
			print_info_func();

			++it;
		}

		grab.update();

		if(LEti::Event_Controller::mouse_button_was_pressed(GLFW_MOUSE_BUTTON_1))
		{
			LEti::Space_Splitter_2D::unregister_point(&cursor_position);
			auto plist = LEti::Space_Splitter_2D::get_collisions__points();

			if(plist.size() != 0)
				 grab.grab(objects_map.at(plist.begin()->first), Grab::Type::drag);
		}if(LEti::Event_Controller::mouse_button_was_pressed(GLFW_MOUSE_BUTTON_2))
		{
			LEti::Space_Splitter_2D::unregister_point(&cursor_position);
			auto plist = LEti::Space_Splitter_2D::get_collisions__points();

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

		ind.draw();
		flat_co.draw();
		flat_co_2.draw();
		flat_co_3.draw();
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

		float impulse_summary = 0.0f;
		for(auto& fco : objects_map)
			impulse_summary += get_particle_velocity(*fco.second, (*fco.second->physics_module()->get_physical_model())[0][0] /*{25.0f, 0.0f, 0.0f}*/ ) / DT;
		if(impulse_summary < 0.01f) impulse_summary = 0.0f;
		std::stringstream string_summary_impulse_value;
		string_summary_impulse_value << std::fixed << std::setprecision(0) << impulse_summary;
		impulses_summary_infoblock.set_text(string_summary_impulse_value.str().c_str());
		impulses_summary_infoblock.draw();

		std::this_thread::sleep_for(std::chrono::milliseconds(delay));

		LEti::Window_Controller::swap_buffers();
	}

	return 0;
}














































