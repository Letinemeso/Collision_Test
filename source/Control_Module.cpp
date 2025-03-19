#include <Control_Module.h>

#include <Math_Stuff.h>

#include <Window/Window_Controller.h>
#include <glfw3.h>


void Control_Module::update(float _dt)
{
    glm::vec3 movement_vec = {0.0f, 0.0f, 0.0f};

    bool has_movement = false;
    if(LR::Window_Controller::is_key_down(GLFW_KEY_W))
    {
        has_movement = true;
        movement_vec.y += 1.0f;
    }
    if(LR::Window_Controller::is_key_down(GLFW_KEY_A))
    {
        has_movement = true;
        movement_vec.x += -1.0f;
    }
    if(LR::Window_Controller::is_key_down(GLFW_KEY_S))
    {
        has_movement = true;
        movement_vec.y += -1.0f;
    }
    if(LR::Window_Controller::is_key_down(GLFW_KEY_D))
    {
        has_movement = true;
        movement_vec.x += 1.0f;
    }

    if(!has_movement)
        return;

    LEti::Math::shrink_vector_to_1(movement_vec);

    transformation_data()->move(movement_vec * m_speed * _dt);
}
