#include <Controllers/Camera_Controller.h>

#include <glfw3.h>

#include <Window/Window_Controller.h>

using namespace Shardis;

namespace Shardis
{
    constexpr float Movement_Speed = 5.0f;

    constexpr float View_Rotation_Speed = LEti::Math::PI;
    constexpr float Cursor_Movement_Threshold_For_View = 1920.0f;
}


void Camera_Controller::M_process_utility(float _dt)
{
    if(LR::Window_Controller::instance().key_was_pressed(GLFW_KEY_TAB))
        LR::Window_Controller::instance().set_cursor_visibility(!LR::Window_Controller::instance().is_cursor_visible());

    if(LR::Window_Controller::instance().key_was_pressed(GLFW_KEY_F))
        m_follow_mode = !m_follow_mode;
}

void Camera_Controller::M_process_movement(float _dt)
{
    if(m_follow_mode)
        return;

    LR::Camera_3D::Settings& camera_settings = m_camera->settings();
    const glm::vec3& look_direction = m_camera->settings().direction;

    glm::vec3 movement = { 0.0f, 0.0f, 0.0f };

    if(LR::Window_Controller::instance().is_key_down(GLFW_KEY_W))
    {
        movement += look_direction * Movement_Speed;
    }
    if(LR::Window_Controller::instance().is_key_down(GLFW_KEY_S))
    {
        movement -= look_direction * Movement_Speed;
    }
    if(LR::Window_Controller::instance().is_key_down(GLFW_KEY_A))
    {
        glm::vec3 stride = LEti::Math::rotate_vector(look_direction * Movement_Speed, camera_settings.top, LEti::Math::HALF_PI);
        movement += stride;
    }
    if(LR::Window_Controller::instance().is_key_down(GLFW_KEY_D))
    {
        glm::vec3 stride = LEti::Math::rotate_vector(look_direction * Movement_Speed, camera_settings.top, -LEti::Math::HALF_PI);
        movement += stride;
    }
    if(LR::Window_Controller::instance().is_key_down(GLFW_KEY_SPACE))
    {
        movement += camera_settings.top * Movement_Speed;
    }
    if(LR::Window_Controller::instance().is_key_down(GLFW_KEY_LEFT_CONTROL))
    {
        movement += -camera_settings.top * Movement_Speed;
    }

    movement *= _dt;

    camera_settings.position += movement;
    m_camera->reconfigure();
}

void Camera_Controller::M_process_view(float _dt)
{
    if(LR::Window_Controller::instance().is_cursor_visible())
        return;

    glm::vec2 cursor_stride = LR::Window_Controller::instance().get_cursor_stride();

    float ratio = -cursor_stride.x / Cursor_Movement_Threshold_For_View;
    float rotation = ratio * View_Rotation_Speed;

    constexpr glm::vec3 rotation_axis = {0.0f, 1.0f, 0.0f};

    LR::Camera_3D::Settings& camera_settings = m_camera->settings();
    camera_settings.direction = LEti::Math::rotate_vector(camera_settings.direction, rotation_axis, rotation);
    camera_settings.top = LEti::Math::rotate_vector(camera_settings.top, rotation_axis, rotation);

    ratio = -cursor_stride.y / Cursor_Movement_Threshold_For_View;
    rotation = ratio * View_Rotation_Speed;
    glm::vec3 perpendicular = LEti::Math::cross_product(camera_settings.direction, camera_settings.top);
    LEti::Math::shrink_vector_to_1(perpendicular);
    camera_settings.direction = LEti::Math::rotate_vector(camera_settings.direction, perpendicular, rotation);
    camera_settings.top = LEti::Math::rotate_vector(camera_settings.top, perpendicular, rotation);

    if(!m_follow_mode || !m_position_getter_func)
    {
        m_camera->reconfigure();
        return;
    }

    constexpr float camera_distance = 5.0f;

    camera_settings.position = m_position_getter_func();
    camera_settings.position -= camera_settings.direction * camera_distance;

    m_camera->reconfigure();
}



void Camera_Controller::update(float _dt)
{
    L_ASSERT(m_camera);

    M_process_utility(_dt);
    M_process_movement(_dt);
    M_process_view(_dt);
}
