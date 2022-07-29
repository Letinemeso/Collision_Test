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

#include "Debug_Drawable_Frame.h"



#define DT LEti::Event_Controller::get_dt()

struct On_Button_Pressed_Msg
{
	DEFINE_TYPE("obpf");
	unsigned int btn = 0;
	On_Button_Pressed_Msg(unsigned int _btn) : btn(_btn) { }
};

int main()
{
	LEti::Event_Controller::init_and_create_window(1200, 800, "Collision Test");

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

	LEti::Resource_Loader::init();

	LEti::Resource_Loader::load_object("textures", "Resources/Textures/textures.mdl");

	

	///////////////// 2d collision test

//	LEti::Resource_Loader::load_object("flat_co_1", "Resources/Models/flat_cos/flat_co_1.mdl");
	LEti::Resource_Loader::load_object("flat_co_1", "Resources/Models/quad.mdl");
	LEti::Object_2D flat_co;

//	LEti::Resource_Loader::load_object("flat_co_2", "Resources/Models/flat_cos/flat_co_2.mdl");
	LEti::Resource_Loader::load_object("flat_co_2", "Resources/Models/quad.mdl");
	LEti::Object_2D flat_co_2;
//	flat_co_2.move(300, 300, 0);

	LEti::Resource_Loader::load_object("flat_co_3", "Resources/Models/quad.mdl");
	LEti::Object_2D flat_co_3;

	flat_co.init("flat_co_1");
	flat_co.set_pos(600, 400, 0);
	flat_co.set_texture("white_texture");

	LEti::Object_2D flat_co_foreshadow;
	flat_co_foreshadow.init("flat_co_1");
	flat_co_foreshadow.set_pos(50, 400, 0);

	glm::vec3 velocity;
	float angle =  0.0f /*LEti::Math::QUARTER_PI*/ /* * 1.75f*/;
	float speed = 200.0f;
//	float speed = 9000.0f;
	velocity.x = cos(angle);
	velocity.y = sin(angle);

	flat_co_2.init("flat_co_2");
//	flat_co_2.set_pos(50, 400, 0);
//	flat_co_2.set_overall_scale(100.0f);
//	flat_co_2.set_scale(20.0f, 20.0f, 1.0f);

	flat_co_2.set_scale(5.0f, 70.0f, 1.0f);
	flat_co_2.set_pos(400, 400, 0);

	flat_co_3.init("flat_co_3");
//	flat_co_3.set_collision_possibility(false);
//	flat_co_3.set_visible(false);
//	flat_co_3.move(1000, 300, 0);
//	flat_co_3.set_pos(1150, 400, 0);
//	flat_co_3.set_scale(50.0f, 50.0f, 1.0f);

	flat_co_3.set_pos(50, 400, 0);
	flat_co_3.set_scale(5.0f, 100.0f, 1.0f);


	LEti::Resource_Loader::load_object("flat_indicator_red", "Resources/Models/flat_indicator_red.mdl");
	LEti::Resource_Loader::load_object("debug_frame", "Resources/Models/debug_frame.mdl");
//	LEti::init_frame("debug_frame");
//	LEti::Debug_Drawable_Frame frame;
//	frame.init("debug_frame");
//	frame.get_vertices()[0] = -50.0f;

//	frame.set_point(0, {0.0f, 500.0f, 0.0f}).set_point(1, {500.0f, 0.0f, 0.0f}).set_point(2, {0.0f, 0.0f, 0.0f}).set_point(3, {500, 500, 0}).set_point(3, {700, 250, 0});
//	frame.set_sequence_element(0, 0).set_sequence_element(1, 1).set_sequence_element(2, 2).set_sequence_element(3, 3).set_sequence_element(4, 4);
//	frame.update();


	LEti::Resource_Loader::load_object("ind", "Resources/Models/intersection_point_indicator.mdl");
	LEti::Object_2D ind;
	ind.init("ind");

	LEti::Resource_Loader::load_object("text_field", "Resources/Models/text_field.mdl");
	LEti::Text_Field intersection_info_block;
	intersection_info_block.init("text_field");
	LEti::Text_Field tf_flat_co_speed;
	tf_flat_co_speed.init("text_field");
	tf_flat_co_speed.set_pos(0, 760, 0);

	LEti::Text_Field fps_info_block;
	fps_info_block.init("text_field");
	fps_info_block.set_pos(1150, 770, 0);

	flat_co.update();
	flat_co_2.update();
	flat_co_3.update();

	flat_co.set_dynamic(true);
	flat_co_2.set_dynamic(true);
	flat_co_3.set_dynamic(true);
	LEti::Space_Splitter_2D::register_object(&flat_co);
	LEti::Space_Splitter_2D::register_object(&flat_co_2);
	LEti::Space_Splitter_2D::register_object(&flat_co_3);

	float triangle_speed = 100.0f;

//	bool intersection_detected = false;
//	if (intersection_detected)
//		intersection_info_block.set_text("Intersection detected");
//	else
//		intersection_info_block.set_text("Intersection not detected");

//	bool moving_down_kostyl = false;

	bool flat_co_enabled = true;

	LEti::Timer fps_timer;
	unsigned int fps_counter = 0;

	LEti::Debug_Drawable_Frame frame;
	frame.init("debug_frame");

	while (!LEti::Event_Controller::window_should_close())
	{
		LEti::Event_Controller::update();

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		if (LEti::Event_Controller::key_was_pressed(GLFW_KEY_TAB))
			LEti::Camera::toggle_controll(LEti::Camera::get_controllable() ? false : true);
		LEti::Camera::update(false, true);

		flat_co.update_previous_state();
		flat_co_2.update_previous_state();
		flat_co_3.update_previous_state();

		if (LEti::Event_Controller::is_key_down(GLFW_KEY_LEFT))
		{
			flat_co_2.move(-(triangle_speed * DT), 0.0f, 0.0f);
		}
		if (LEti::Event_Controller::is_key_down(GLFW_KEY_RIGHT))
		{
			flat_co_2.move((triangle_speed * DT), 0.0f, 0.0f);
		}
		if (LEti::Event_Controller::is_key_down(GLFW_KEY_DOWN))
		{
			flat_co_2.move(0.0f, -(triangle_speed * DT), 0.0f);
		}
		if (LEti::Event_Controller::is_key_down(GLFW_KEY_UP))
		{
			flat_co_2.move(0.0f, (triangle_speed * DT), 0.0f);
		}

		if (LEti::Event_Controller::is_key_down(GLFW_KEY_A))
		{
			flat_co_3.move(-(triangle_speed * DT), 0.0f, 0.0f);
		}
		if (LEti::Event_Controller::is_key_down(GLFW_KEY_D))
		{
			flat_co_3.move((triangle_speed * DT), 0.0f, 0.0f);
		}
		if (LEti::Event_Controller::is_key_down(GLFW_KEY_S))
		{
			flat_co_3.move(0.0f, -(triangle_speed * DT), 0.0f);
		}
		if (LEti::Event_Controller::is_key_down(GLFW_KEY_W))
		{
			flat_co_3.move(0.0f, (triangle_speed * DT), 0.0f);
		}

		if (LEti::Event_Controller::key_was_pressed(GLFW_KEY_I))
		{
			speed += 50.0f;
		}
		if (LEti::Event_Controller::key_was_pressed(GLFW_KEY_K))
		{
			speed -= 50.0f;
			if(speed < 0.0f)
				speed = 0.0f;
		}
		if (LEti::Event_Controller::is_key_down(GLFW_KEY_J))
		{
			angle += LEti::Math::HALF_PI * LEti::Event_Controller::get_dt();
		}
		if (LEti::Event_Controller::is_key_down(GLFW_KEY_L))
		{
			angle -= LEti::Math::HALF_PI * LEti::Event_Controller::get_dt();
		}

		if(LEti::Event_Controller::key_was_pressed(GLFW_KEY_C))
		{
			flat_co.set_pos(0, 0, 0);
			flat_co_2.set_pos(0, 0, 0);
			flat_co_3.set_pos(0, 0, 0);
		}

		if(flat_co.get_pos().y >= 800.0f)
		{
			flat_co.set_pos(flat_co.get_pos().x, 799.0f, 0.0f);
			angle = LEti::Math::DOUBLE_PI - angle;
		}
		else if(flat_co.get_pos().y <= 0.0f)
		{
			flat_co.set_pos(flat_co.get_pos().x, 1.0f, 0.0f);
			angle = LEti::Math::DOUBLE_PI - angle;
		}

		if(flat_co.get_pos().x >= 1200.0f)
		{
			flat_co.set_pos(1199.0f, flat_co.get_pos().y, 0.0f);
			angle = LEti::Math::DOUBLE_PI - angle + LEti::Math::PI;
		}
		else if(flat_co.get_pos().x <= 0.0f)
		{
			flat_co.set_pos(1.0f, flat_co.get_pos().y, 0.0f);
			angle = LEti::Math::DOUBLE_PI - angle + LEti::Math::PI;
		}

		if(angle < 0.0f)
			angle += LEti::Math::DOUBLE_PI;
		else if(angle > LEti::Math::DOUBLE_PI)
			angle -= LEti::Math::DOUBLE_PI;

		velocity.x = speed * cos(angle) * LEti::Event_Controller::get_dt();;
		velocity.y = speed * sin(angle) * LEti::Event_Controller::get_dt();;
		flat_co.move(velocity.x, velocity.y, 0.0f);

//		flat_co_3.move(0.0f, moving_down_kostyl ? -1.0f : 1.0f, 0.0f);
//		moving_down_kostyl = !moving_down_kostyl;

		flat_co.update();
		flat_co_2.update();
		flat_co_3.update();

		LEti::Space_Splitter_2D::update();

//		frame.draw();

		std::list<LEti::Space_Splitter_2D::Collision_Data> list = LEti::Space_Splitter_2D::get_collisions();

		auto it = list.begin();
		while(it != list.end())
		{
			ind.set_pos(it->collision_data.point.x, it->collision_data.point.y, 0.0f);
//			ind.draw();

			glm::vec3 diff = it->first->get_pos() - it->second->get_pos();
//			if(fabs(diff.x) > fabs(diff.y))
				angle = LEti::Math::DOUBLE_PI - angle + LEti::Math::PI;
//			else
//				angle = LEti::Math::DOUBLE_PI - angle;

			velocity.x = speed * cos(angle) * LEti::Event_Controller::get_dt() * (1.0f - it->collision_data.time_of_intersection_ratio);
			velocity.y = speed * sin(angle) * LEti::Event_Controller::get_dt() * (1.0f - it->collision_data.time_of_intersection_ratio);

			flat_co.move(velocity.x, velocity.y, 0.0f);
			flat_co.update();

//			glm::vec3 controlled_co_diff = (flat_co_3.get_pos() - flat_co_3.get_pos_prev()) * -1.0f * (1.0f - it->collision_data.time_of_intersection_ratio) * 50.0f;
//			flat_co_3.move(controlled_co_diff.x, controlled_co_diff.y, controlled_co_diff.z);

			++it;
		}
//		if(list.size() == 0)
//			ind.set_pos(0, 0, 0);
//		std::cout << "\n";

		if(angle < 0.0f)
			angle += LEti::Math::DOUBLE_PI;
		else if(angle > LEti::Math::DOUBLE_PI)
			angle -= LEti::Math::DOUBLE_PI;

		flat_co_2.draw();
		if(flat_co_enabled)
			flat_co.draw();
//		flat_co_foreshadow.set_pos(flat_co.get_pos().x + velocity.x, flat_co.get_pos().y + velocity.y, 0.0f);
//		flat_co_foreshadow.draw();
		flat_co_3.draw();

		ind.draw();

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
//			fps_info_block.set_text((std::to_string((int)(1.0f / LEti::Event_Controller::get_dt()))).c_str());
			fps_info_block.set_text((std::to_string(fps_counter)).c_str());
			fps_counter = 0;
		}
		fps_info_block.draw();

		tf_flat_co_speed.set_text((std::to_string(speed)).c_str());
		tf_flat_co_speed.draw();

		LEti::Event_Controller::swap_buffers();
	}

	return 0;
}














































