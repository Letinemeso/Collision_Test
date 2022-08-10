#include "Event_Controller.h"
#include "Shader.h"
#include "Camera.h"
#include "Resource_Loader.h"

#include "Object.h"
#include "Text_Field.h"

#include "Physical_Model_3D.h"
#include "Physical_Model_2D.h"

#include "Message_Translator.h"

#include "Space_Splitter_2D.h"
#include "Physics/Space_Hasher_2D.h"
#include "Physics/Default_Narrow_CD.h"

#include "Timer.h"

#include "Debug_Drawable_Frame.h"



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
	float m_speed = 0.0f;
	float m_angle = 0.0f;
	float m_rotation_delta = /*LEti::Math::HALF_PI*/ LEti::Math::QUARTER_PI /*0.0f*/;
//	float m_rotation_delta = LEti::Math::HALF_PI  /*0.0f*/;
	float m_mass = 1.0f;

public:
	void update() override
	{
		glm::vec3 trajectory{0.0f, 0.0f, 0.0f};
		trajectory.x = m_speed * cos(m_angle) * DT;
		trajectory.y = m_speed * sin(m_angle) * DT;
		move(trajectory.x, trajectory.y, 0.0f);

		rotate(m_rotation_delta * DT);

		LEti::Object_2D::update();
	}

	void update_with_additional_ratio(float _ratio)
	{
		glm::vec3 trajectory{0.0f, 0.0f, 0.0f};
		trajectory.x = m_speed * cos(m_angle) * LEti::Event_Controller::get_dt() * (1.0f - _ratio);
		trajectory.y = m_speed * sin(m_angle) * LEti::Event_Controller::get_dt() * (1.0f - _ratio);
		move(trajectory.x, trajectory.y, 0.0f);

		rotate(m_rotation_delta * DT * (1.0f - _ratio));

		LEti::Object_2D::update();
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
//	LEti::Space_Splitter_2D::set_precision(10);
	LEti::Space_Splitter_2D::get_broad_phase()->set_precision(10);
	LEti::Space_Splitter_2D::get_narrow_phase()->set_precision(100);

	LEti::Resource_Loader::init();

	LEti::Resource_Loader::load_object("textures", "Resources/Textures/textures.mdl");

	LEti::Event_Controller::set_max_dt(60.0f / 1000.0f);

	

	///////////////// 2d collision test

	LEti::Resource_Loader::load_object("flat_co_1", "Resources/Models/quad.mdl");
	Moving_Object flat_co;
	flat_co.m_speed = 200.0f;
	flat_co.m_angle = LEti::Math::PI;

	LEti::Resource_Loader::load_object("flat_co_2", "Resources/Models/quad.mdl");
	Moving_Object flat_co_2;

	LEti::Resource_Loader::load_object("flat_co_3", "Resources/Models/quad.mdl");
	Moving_Object flat_co_3;

	flat_co.init("flat_co_1");
	flat_co.set_pos(800, 400, 0);
	flat_co.set_texture("white_texture");

	LEti::Object_2D flat_co_foreshadow;
	flat_co_foreshadow.init("flat_co_1");
	flat_co_foreshadow.set_pos(50, 400, 0);

	flat_co_2.init("flat_co_2");
	flat_co_2.set_scale(20.0f, 20.0f, 1.0f);
	flat_co_2.set_pos(400, 600, 0);
	flat_co_2.m_speed = 0.0f;
	flat_co_2.m_angle = 2.53f;

	flat_co_3.init("flat_co_3");
	flat_co_3.set_scale(50.0f, 50.0f, 1.0f);
	flat_co_3.set_pos(400, 400, 0);
	flat_co_3.m_speed = 200.0f;
	flat_co_3.m_angle = 0.0f;

	float co_spd= 150.0f;
	auto reset_func = [&flat_co, &flat_co_2, &flat_co_3]()
	{
//		flat_co.set_pos(400, 400, 0);
//		flat_co.m_angle = LEti::Math::HALF_PI + LEti::Math::PI/* 0.0f*/;
//		flat_co.m_speed = 200.0f;

//		flat_co_2.set_pos(1000, 600, 0);
//		flat_co_2.m_angle = 2.34f;
//		flat_co_2.m_speed = 200.0f;

//		flat_co_3.set_pos(800, 400, 0);
//		flat_co_3.m_angle = LEti::Math::PI + 0.44f;
//		flat_co_3.m_speed = 200.0f;

		flat_co.set_pos(200, 400, 0);
		flat_co.m_angle = /*LEti::Math::HALF_PI + LEti::Math::PI*/ 0.0f;
		flat_co.m_speed = 0.0f;

		flat_co.set_overall_scale(300.0f);

		flat_co_2.set_pos(800, 700, 0);
		flat_co_2.m_angle = /*LEti::Math::PI*/ 0 /*2.34f*/;
		flat_co_2.m_speed = 0.0f;

		flat_co_3.set_pos(1050, 400, 0);
		flat_co_3.m_angle = LEti::Math::PI /*+ 0.44f*/;
		flat_co_3.m_speed = 0.0f;

		flat_co_3.set_overall_scale(200.0f);
	};
	reset_func();

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
	LEti::Text_Field intersection_info_block;
	intersection_info_block.init("text_field");
	LEti::Text_Field tf_flat_co_speed;
	tf_flat_co_speed.init("text_field");
	tf_flat_co_speed.set_pos(0, 760, 0);

	LEti::Text_Field fps_info_block;
	fps_info_block.init("text_field");
//	fps_info_block.set_pos(1150, 770, 0);
	fps_info_block.set_pos(10, 770, 0);

	flat_co.update();
	flat_co_2.update();
	flat_co_3.update();

	glm::vec3 cursor_position;

	flat_co.set_dynamic(true);
	flat_co_2.set_dynamic(true);
	flat_co_3.set_dynamic(true);
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
			flat_co.m_speed += 100.0f;
			flat_co_2.m_speed += 100.0f;
			flat_co_3.m_speed += 100.0f;
		}
		if (LEti::Event_Controller::key_was_pressed(GLFW_KEY_K))
		{
			flat_co.m_speed -= 100.0f;
			if(flat_co.m_speed < 0.0f)
				flat_co.m_speed = 0.0f;
			flat_co_2.m_speed -= 100.0f;
			if(flat_co_2.m_speed < 0.0f)
				flat_co_2.m_speed = 0.0f;
			flat_co_3.m_speed -= 100.0f;
			if(flat_co_3.m_speed < 0.0f)
				flat_co_3.m_speed = 0.0f;
		}
		if (LEti::Event_Controller::is_key_down(GLFW_KEY_J))
		{
			flat_co.m_angle += LEti::Math::HALF_PI * LEti::Event_Controller::get_dt();
		}
		if (LEti::Event_Controller::is_key_down(GLFW_KEY_L))
		{
			flat_co.m_angle -= LEti::Math::HALF_PI * LEti::Event_Controller::get_dt();
		}

		if(LEti::Event_Controller::key_was_pressed(GLFW_KEY_C))
		{
			flat_co.set_pos(0, 0, 0);
			flat_co_2.set_pos(0, 0, 0);
			flat_co_3.set_pos(0, 0, 0);
		}

		if(LEti::Event_Controller::is_key_down(GLFW_KEY_LEFT))
		{
			flat_co.move(-(co_spd * DT), 0.0f, 0.0f);
		}
		if(LEti::Event_Controller::is_key_down(GLFW_KEY_RIGHT))
		{
			flat_co.move( (co_spd * DT), 0.0f, 0.0f);
		}
		if(LEti::Event_Controller::is_key_down(GLFW_KEY_UP))
		{
			flat_co.move(0.0f,  (co_spd * DT), 0.0f);
		}
		if(LEti::Event_Controller::is_key_down(GLFW_KEY_DOWN))
		{
			flat_co.move(0.0f, -(co_spd * DT), 0.0f);
		}

		if(LEti::Event_Controller::key_was_pressed(GLFW_KEY_R))
		{
			reset_func();
		}

		for(auto& co : objects_map)
		{
			Moving_Object& cco = *co.second;
			if(cco.get_pos().y >= 800.0f)
			{
				cco.set_pos(cco.get_pos().x, 799.0f, 0.0f);
				cco.m_angle = LEti::Math::DOUBLE_PI - cco.m_angle;
			}
			else if(cco.get_pos().y <= 0.0f)
			{
				cco.set_pos(cco.get_pos().x, 1.0f, 0.0f);
				cco.m_angle = LEti::Math::DOUBLE_PI - cco.m_angle;
			}

			if(cco.get_pos().x >= 1200.0f)
			{
				cco.set_pos(1199.0f, cco.get_pos().y, 0.0f);
				cco.m_angle = LEti::Math::DOUBLE_PI - cco.m_angle + LEti::Math::PI;
			}
			else if(cco.get_pos().x <= 0.0f)
			{
				cco.set_pos(1.0f, cco.get_pos().y, 0.0f);
				cco.m_angle = LEti::Math::DOUBLE_PI - cco.m_angle + LEti::Math::PI;
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
//			auto plist = LEti::Space_Splitter_2D::get_objects_encircling_point(cpos);
			LEti::Space_Splitter_2D::register_point(&cursor_position);

//			if(plist.size() == 0)
//			{
//				std::cout << "list is empty\n\n";
//			}
//			else
//			{
//				for(const auto& obj : plist)
//					std::cout << obj << "\n";
//				std::cout << "\n";
//			}
		}

		auto draw_frames_relative_to_other = [&frame, &frame_red](const LEti::Object_2D& _obj, const LEti::Object_2D& _other)->void
		{
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

//				frame.clear_points().clear_sequence();

			LEti::Physical_Model_2D::Imprint pm = *_obj.get_physical_model_prev_state();

			draw_frame(frame_red, pm);

			glm::mat4x4 diff_pos = _obj.get_translation_matrix_for_time_ratio(1.0f) * _other.get_translation_matrix_diff_inversed_for_time_ratio(1.0f);
			glm::mat4x4 diff_rotation = _obj.get_rotation_matrix_for_time_ratio(1.0f) * _other.get_rotation_matrix_diff_inversed_for_time_ratio(1.0f);
			glm::mat4x4 diff_scale = _obj.get_scale_matrix_for_time_ratio(1.0f) * _other.get_scale_matrix_diff_inversed_for_time_ratio(1.0f);
			pm.update(diff_pos, diff_rotation, diff_scale);

			draw_frame(frame, pm);

			draw_frame(frame, *_other.get_physical_model_prev_state());

		};
		draw_frames_relative_to_other(flat_co, flat_co_3);
//		LEti::Window_Controller::swap_buffers();


		LEti::Space_Splitter_2D::update();

		const LEti::Default_Narrow_CD::Collision_Data_List__Models& list = LEti::Space_Splitter_2D::get_collisions__models();

		if(intersection_on_prev_frame)
			std::cout << "";

		if(list.size() == 0)
			intersection_on_prev_frame = false;

		auto it = list.begin();
		while(it != list.end())
		{
			ind.set_pos(it->collision_data.point.x, it->collision_data.point.y, 0.0f);
			ind.draw();

			Moving_Object& f = *(objects_map.at(it->first));
			Moving_Object& s = *(objects_map.at(it->second));

			f.m_angle += LEti::Math::PI;
			s.m_angle += LEti::Math::PI;

			f.m_rotation_delta *= -1.0f;
			s.m_rotation_delta *= -1.0f;

//			f.update_with_additional_ratio(0.0f);
//			s.update_with_additional_ratio(0.0f);
			f.update_with_additional_ratio(it->collision_data.time_of_intersection_ratio);
			s.update_with_additional_ratio(it->collision_data.time_of_intersection_ratio);

			auto idk = f.get_physical_model()->is_intersecting_with_another_model(*s.get_physical_model());
//			if(idk)
//			{
//				LEti::Window_Controller::swap_buffers();
//				std::cout << "models still intersect!\n\n";
//			}

			++it;
		}
		if(list.size() != 0)
			intersection_on_prev_frame = true;

		if(LEti::Event_Controller::mouse_button_was_pressed(GLFW_MOUSE_BUTTON_1))
		{
			LEti::Space_Splitter_2D::unregister_point(&cursor_position);
			auto plist = LEti::Space_Splitter_2D::get_collisions__points();

			if(plist.size() == 0)
			{
				std::cout << "list is empty\n\n";
			}
			else
			{
				for(const auto& obj : plist)
					std::cout << obj.object << "\n";
				std::cout << "\n";
			}
		}

		if(!intersection_on_prev_frame)
		{
//			flat_co.draw();
//			flat_co_2.draw();
//			flat_co_3.draw();
		}

		intersection_info_block.set_text(std::to_string(list.size()).c_str());
		intersection_info_block.draw();

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














































