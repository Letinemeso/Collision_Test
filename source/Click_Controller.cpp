#include <Click_Controller.h>

#include <Window/Window_Controller.h>
#include <glfw3.h>


Click_Controller::Click_Controller()
{

}

Click_Controller::~Click_Controller()
{
    for(Objects_List::Iterator it = m_objects_list.begin(); !it.end_reached(); ++it)
        delete *it;

    delete m_object_stub;
}



void Click_Controller::M_process_mouse_click()
{
    if(!LR::Window_Controller::mouse_button_was_pressed(GLFW_MOUSE_BUTTON_1))
        return;

    glm::vec3 clicked_at = m_camera->convert_window_coords({ LR::Window_Controller::get_cursor_position().x, LR::Window_Controller::get_cursor_position().y, 0.0f });
    LEti::Object* object = LEti::Object_Stub::construct_from(m_object_stub);
    object->current_state().set_position(clicked_at);
    object->update(0.0f);
    object->update_previous_state();
    m_objects_list.push_back(object);
}



void Click_Controller::update(float _dt)
{
    M_process_mouse_click();


    for(Objects_List::Iterator it = m_objects_list.begin(); !it.end_reached(); ++it)
    {
        LEti::Object* object = *it;
        object->update(_dt);
    }
}
