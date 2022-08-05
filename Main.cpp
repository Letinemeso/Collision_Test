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
	float m_mass = 1.0f;

public:
	void update() override
	{
		glm::vec3 trajectory{0.0f, 0.0f, 0.0f};
		trajectory.x = m_speed * cos(m_angle) * LEti::Event_Controller::get_dt();
		trajectory.y = m_speed * sin(m_angle) * LEti::Event_Controller::get_dt();
		move(trajectory.x, trajectory.y, 0.0f);
		LEti::Object_2D::update();
	}

	void update_with_additional_ratio(float _ratio)
	{
		glm::vec3 trajectory{0.0f, 0.0f, 0.0f};
		trajectory.x = m_speed * cos(m_angle) * LEti::Event_Controller::get_dt() * (1.0f - _ratio);
		trajectory.y = m_speed * sin(m_angle) * LEti::Event_Controller::get_dt() * (1.0f - _ratio);
		move(trajectory.x, trajectory.y, 0.0f);
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

	LEti::Space_Splitter_2D::set_precision(10);

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

	auto reset_func = [&flat_co, &flat_co_2, &flat_co_3]()
	{
//		flat_co.set_pos(800, 400, 0);
//		flat_co.m_angle = 0.1f;
//		flat_co.m_speed = 200.0f;
		flat_co.set_pos(400, 400, 0);
		flat_co.m_angle = LEti::Math::HALF_PI + LEti::Math::PI/* 0.0f*/;
		flat_co.m_speed = 200.0f;

		flat_co_2.set_pos(1000, 600, 0);
		flat_co_2.m_angle = 2.34f;
		flat_co_2.m_speed = 200.0f;

		flat_co_3.set_pos(800, 400, 0);
		flat_co_3.m_angle = LEti::Math::PI + 0.44f;
		flat_co_3.m_speed = 200.0f;
	};
	reset_func();

	std::map<const LEti::Object_2D*, Moving_Object*> objects_map;
	objects_map.emplace(&flat_co, &flat_co);
	objects_map.emplace(&flat_co_2, &flat_co_2);
	objects_map.emplace(&flat_co_3, &flat_co_3);

	LEti::Resource_Loader::load_object("flat_indicator_red", "Resources/Models/flat_indicator_red.mdl");
	LEti::Resource_Loader::load_object("debug_frame", "Resources/Models/debug_frame.mdl");

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

	flat_co.set_dynamic(true);
	flat_co_2.set_dynamic(true);
	flat_co_3.set_dynamic(true);
	LEti::Space_Splitter_2D::register_object(&flat_co);
	LEti::Space_Splitter_2D::register_object(&flat_co_2);
	LEti::Space_Splitter_2D::register_object(&flat_co_3);

	bool flat_co_enabled = true;

	unsigned int fps_counter = 0;

	LEti::Debug_Drawable_Frame frame;
	frame.init("debug_frame");

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
			for(auto& co : objects_map)
				co.second->rotate(LEti::Math::HALF_PI * DT);
		}
		if(LEti::Event_Controller::is_key_down(GLFW_KEY_RIGHT))
		{
			for(auto& co : objects_map)
				co.second->rotate(-(LEti::Math::HALF_PI * DT));
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

		LEti::Space_Splitter_2D::update();

		std::list<LEti::Space_Splitter_2D::Collision_Data> list = LEti::Space_Splitter_2D::get_collisions();

		if(list.size() == 0)
			intersection_on_prev_frame = false;

		auto it = list.begin();
		while(it != list.end())
		{
			if(intersection_on_prev_frame)
				std::cout << "error!\n";
			std::cout << it->collision_data.time_of_intersection_ratio << "\n";

			ind.set_pos(it->collision_data.point.x, it->collision_data.point.y, 0.0f);
			ind.draw();

			Moving_Object& f = *(objects_map.at(it->first));
			Moving_Object& s = *(objects_map.at(it->second));

			std::cout << "before:\n\tfirst angle: " << f.m_angle << "\n\tsecond angle: " << s.m_angle << "\n";

			glm::vec3 diff = f.get_pos() - s.get_pos();
			if(fabs(diff.x) > fabs(diff.y))
			{
				f.m_angle = LEti::Math::DOUBLE_PI - f.m_angle + LEti::Math::PI;
				s.m_angle = LEti::Math::DOUBLE_PI - s.m_angle + LEti::Math::PI;
			}
			else
			{
				f.m_angle = LEti::Math::DOUBLE_PI - f.m_angle;
				s.m_angle = LEti::Math::DOUBLE_PI - s.m_angle;
			}

//			f.update_with_additional_ratio(0.0f);
//			s.update_with_additional_ratio(0.0f);
			f.update_with_additional_ratio(it->collision_data.time_of_intersection_ratio);
			s.update_with_additional_ratio(it->collision_data.time_of_intersection_ratio);

			std::cout << "after:\n\tfirst angle: " << f.m_angle << "\n\tsecond angle: " << s.m_angle << "\n\n";

			auto idk = f.get_physical_model()->is_intersecting_with_another_model(*s.get_physical_model());
			if(idk)
				std::cout << "models still intersect!\n\n";

			++it;
		}
		if(list.size() != 0)
			intersection_on_prev_frame = true;

		if(LEti::Event_Controller::mouse_button_was_pressed(GLFW_MOUSE_BUTTON_1))
		{
			glm::vec3 cpos;
			cpos.z = 0.0f;
			cpos.x = LEti::Window_Controller::get_cursor_position().x;
			cpos.y = LEti::Window_Controller::get_cursor_position().y;
			auto plist = LEti::Space_Splitter_2D::get_objects_encircling_point(cpos);
			if(plist.size() == 0)
			{
				std::cout << "list is empty\n\n";
			}
			else
			{
				for(const auto& obj : plist)
					std::cout << obj << "\n";
				std::cout << "\n";
			}
		}

		if(flat_co_enabled)
			flat_co.draw();
		flat_co_2.draw();
		flat_co_3.draw();

//		ind.draw();

		frame.clear_points().clear_sequence();
		const auto pm = ((const LEti::Object_2D&)flat_co).get_physical_model();
		unsigned int counter = 0;
		for(unsigned int i=0; i<pm->get_polygons_count(); ++i)
		{
			for(unsigned int j=0; j<3; ++j)
			{
				frame.set_point(counter, (*pm)[i][j]).set_sequence_element(counter, counter);
				++counter;
			}
		}
		frame.update();
		frame.draw();
		frame.clear_points().clear_sequence();
		const auto pm_2 = ((const LEti::Object_2D&)flat_co_2).get_physical_model();
		counter = 0;
		for(unsigned int i=0; i<pm_2->get_polygons_count(); ++i)
		{
			for(unsigned int j=0; j<3; ++j)
			{
				frame.set_point(counter, (*pm_2)[i][j]).set_sequence_element(counter, counter);
				++counter;
			}
		}
		frame.update();
		frame.draw();
		frame.clear_points().clear_sequence();
		const auto pm_3 = ((const LEti::Object_2D&)flat_co_3).get_physical_model();
		counter = 0;
		for(unsigned int i=0; i<pm_3->get_polygons_count(); ++i)
		{
			for(unsigned int j=0; j<3; ++j)
			{
				frame.set_point(counter, (*pm_3)[i][j]).set_sequence_element(counter, counter);
				++counter;
			}
		}
		frame.update();
		frame.draw();

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














































