#include "Event_Controller.h"
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

    LEti::Message_Translator::register_message_type<On_Button_Pressed_Msg>();
//    LEti::Message_Translator::subscribe(On_Button_Pressed_Msg::type(), print_some_shit_om_btn_pressed);

    LEti::Resource_Loader::load_object("textures", "Resources/Textures/textures.mdl");

    /*
	LEti::Resource_Loader::load_object("colliding_object", "Resources/Models/colliding_object.mdl");
	LEti::Object coll_obj;
	coll_obj.init("colliding_object");

    auto static_pm_data = LEti::Resource_Loader::get_data<float>("colliding_object", "coords");
    LEti::Physical_Model_Interface* static_pm = new LEti::Physical_Model_3D;
    static_pm->setup(static_pm_data.first, static_pm_data.second);



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

    LEti::Physical_Model_3D pm;
    pm.setup(LEti::Resource_Loader::get_data<float>("pyramid", "coords").first, 72);
    pm.update(move, kostyl_matrix, kostyl_matrix);

    static_pm->update(kostyl_matrix, kostyl_matrix, kostyl_matrix);
    */

    //////////////////////

    LEti::Resource_Loader::load_object("co_2d", "Resources/Models/flat_co.mdl");
    LEti::Object co_2d;
    co_2d.init("co_2d");

    auto data = LEti::Resource_Loader::get_data<float>("co_2d", "coords");
    LEti::Physical_Model_Interface* co_2d_fm = new LEti::Physical_Model_2D;
    co_2d_fm->setup(data.first, data.second);

    LEti::Resource_Loader::load_object("co_2d_2", "Resources/Models/flat_co_2.mdl");
    LEti::Object co_2d_2;
    co_2d_2.init("co_2d_2");

    auto data2 = LEti::Resource_Loader::get_data<float>("co_2d_2", "coords");
    LEti::Physical_Model_Interface* co_2d_fm_2 = new LEti::Physical_Model_2D;
    co_2d_fm_2->setup(data2.first, data2.second);

    glm::mat4x4 move
    (
        1.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f
    );
    glm::mat4x4 fake
    (
        1.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f
    );
    glm::mat4x4 fake_size
    (
        0.3f, 0.0f, 0.0f, 0.0f,
        0.0f, 0.3f, 0.0f, 0.0f,
        0.0f, 0.0f, 0.3f, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f
    );

    glm::vec3 axis(0.0f, 0.0f, 1.0f);
    float angle = 0.0f;
    glm::mat4x4 rotation = glm::rotate(angle, axis);

    co_2d_fm->update(fake, fake, fake);
    co_2d_fm_2->update(fake, fake_size, rotation);
    co_2d_2.set_overall_scale(0.3f);

    /////////////////////

    LEti::Resource_Loader::load_object("text_field", "Resources/Models/text_field.mdl");
    LEti::Text_Field intersection_info_block;
    intersection_info_block.init("text_field");

    LEti::Resource_Loader::load_object("indicator", "Resources/Models/intersection_point_indicator.mdl");
    LEti::Object indicator;
    indicator.init("indicator");
    indicator.set_visible(false);

    LEti::Message_Translator::subscribe<On_Button_Pressed_Msg>([&](const On_Button_Pressed_Msg& _msg)
    {
        if(_msg.btn == GLFW_KEY_LEFT)
        {
            co_2d_2.move(-25, 0, 0);
            move[3][0] -= 25;
        }
        if(_msg.btn == GLFW_KEY_RIGHT)
        {
            co_2d_2.move(25, 0, 0);
            move[3][0] += 25;
        }
    });

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

		if (LEti::Event_Controller::key_was_pressed(GLFW_KEY_LEFT))
		{
//			pyramid.move(-0.1f, 0.0f, 0.0f);
//			move[3][0] -= 0.1f;
//			pm.update(move, kostyl_matrix, kostyl_matrix);

//            co_2d_2.move(-25, 0, 0);
//            move[3][0] -= 25;
            LEti::Message_Translator::publish(On_Button_Pressed_Msg(GLFW_KEY_LEFT));
		}
		if (LEti::Event_Controller::key_was_pressed(GLFW_KEY_RIGHT))
		{
//			pyramid.move(0.1f, 0.0f, 0.0f);
//			move[3][0] += 0.1f;
//            pm.update(move, kostyl_matrix, kostyl_matrix);
//            co_2d_2.move(25, 0, 0);
//            move[3][0] += 25;
            LEti::Message_Translator::publish(On_Button_Pressed_Msg(GLFW_KEY_RIGHT));
		}
		if (LEti::Event_Controller::key_was_pressed(GLFW_KEY_DOWN))
		{
//			pyramid.move(0.0f, 0.0f, 0.1f);
//			move[3][2] += 0.1f;
//			pm.update(move, kostyl_matrix, kostyl_matrix);
            co_2d_2.move(0, -25, 0);
            move[3][1] -= 25;
		}
		if (LEti::Event_Controller::key_was_pressed(GLFW_KEY_UP))
		{
//			pyramid.move(0.0f, 0.0f, -0.1f);
//			move[3][2] -= 0.1f;
//			pm.update(move, kostyl_matrix, kostyl_matrix);
            co_2d_2.move(0, 25, 0);
            move[3][1] += 25;
		}
        if (LEti::Event_Controller::key_was_pressed(GLFW_KEY_I))
        {
            LEti::Message_Translator::publish(On_Button_Pressed_Msg('I'));
//            pyramid.move(0.0f, 0.1f, 0.0f);
//            move[3][1] += 0.1f;
//            pm.update(move, kostyl_matrix, kostyl_matrix);
        }
        if (LEti::Event_Controller::key_was_pressed(GLFW_KEY_K))
        {
            LEti::Message_Translator::publish(On_Button_Pressed_Msg('K'));
//            pyramid.move(0.0f, -0.1f, 0.0f);
//            move[3][1] -= 0.1f;
//            pm.update(move, kostyl_matrix, kostyl_matrix);
        }
        if (LEti::Event_Controller::key_was_pressed(GLFW_KEY_Q))
        {
            angle += LEti::Utility::QUARTER_PI / 4.0f;
            co_2d_2.set_rotation_axis(axis.x, axis.y, axis.z);
            co_2d_2.set_rotation_angle(angle);
            rotation = glm::rotate(angle, axis);
        }
        if (LEti::Event_Controller::key_was_pressed(GLFW_KEY_E))
        {
            angle -= LEti::Utility::QUARTER_PI / 4.0f;
            co_2d_2.set_rotation_axis(axis.x, axis.y, axis.z);
            co_2d_2.set_rotation_angle(angle);
            rotation = glm::rotate(angle, axis);
        }

//        auto pos = co_2d_2.get_pos();
        co_2d_fm_2->update(move, fake_size, rotation);

//        if(static_pm->is_intersecting_with_another_model(pm) && !intersection_detected)
//        {
//            intersection_info_block.set_text("Intersection detected");
//            intersection_detected = true;
//        }
//        else if(!static_pm->is_intersecting_with_another_model(pm) && intersection_detected)
//        {
//            intersection_info_block.set_text("Intersection not detected");
//            intersection_detected = false;
//        }

        LEti::Physical_Model_Interface::Intersection_Data id = co_2d_fm_2->is_intersecting_with_another_model(*co_2d_fm);
//        if(id && !intersection_detected)
//        {
//            std::string intersection_message("detected ");
//            if(id.type == LEti::Physical_Model_Interface::Intersection_Data::Intersection_Type::inside)
//                intersection_message += "PM is fully inside";
//            else if(id.type == LEti::Physical_Model_Interface::Intersection_Data::Intersection_Type::partly_outside)
//            {
//                intersection_message += "intsc crds ";
//                intersection_message += std::to_string(id.closest_intersection_point.x);
//                intersection_message += ' ';
//                intersection_message += std::to_string(id.closest_intersection_point.y);
//                intersection_message += ' ';
//                intersection_message += std::to_string(id.closest_intersection_point.z);
//            }

//            intersection_info_block.set_text(/*"Intersection detected"*/ intersection_message.c_str());
//            intersection_detected = true;
//        }
//        else if(!id && intersection_detected)
//        {
//            intersection_info_block.set_text("Intersection not detected");
//            intersection_detected = false;
//        }
        indicator.set_visible(false);
        std::string intersection_message;
        if(id.type == LEti::Physical_Model_Interface::Intersection_Data::Intersection_Type::inside)
            intersection_message += "PM is fully inside";
        else if(id.type == LEti::Physical_Model_Interface::Intersection_Data::Intersection_Type::partly_outside)
        {
            intersection_message += "intsc crds ";
            intersection_message += std::to_string(id.closest_intersection_point.x);
            intersection_message += ' ';
            intersection_message += std::to_string(id.closest_intersection_point.y);
            intersection_message += ' ';
            intersection_message += std::to_string(id.closest_intersection_point.z);
            indicator.set_pos(id.closest_intersection_point.x, id.closest_intersection_point.y, id.closest_intersection_point.z);
            indicator.set_visible(true);
        }
        else
            intersection_message += "no intersection";
        intersection_info_block.set_text(/*"Intersection detected"*/ intersection_message.c_str());

//        std::cout << "look intersection: " << pm.is_intersecting_with_beam(LEti::Camera::get_pos(), LEti::Camera::get_look_direction()) << "\n";


        glDisable(GL_DEPTH_TEST);
        co_2d.draw();
        co_2d_2.draw();

        indicator.draw();

		// quad.draw();
//		pyramid.draw();
//		coll_obj.draw();

        intersection_info_block.draw();

		LEti::Event_Controller::swap_buffers();
	}

//    delete static_pm;

	return 0;
}
