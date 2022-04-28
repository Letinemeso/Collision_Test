#include "Event_Controller.h"
#include "Shader.h"
#include "Camera.h"
#include "Resource_Loader.h"

#include "Object.h"
#include "Text_Field.h"

#include "Physical_Model.h"


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

	/*LEti::Resource_Loader::load_object("quad", "resources/models/quad.mdl");
	LEti::Object quad;
	quad.init("quad");*/

    LEti::Resource_Loader::load_object("textures", "Resources/Textures/textures.mdl");

	LEti::Resource_Loader::load_object("colliding_object", "Resources/Models/colliding_object.mdl");
	LEti::Object coll_obj;
	coll_obj.init("colliding_object");

    auto static_pm_data = LEti::Resource_Loader::get_data<float>("colliding_object", "coords");
    LEti::Physical_Model static_pm;
    static_pm.setup(static_pm_data.first, static_pm_data.second);



	LEti::Resource_Loader::load_object("pyramid", "Resources/Models/pyramid.mdl");
	LEti::Object pyramid;
	pyramid.init("pyramid");




	glm::mat4x4 move
	(
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f
	);

	glm::mat4x4 kostyl_matrix
	(
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f
	);

	glm::vec3 point(0.3f, 0.3f, 0.3f);

	LEti::Physical_Model pm;
    pm.setup(LEti::Resource_Loader::get_data<float>("pyramid", "coords").first, 72);
	pm.update(move, kostyl_matrix, kostyl_matrix);
//	std::cout << pm.is_intersecting_with_point(point) << "\n";

    static_pm.update(kostyl_matrix, kostyl_matrix, kostyl_matrix);



    LEti::Resource_Loader::load_object("text_field", "Resources/Models/text_field.mdl");
    LEti::Text_Field intersection_info_block;
    intersection_info_block.init("text_field");

    bool intersection_detected = pm.is_intersecting_with_another_model(static_pm);
    if(intersection_detected)
        intersection_info_block.set_text("Intersection detected");
    else
        intersection_info_block.set_text("Intersection not detected");

	std::cout.precision(1);
	while (!LEti::Event_Controller::window_should_close())
	{
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		LEti::Event_Controller::update();

		if (LEti::Event_Controller::key_was_pressed(GLFW_KEY_TAB))
			LEti::Camera::toggle_controll(LEti::Camera::get_controllable() ? false : true);
		LEti::Camera::update(false, true);

		if (LEti::Event_Controller::key_was_pressed(GLFW_KEY_LEFT))
		{
			pyramid.move(-0.1f, 0.0f, 0.0f);
			move[3][0] -= 0.1f;
			pm.update(move, kostyl_matrix, kostyl_matrix);
//			std::cout /*<< std::fixed << move[3][0] << ' ' << move[3][2] << '\t'*/ << pm.is_intersecting_with_point(point) << "\n";
		}
		if (LEti::Event_Controller::key_was_pressed(GLFW_KEY_RIGHT))
		{
			pyramid.move(0.1f, 0.0f, 0.0f);
			move[3][0] += 0.1f;
            pm.update(move, kostyl_matrix, kostyl_matrix);
//            std::cout /*<< std::fixed << move[3][0] << ' ' << move[3][2] << '\t'*/ << pm.is_intersecting_with_point(point) << "\n";
		}
		if (LEti::Event_Controller::key_was_pressed(GLFW_KEY_DOWN))
		{
			pyramid.move(0.0f, 0.0f, 0.1f);
			move[3][2] += 0.1f;
			pm.update(move, kostyl_matrix, kostyl_matrix);
//            std::cout /*<< std::fixed << move[3][0] << ' ' << move[3][2] << '\t'*/ << pm.is_intersecting_with_point(point) << "\n";
		}
		if (LEti::Event_Controller::key_was_pressed(GLFW_KEY_UP))
		{
			pyramid.move(0.0f, 0.0f, -0.1f);
			move[3][2] -= 0.1f;
			pm.update(move, kostyl_matrix, kostyl_matrix);
//            std::cout /*<< std::fixed << move[3][0] << ' ' << move[3][2] << '\t'*/ << pm.is_intersecting_with_point(point) << "\n";
		}
        if (LEti::Event_Controller::key_was_pressed(GLFW_KEY_I))
        {
            pyramid.move(0.0f, 0.1f, 0.0f);
            move[3][1] += 0.1f;
            pm.update(move, kostyl_matrix, kostyl_matrix);
//            std::cout /*<< std::fixed << move[3][0] << ' ' << move[3][2] << '\t'*/ << pm.is_intersecting_with_point(point) << "\n";
        }
        if (LEti::Event_Controller::key_was_pressed(GLFW_KEY_K))
        {
            pyramid.move(0.0f, -0.1f, 0.0f);
            move[3][1] -= 0.1f;
            pm.update(move, kostyl_matrix, kostyl_matrix);
//            std::cout /*<< std::fixed << move[3][0] << ' ' << move[3][2] << '\t'*/ << pm.is_intersecting_with_point(point) << "\n";
        }

//        std::cout << pm.is_intersecting_with_another_model(static_pm) << '\n';

        if(static_pm.is_intersecting_with_another_model(pm) && !intersection_detected)
        {
            intersection_info_block.set_text("Intersection detected");
            intersection_detected = true;
        }
        else if(!static_pm.is_intersecting_with_another_model(pm) && intersection_detected)
        {
            intersection_info_block.set_text("Intersection not detected");
            intersection_detected = false;
        }

//        std::cout << "look intersection: " << pm.is_intersecting_with_beam(LEti::Camera::get_pos(), LEti::Camera::get_look_direction()) << "\n";



		// quad.draw();
		pyramid.draw();
		coll_obj.draw();

        intersection_info_block.draw();

		LEti::Event_Controller::swap_buffers();
	}

	return 0;
}
