/*#include "Event_Controller.h"
#include "Shader.h"
#include "Camera.h"
#include "Resource_Loader.h"

#include "Object.h"
#include "Text_Field.h"

#include "Physical_Model_3D.h"
#include "Physical_Model_2D.h"

#include "Message_Translator.h"

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

	LEti::Resource_Loader::load_object("cube", "Resources/Models/intersection_indicator_3d.mdl");
	LEti::Object_3D cube;
	cube.init("cube");
	cube.set_visible(false);

	///////////////// 3d collision test

	LEti::Resource_Loader::load_object("colliding_object", "Resources/Models/colliding_object.mdl");
	LEti::Object_3D coll_obj;
	coll_obj.init("colliding_object");

	LEti::Resource_Loader::load_object("pyramid", "Resources/Models/pyramid.mdl");
	LEti::Object_3D pyramid;
	pyramid.init("pyramid");

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
			pyramid.move(-(triangle_speed * LEti::Event_Controller::get_dt()), 0.0f, 0.0f);
		}
		if (LEti::Event_Controller::is_key_down(GLFW_KEY_RIGHT))
		{
			pyramid.move((triangle_speed * LEti::Event_Controller::get_dt()), 0.0f, 0.0f);
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

		LEti::Physical_Model_Interface::Intersection_Data id = pyramid.is_colliding_with_other(coll_obj);


		cube.set_visible(false);
		std::string intersection_message;
		if(id.type == LEti::Physical_Model_Interface::Intersection_Data::Intersection_Type::inside)
			intersection_message += "PM is fully inside";
		else if(id.type == LEti::Physical_Model_Interface::Intersection_Data::Intersection_Type::partly_outside)
		{
			intersection_message += "intersection at ";
			intersection_message += std::to_string(id.closest_intersection_point.x);
			intersection_message += ' ';
			intersection_message += std::to_string(id.closest_intersection_point.y);
			intersection_message += ' ';
			intersection_message += std::to_string(id.closest_intersection_point.z);
			cube.set_pos(id.closest_intersection_point.x, id.closest_intersection_point.y, id.closest_intersection_point.z);
			cube.set_visible(true);
		}
		else
			intersection_message += "no intersection";
		intersection_info_block.set_text(intersection_message.c_str());


		pyramid.update();
		coll_obj.update();

		pyramid.draw();
		coll_obj.draw();

		cube.draw();

		intersection_info_block.draw();

		LEti::Event_Controller::swap_buffers();
	}

	return 0;
}
*/


#include <iostream>
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
}



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





















































