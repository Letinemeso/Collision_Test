#include "Event_Controller.h"
#include "Shader.h"
#include "Camera.h"
#include "Resource_Loader.h"

#include "Object.h"

#include "Physical_Model.h"


int main()
{
	LEti::Event_Controller::init_and_create_window(1200, 800, "Board Game");

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_DEPTH_TEST);

	glEnable(GL_CULL_FACE);
	glCullFace(GL_CW);

	LEti::Shader::init_shader("resources/shaders/vertex_shader.shader", "resources/shaders/fragment_shader.shader");
	ASSERT(!LEti::Shader::is_valid());
	LEti::Shader::set_texture_uniform_name("input_texture");
	LEti::Shader::set_transform_matrix_uniform_name("transform_matrix");
	LEti::Shader::set_projection_matrix_uniform_name("projection_matrix");

	// LEti::Camera::setup_orthographic_matrix();

	LEti::Camera::set_fov_and_max_distance(LEti::Utility::HALF_PI, 50.0f);
	LEti::Camera::set_camera_data({ 0.0f, 2.0f, 2.0f }, { 0.0f, -2.0f, -1.0f });

	/*LEti::Resource_Loader::load_object("quad", "resources/models/quad.mdl");
	LEti::Object quad;
	quad.init("quad");*/




	LEti::Resource_Loader::load_object("pyramid", "Resources/Models/pyramid.mdl");
	LEti::Object pyramid;
	pyramid.init("pyramid");




	float pm_coords[36] =
	{
		0, 1, 0,	0, 0, 0,	0, 0, 1,	//A D C

		0, 1, 0,	1, 0, 0,	0, 0, 1,	//A B C

		0, 1, 0,	1, 0, 0,	0, 0, 0,	//A C D

		0, 0, 1,	1, 0, 0,	0, 0, 0		//C B D
	};

	glm::mat4x4 move
	(
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 4.0f, 0.0f, 1.0f
	);

	glm::mat4x4 kostyl_matrix
	(
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f
	);

	LEti::Physical_Model pm;
	pm.setup(LEti::Resource_Loader::get_data<float>("pyramid", "coords").first, 36);
	pm.update(move, kostyl_matrix, kostyl_matrix);

	std::cout << pm.is_intersecting_with_point({ 0.3f, 4.3f, 0.3f });





	while (!LEti::Event_Controller::window_should_close())
	{
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		LEti::Event_Controller::update();

		if (LEti::Event_Controller::key_was_pressed(GLFW_KEY_TAB))
			LEti::Camera::toggle_controll(LEti::Camera::get_controllable() ? false : true);
		LEti::Camera::update(false, true);

		if (LEti::Event_Controller::key_was_pressed(GLFW_KEY_LEFT))
			pyramid.move(-0.1f, 0.0f, 0.0f);
		if (LEti::Event_Controller::key_was_pressed(GLFW_KEY_RIGHT))
			pyramid.move(0.1f, 0.0f, 0.0f);
		if (LEti::Event_Controller::key_was_pressed(GLFW_KEY_DOWN))
			pyramid.move(0.0f, 0.0f, 0.1f);
		if (LEti::Event_Controller::key_was_pressed(GLFW_KEY_UP))
			pyramid.move(0.0f, 0.0f, -0.1f);

		// quad.draw();
		pyramid.draw();

		LEti::Event_Controller::swap_buffers();
	}

	return 0;
}