/*#include "Event_Controller.h"
#include "Shader.h"
#include "Camera.h"
#include "Resource_Loader.h"

#include "Object.h"
#include "Text_Field.h"

#include "Physical_Model_3D.h"
#include "Physical_Model_2D.h"

#include "Message_Translator.h"

//#include "Space_Splitter_2D.h"
#include "Space_Splitter_3D.h"

//#include "Timer.h"

struct On_Button_Pressed_Msg
{
	DEFINE_TYPE("obpf");
	unsigned int btn = 0;
	On_Button_Pressed_Msg(unsigned int _btn) : btn(_btn) { }
};

int main()
{
	LEti::Event_Controller::init_and_create_window(1200, 800, "Board Game");

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

	LEti::Camera::set_fov_and_max_distance(LEti::Utility::HALF_PI, 50.0f);
	LEti::Camera::set_camera_data({ 0.0f, 2.0f, 2.0f }, { 0.0f, -2.0f, -1.0f });

	LEti::Resource_Loader::init();

	LEti::Resource_Loader::load_object("textures", "Resources/Textures/textures.mdl");

	LEti::Resource_Loader::load_object("red_cube", "Resources/Models/red_cube.mdl");
	LEti::Object_3D cube;
	cube.init("red_cube");

//	LEti::global_indicator.init("cube");

	LEti::Resource_Loader::load_object("cube", "Resources/Models/intersection_indicator_3d.mdl");

	///////////////// 3d collision test

	LEti::Resource_Loader::load_object("colliding_object", "Resources/Models/colliding_object.mdl");
	LEti::Object_3D coll_obj;
	coll_obj.init("colliding_object");

	LEti::Resource_Loader::load_object("pyramid", "Resources/Models/pyramid.mdl");
	LEti::Object_3D pyramid;
	pyramid.init("pyramid");

	LEti::Object_3D pyramid_far;
	pyramid_far.init("pyramid");
//	pyramid_far.move(5.0f, 5.0f, 5.0f);
	pyramid_far.move(0, 0, 0);

	LEti::Space_Splitter_3D::register_object(&pyramid);
	LEti::Space_Splitter_3D::register_object(&pyramid_far);
	LEti::Space_Splitter_3D::register_object(&coll_obj);

	glm::mat4x4 move
	(
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f
	);

	glm::mat4x4 fake_rotate = glm::rotate(pyramid.get_rotation_angle(), pyramid.get_rotation_axis());

	glm::mat4x4 kostyl_matrix
	(
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f
	);

	glm::vec3 point(0.3f, 0.3f, 0.3f);

	LEti::Resource_Loader::load_object("text_field", "Resources/Models/text_field.mdl");
	LEti::Text_Field intersection_info_block;
	intersection_info_block.init("text_field");


	float triangle_speed = 0.5f;

	bool intersection_detected = false;
	if(intersection_detected)
		intersection_info_block.set_text("Intersection detected");
	else
		intersection_info_block.set_text("Intersection not detected");

	while (!LEti::Event_Controller::window_should_close())
	{
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		LEti::Event_Controller::update();

		if (LEti::Event_Controller::key_was_pressed(GLFW_KEY_TAB))
			LEti::Camera::toggle_controll(LEti::Camera::get_controllable() ? false : true);
		LEti::Camera::update(false, true);

		if (LEti::Event_Controller::is_key_down(GLFW_KEY_LEFT))
		{
			pyramid.move((triangle_speed * LEti::Event_Controller::get_dt()), 0.0f, 0.0f);
		}
		if (LEti::Event_Controller::is_key_down(GLFW_KEY_RIGHT))
		{
			pyramid.move(-(triangle_speed * LEti::Event_Controller::get_dt()), 0.0f, 0.0f);
		}
		if (LEti::Event_Controller::is_key_down(GLFW_KEY_DOWN))
		{
			pyramid.move(0.0f, 0.0f, -(triangle_speed * LEti::Event_Controller::get_dt()));
		}
		if (LEti::Event_Controller::is_key_down(GLFW_KEY_UP))
		{
			pyramid.move(0.0f, 0.0f, (triangle_speed * LEti::Event_Controller::get_dt()));
		}
		if (LEti::Event_Controller::is_key_down(GLFW_KEY_I))
		{
			pyramid.move(0.0f, (triangle_speed * LEti::Event_Controller::get_dt()), 0.0f);
		}
		if (LEti::Event_Controller::is_key_down(GLFW_KEY_K))
		{
			pyramid.move(0.0f, -(triangle_speed * LEti::Event_Controller::get_dt()), 0.0f);
		}
		if (LEti::Event_Controller::is_key_down(GLFW_KEY_Q))
		{
			pyramid.rotate(LEti::Utility::QUARTER_PI * LEti::Event_Controller::get_dt());
		}
		if (LEti::Event_Controller::is_key_down(GLFW_KEY_E))
		{
			pyramid.rotate(- LEti::Utility::QUARTER_PI * LEti::Event_Controller::get_dt());
		}

		fake_rotate = glm::rotate(pyramid.get_rotation_angle(), pyramid.get_rotation_axis());
		glm::vec3 pyramid_pos = pyramid.get_pos();
		move[3][0] = pyramid_pos.x;
		move[3][1] = pyramid_pos.y;
		move[3][2] = pyramid_pos.z;

		pyramid_far.update();
		pyramid.update();
		coll_obj.update();

		LEti::Space_Splitter_3D::update();

		auto list = LEti::Space_Splitter_3D::get_collisions();
		auto it = list.begin();
		while(it != list.end())
		{
//			std::cout << it->first << ' ' << it->second << "\n";
			intersection_info_block.set_text(std::to_string(list.size()).c_str());
			cube.set_pos(it->collision_data.closest_intersection_point.x, it->collision_data.closest_intersection_point.y, it->collision_data.closest_intersection_point.z);
			cube.draw();
			++it;
		}
//		std::cout << "\n";

//		LEti::Physical_Model_Interface::Intersection_Data id = pyramid.is_colliding_with_other(coll_obj);


//		cube.set_visible(false);
//		std::string intersection_message;
//		if(id.type == LEti::Physical_Model_Interface::Intersection_Data::Intersection_Type::inside)
//			intersection_message += "PM is fully inside";
//		else if(id.type == LEti::Physical_Model_Interface::Intersection_Data::Intersection_Type::partly_outside)
//		{
//			intersection_message += "intersection at ";
//			intersection_message += std::to_string(id.closest_intersection_point.x);
//			intersection_message += ' ';
//			intersection_message += std::to_string(id.closest_intersection_point.y);
//			intersection_message += ' ';
//			intersection_message += std::to_string(id.closest_intersection_point.z);
//			cube.set_pos(id.closest_intersection_point.x, id.closest_intersection_point.y, id.closest_intersection_point.z);
////			cube.set_visible(true);
//		}
//		else
//			intersection_message += "no intersection";
//		intersection_info_block.set_text(intersection_message.c_str());

		pyramid_far.draw();
		pyramid.draw();
		coll_obj.draw();

//		cube.draw();

		intersection_info_block.draw();

		LEti::Event_Controller::swap_buffers();
	}

	return 0;
}*/

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
	LEti::Event_Controller::init_and_create_window(1200, 800, "Board Game");

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

	LEti::Camera::set_fov_and_max_distance(LEti::Utility::HALF_PI, 50.0f);
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
	float angle =  0.0f /*LEti::Utility::QUARTER_PI*/ /* * 1.75f*/;
	float speed = 50.0f;
	velocity.x = cos(angle);
	velocity.y = sin(angle);

	flat_co_2.init("flat_co_2");
	flat_co_2.set_pos(50, 400, 0);
//	flat_co_2.set_overall_scale(100.0f);
	flat_co_2.set_scale(1.0f, 5.0f, 1.0f);

	flat_co_3.init("flat_co_3");
//	flat_co_3.set_collision_possibility(false);
//	flat_co_3.set_visible(false);
//	flat_co_3.move(1000, 300, 0);
	flat_co_3.set_pos(1150, 400, 0);
	flat_co_3.set_scale(1.0f, 800.0f, 1.0f);


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

	LEti::Text_Field fps_info_block;
	fps_info_block.init("text_field");
	fps_info_block.set_pos(1150, 770, 0);

	flat_co.set_is_dynamic(true);
	flat_co_2.set_is_dynamic(true);
	flat_co_3.set_is_dynamic(true);
	LEti::Space_Splitter_2D::register_object(&flat_co);
	LEti::Space_Splitter_2D::register_object(&flat_co_2);
	LEti::Space_Splitter_2D::register_object(&flat_co_3);

	float triangle_speed = 100.0f;

//	bool intersection_detected = false;
//	if (intersection_detected)
//		intersection_info_block.set_text("Intersection detected");
//	else
//		intersection_info_block.set_text("Intersection not detected");

	bool moving_down_kostyl = false;

	bool flat_co_enabled = true;

	LEti::Timer fps_timer;
	unsigned int fps_counter = 0;

	while (!LEti::Event_Controller::window_should_close())
	{
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		LEti::Event_Controller::update();

		if (LEti::Event_Controller::key_was_pressed(GLFW_KEY_TAB))
			LEti::Camera::toggle_controll(LEti::Camera::get_controllable() ? false : true);
		LEti::Camera::update(false, true);

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

		if (LEti::Event_Controller::is_key_down(GLFW_KEY_F))
		{
			flat_co_3.move(-(triangle_speed * DT), 0.0f, 0.0f);
		}
		if (LEti::Event_Controller::is_key_down(GLFW_KEY_H))
		{
			flat_co_3.move((triangle_speed * DT), 0.0f, 0.0f);
		}
		if (LEti::Event_Controller::is_key_down(GLFW_KEY_G))
		{
			flat_co_3.move(0.0f, -(triangle_speed * DT), 0.0f);
		}
		if (LEti::Event_Controller::is_key_down(GLFW_KEY_T))
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
			angle += LEti::Utility::HALF_PI * LEti::Event_Controller::get_dt();
		}
		if (LEti::Event_Controller::is_key_down(GLFW_KEY_L))
		{
			angle -= LEti::Utility::HALF_PI * LEti::Event_Controller::get_dt();
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
			angle = LEti::Utility::DOUBLE_PI - angle;
		}
		else if(flat_co.get_pos().y <= 0.0f)
		{
			flat_co.set_pos(flat_co.get_pos().x, 1.0f, 0.0f);
			angle = LEti::Utility::DOUBLE_PI - angle;
		}

		if(flat_co.get_pos().x >= 1200.0f)
		{
			flat_co.set_pos(1199.0f, flat_co.get_pos().y, 0.0f);
			angle = LEti::Utility::DOUBLE_PI - angle + LEti::Utility::PI;
		}
		else if(flat_co.get_pos().x <= 0.0f)
		{
			flat_co.set_pos(1.0f, flat_co.get_pos().y, 0.0f);
			angle = LEti::Utility::DOUBLE_PI - angle + LEti::Utility::PI;
		}

		if(angle < 0.0f)
			angle += LEti::Utility::DOUBLE_PI;
		else if(angle > LEti::Utility::DOUBLE_PI)
			angle -= LEti::Utility::DOUBLE_PI;

		velocity.x = speed * cos(angle) * LEti::Event_Controller::get_dt();;
		velocity.y = speed * sin(angle) * LEti::Event_Controller::get_dt();;
		flat_co.move(velocity.x, velocity.y, 0.0f);

		flat_co_3.move(0.0f, moving_down_kostyl ? -1.0f : 1.0f, 0.0f);
		moving_down_kostyl = !moving_down_kostyl;

		flat_co.update();
		flat_co_2.update();
		flat_co_3.update();

		LEti::Space_Splitter_2D::update();

//		frame.draw();

		auto list = LEti::Space_Splitter_2D::get_collisions();

		auto it = list.begin();
		while(it != list.end())
		{
			ind.set_pos(it->collision_data.closest_intersection_point.x, it->collision_data.closest_intersection_point.y, 0.0f);
//			ind.draw();

			glm::vec3 diff = it->first->get_pos() - it->second->get_pos();
			if(fabs(diff.x) > fabs(diff.y))
				angle = LEti::Utility::DOUBLE_PI - angle + LEti::Utility::PI;
			else
				angle = LEti::Utility::DOUBLE_PI - angle;

			velocity.x = speed * cos(angle) * LEti::Event_Controller::get_dt() * (/*1.0f - */it->collision_data.time_of_intersection_ratio);
			velocity.y = speed * sin(angle) * LEti::Event_Controller::get_dt() * (/*1.0f - */it->collision_data.time_of_intersection_ratio);

			flat_co.move(velocity.x, velocity.y, 0.0f);
			flat_co.update();

			++it;
		}
//		if(list.size() == 0)
//			ind.set_pos(0, 0, 0);
//		std::cout << "\n";

		if(angle < 0.0f)
			angle += LEti::Utility::DOUBLE_PI;
		else if(angle > LEti::Utility::DOUBLE_PI)
			angle -= LEti::Utility::DOUBLE_PI;

		flat_co_2.draw();
		if(flat_co_enabled)
			flat_co.draw();
//		flat_co_foreshadow.set_pos(flat_co.get_pos().x + velocity.x, flat_co.get_pos().y + velocity.y, 0.0f);
//		flat_co_foreshadow.draw();
		flat_co_3.draw();

		ind.draw();

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
		fps_info_block.update();
		fps_info_block.draw();

		LEti::Event_Controller::swap_buffers();
	}

	return 0;
}


/*#include <iostream>
#include <vector>

#include "Tree.h"

struct vec
{
	float x, y;

	vec(float _x, float _y) : x(_x), y(_y) { }
	vec(const vec& _other) : x(_other.x), y(_other.y) { }
};

struct physical_model
{
	vec p1, p2, p3;

	physical_model(const vec& _1, const vec& _2, const vec& _3) : p1(_1), p2(_2), p3(_3) { }
};

struct border_data
{
	bool inf = true;
	float value = 0.0f;

	border_data() { }
	border_data(float _v) : inf(false), value(_v) { }
	void operator=(float _v) { value = _v; inf = false; }

	operator bool() const { return !inf; }
};

struct area_data
{
	border_data left, right, top, bottom;
	std::vector<const physical_model*> models;

	bool point_is_inside(vec _point)
	{
		bool result = true;
		if(left) result = result && left.value < _point.x;
		if(right) result = result && right.value > _point.x;
		if(top) result = result && top.value > _point.y;
		if(bottom) result = result && bottom.value < _point.y;
		return result;
	}

	friend std::ostream& operator<<(std::ostream& _stream, const area_data& _data);
};
std::ostream& operator<<(std::ostream& _stream, const area_data& _data)
{
	_stream << "\ttop border: ";
	if(_data.top) _stream << _data.top.value; else _stream << "inf";
	_stream << '\n';
	_stream << "left border: ";
	if(_data.left) _stream << _data.left.value; else _stream << "inf";
	_stream << "   ";
	_stream << "right border: ";
	if(_data.right) _stream << _data.right.value; else _stream << "inf";
	_stream << '\n';
	_stream << "\tbottom border: ";
	if(_data.bottom) _stream << _data.bottom.value; else _stream << "inf";
	_stream << '\n';

	_stream << "list of physical models:\n";
	for(unsigned int i=0; i<_data.models.size(); ++i)
		_stream << '\t' << _data.models[i] << '\n';

	return _stream;
}


struct quadratic_border
{
	float left, right, top, bottom;

	quadratic_border(const physical_model& _pm)
	{
		left = _pm.p1.x;
		right = _pm.p1.x;
		top = _pm.p1.y;
		bottom = _pm.p1.y;

		if (left > _pm.p2.x) left = _pm.p2.x;
		if (right < _pm.p2.x) right = _pm.p2.x;
		if (top < _pm.p2.y) top = _pm.p2.y;
		if (bottom > _pm.p2.y) bottom = _pm.p2.y;

		if (left > _pm.p3.x) left = _pm.p3.x;
		if (right < _pm.p3.x) right = _pm.p3.x;
		if (top < _pm.p3.y) top = _pm.p3.y;
		if (bottom > _pm.p3.y) bottom = _pm.p3.y;
	}

	bool is_inside(const area_data& _ad)
	{
		bool result = true;
		if(_ad.left) result = result&& _ad.left.value < right;
		if(_ad.right) result = result && _ad.right.value > left;
		if(_ad.top) result = result && _ad.top.value > bottom;
		if(_ad.bottom) result = result && _ad.bottom.value < top;
		return result;
	}

	vec left_top() const { return {left, top}; }
	vec left_bottom() const { return {left, bottom}; }
	vec right_top() const { return {right, top}; }
	vec right_bottom() const { return {right, bottom}; }
};


class space_splitter
{
private:
	LEti::Tree<area_data, 4> m_tree;
	std::vector<const physical_model*> m_models;

	void split_space(LEti::Tree<area_data, 4>::Iterator it)
	{
		const auto& models = it->models;
		if (models.size() < 3) return;

		for(unsigned int i=0; i<models.size(); ++i)
		{
			quadratic_border qb(*models[i]);

			vec split_point{0.0f, 0.0f};
			if(it->point_is_inside(qb.left_top())) split_point = qb.left_top();
			else if(it->point_is_inside(qb.left_bottom())) split_point = qb.left_bottom();
			else if(it->point_is_inside(qb.right_top())) split_point = qb.right_top();
			else if(it->point_is_inside(qb.right_bottom())) split_point = qb.right_bottom();
			else continue;

			LEti::Tree<area_data, 4>::Iterator next = it;
			next.descend(it.insert_into_availible_index({}));
			next->left = it->left;
			next->top = it->top;
			next->right = split_point.x;
			next->bottom = split_point.y;

			next.ascend();
			next.descend(it.insert_into_availible_index({}));
			next->left = split_point.x;
			next->top = it->top;
			next->right = it->right;
			next->bottom = split_point.y;

			next.ascend();
			next.descend(it.insert_into_availible_index({}));
			next->left = split_point.x;
			next->top = split_point.y;
			next->right = it->right;
			next->bottom = it->bottom;

			next.ascend();
			next.descend(it.insert_into_availible_index({}));
			next->left = it->left;
			next->top = split_point.y;
			next->right = split_point.x;
			next->bottom = it->bottom;

			break;
		}

		for(unsigned int i=0; i<4; ++i)
		{
			LEti::Tree<area_data, 4>::Iterator next = it;
			next.descend(i);

			for(unsigned int m=0; m<models.size(); ++m)
			{
				quadratic_border qb(*models[m]);
				if(qb.is_inside(*next))
					next->models.push_back(models[m]);
			}

			split_space(next);
		}
	}

public:
	void register_model(const physical_model* _pm)
	{
		m_models.push_back(_pm);
	}

	LEti::Tree<area_data, 4>::Const_Iterator construct()
	{
		area_data temp;
		for (unsigned int i = 0; i < m_models.size(); ++i)
			temp.models.push_back(m_models[i]);

		LEti::Tree<area_data, 4>::Iterator it = m_tree.create_iterator();
		if(it.valid()) it.delete_branch();
		it = m_tree.create_iterator();

		it.insert_into_availible_index(temp);

		split_space(it);

		return it;
	}

};


int main()
{
	physical_model pm(
		{6, 1},
		{12, 2},
		{10, 3}
	);
	physical_model pm2(
		{1, 4},
		{7, 4},
		{3, 9}
	);
	physical_model pm3(
		{2, 10},
		{3, 5},
		{5, 11}
	);


	space_splitter ss;
	ss.register_model(&pm3);
	ss.register_model(&pm2);
	ss.register_model(&pm);

	auto it = ss.construct();

	unsigned int i=0;
	while(!it.end())
	{
		std::cout << "area_data #" << i << ":\n\n" << *it << "\n";
		++it;
		++i;
	}
	std::cout << "area_data #" << i << ":\n\n" << *it << "\n";


	return 0;
}*/



/*#include "Tree.h"
#include <iostream>


int main()
{
	LEti::Tree<int, 3> tree;

	LEti::Tree<int, 3>::Iterator it = tree.create_iterator();

//	for(unsigned int i=0; i<3; ++i)
//		it.insert_into_availible_index(i);

	LEti::Tree<int, 3>::Const_Iterator cit = tree.create_iterator();

	LEti::Tree<int, 3>::Iterator* it_casted = (LEti::Tree<int, 3>::Iterator*)&cit;
	for(unsigned int i=0; i<3; ++i)
		it_casted->insert_into_availible_index(i);

//	it_casted

	LEti::Tree<int, 3>::Iterator result_it = tree.create_iterator();
	while(!result_it.end())
	{
		std::cout << *result_it << "\n";
		++result_it;
	}
	std::cout << *result_it << "\n";

	return 0;
}*/





















































