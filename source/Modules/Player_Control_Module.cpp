#include <Modules/Player_Control_Module.h>

#include <glfw3.h>
#include <gtc/quaternion.hpp>

#include <Object.h>

#include <Modules/Attack_Module.h>
#include <Tools/Entity_Tools.h>

using namespace Shardis;

namespace Shardis
{
    constexpr float Attack_Prepare_Duration = 0.06f;
}


Player_Control_Module::Player_Control_Module()
{
    m_message_handle__update_camera = LST::Message_Translator::instance().subscribe<Message__Update_Camera>([this](Message__Update_Camera& _msg)
    {
        M_reconfigure_camera();
        M_update_first_person_animation_trasformation();

        m_light_source_module->update(0.0f);
    });

    m_message_handle__damaged = LST::Message_Translator::instance().subscribe<Message__On_Entity_Damaged>([this](Message__On_Entity_Damaged& _msg)
    {
        if(_msg.damaged_entity != parent_object())
            return;

        m_thrust_prepare_timer.reset();
    });
}

Player_Control_Module::~Player_Control_Module()
{
    LST::Message_Translator::instance().unsubscribe(m_message_handle__update_camera);

    delete m_weapon;
    delete m_first_person_dm;
}



void Player_Control_Module::set_weapon(Weapon* _ptr)
{
    L_ASSERT(_ptr);

    delete m_weapon;
    m_weapon = _ptr;

    delete m_first_person_dm;
    m_first_person_dm = LR::Draw_Module_Stub::construct_from( m_weapon->first_person_animation_stub() );

    m_first_person_dm->set_transformation_data(&m_first_person_dm_transformation);
    m_first_person_dm->set_transformation_data_prev_state(&m_first_person_dm_transformation);
}



void Player_Control_Module::M_on_parent_object_set()
{
    m_velocity_module = parent_object()->get_module_of_type<Velocity_Module>();
    L_ASSERT(m_velocity_module);
    m_light_source_module = parent_object()->get_module_of_type<Light_Source_Module>();
    L_ASSERT(m_light_source_module);
}



const glm::vec3& Player_Control_Module::M_get_start_weapon_position() const
{
    if(m_weapon->is_idling())
        return m_weapon->weapon_visual_idle_position();

    const Weapon::Start_End_Weapon_Movement& movement = m_weapon->weapon_visual_attack_movement( m_weapon->previous_attack() );
    return movement.end;
}

const glm::vec3& Player_Control_Module::M_get_start_weapon_rotation() const
{
    if(m_weapon->is_idling())
        return m_weapon->weapon_visual_idle_rotation();

    const Weapon::Start_End_Weapon_Movement& rotation = m_weapon->weapon_visual_attack_rotation( m_weapon->previous_attack() );
    return rotation.end;
}


void Player_Control_Module::M_prepare_attack_movement()
{
    unsigned int attack_id = m_weapon->next_attack();

    float total_attack_duration = m_weapon->attack_duration(attack_id);

    const Weapon::Start_End_Weapon_Movement& movement_trajectory = m_weapon->weapon_visual_attack_movement(attack_id);

    m_weapon_position_kfa.clear();

    const glm::vec3& starting_position = M_get_start_weapon_position();

    if( LEti::Math::vecs_are_equal(starting_position, movement_trajectory.start) )
    {
        m_weapon_position_kfa.add_frame(0.0f, movement_trajectory.start);
        m_weapon_position_kfa.add_frame(total_attack_duration, movement_trajectory.end);
    }
    else
    {
        m_weapon_position_kfa.add_frame(0.0f, starting_position);
        m_weapon_position_kfa.add_frame(Attack_Prepare_Duration, movement_trajectory.start);
        m_weapon_position_kfa.add_frame(total_attack_duration, movement_trajectory.end);
    }

    m_weapon_position_kfa.start();
}

void Player_Control_Module::M_prepare_attack_rotation()
{
    unsigned int attack_id = m_weapon->next_attack();

    float total_attack_duration = m_weapon->attack_duration(attack_id);

    const Weapon::Start_End_Weapon_Movement& rotation_trajectory = m_weapon->weapon_visual_attack_rotation(attack_id);

    m_weapon_rotation_kfa.clear();

    const glm::vec3& starting_rotation = M_get_start_weapon_rotation();

    if( LEti::Math::vecs_are_equal(starting_rotation, rotation_trajectory.start) )
    {
        m_weapon_rotation_kfa.add_frame(0.0f, rotation_trajectory.start);
        m_weapon_rotation_kfa.add_frame(total_attack_duration, rotation_trajectory.end);
    }
    else
    {
        m_weapon_rotation_kfa.add_frame(0.0f, starting_rotation);
        m_weapon_rotation_kfa.add_frame(Attack_Prepare_Duration, rotation_trajectory.start);
        m_weapon_rotation_kfa.add_frame(total_attack_duration, rotation_trajectory.end);
    }

    m_weapon_rotation_kfa.start();
}

void Player_Control_Module::M_on_weapon_started_idling()
{
    glm::vec3 last_position = m_weapon_position_kfa.current_value();
    glm::vec3 last_rotation = m_weapon_rotation_kfa.current_value();

    m_weapon_position_kfa.clear();
    m_weapon_position_kfa.add_frame(0.0f, last_position);
    m_weapon_position_kfa.add_frame(Attack_Prepare_Duration, m_weapon->weapon_visual_idle_position());
    m_weapon_position_kfa.start();

    m_weapon_rotation_kfa.clear();
    m_weapon_rotation_kfa.add_frame(0.0f, last_rotation);
    m_weapon_rotation_kfa.add_frame(Attack_Prepare_Duration, m_weapon->weapon_visual_idle_rotation());
    m_weapon_rotation_kfa.start();
}



void Player_Control_Module::M_process_view(float _dt)
{
    constexpr float View_Rotation_Speed = LEti::Math::PI;
    constexpr float Cursor_Movement_Threshold_For_View = 1920.0f;   //  this shouldn't be hard-coded

    glm::vec2 cursor_stride = LR::Window_Controller::instance().get_cursor_stride();

    float horizontal_ratio = -cursor_stride.x / Cursor_Movement_Threshold_For_View;
    float horizontal_rotation = horizontal_ratio * View_Rotation_Speed;

    transformation_data()->rotate({0.0f, horizontal_rotation, 0.0f});

    float vertical_ratio = -cursor_stride.y / Cursor_Movement_Threshold_For_View;
    float vertical_rotation = vertical_ratio * View_Rotation_Speed;

    constexpr float Max_Vertical_Rotation = LEti::Math::HALF_PI * 0.95f;

    m_camera_vertical_angle += vertical_rotation;
    if(m_camera_vertical_angle < -Max_Vertical_Rotation)
        m_camera_vertical_angle = -Max_Vertical_Rotation;
    if(m_camera_vertical_angle > Max_Vertical_Rotation)
        m_camera_vertical_angle = Max_Vertical_Rotation;
}

void Player_Control_Module::M_process_movement(float _dt)
{
    const glm::vec3& look_direction = m_camera->settings().direction;

    glm::vec3 look_direction_flattened = look_direction;
    look_direction_flattened.y = 0.0f;
    LEti::Math::shrink_vector_to_1(look_direction_flattened);

    constexpr glm::vec3 top_direction = {0.0f, 1.0f, 0.0f};

    if(m_velocity_module->is_grounded() && LR::Window_Controller::instance().is_key_down(GLFW_KEY_SPACE))
    {
        glm::vec3 impulse = m_velocity_module->impulse();
        impulse.y = m_jump_impulse_strength;
        m_velocity_module->set_impulse(impulse);
        m_velocity_module->mark_grounded(false);
    }

    glm::vec3 direction = { 0.0f, 0.0f, 0.0f };

    if(LR::Window_Controller::instance().is_key_down(GLFW_KEY_W))
        direction += look_direction_flattened;
    if(LR::Window_Controller::instance().is_key_down(GLFW_KEY_S))
        direction += -look_direction_flattened;
    if(LR::Window_Controller::instance().is_key_down(GLFW_KEY_A))
        direction += LEti::Math::rotate_vector(look_direction_flattened, top_direction, LEti::Math::HALF_PI);
    if(LR::Window_Controller::instance().is_key_down(GLFW_KEY_D))
        direction += LEti::Math::rotate_vector(look_direction_flattened, top_direction, -LEti::Math::HALF_PI);

    LEti::Math::shrink_vector_to_1(direction);

    bool moving = LEti::Math::vector_length_squared(direction) > 1e-6f;

    if(!m_dash_cooldown_timer.is_active() && LR::Window_Controller::instance().key_was_pressed(GLFW_KEY_LEFT_SHIFT))
    {
        const glm::vec3& dash_direction = moving ? direction : look_direction_flattened;
        m_velocity_module->set_impulse(dash_direction * m_dash_impulse);
        m_dash_cooldown_timer.start(m_dash_cooldown);
        m_dash_duration_timer.start(m_dash_duration);

        Message__Player_Evades msg;
        LST::Message_Translator::instance().translate(msg);

        return;
    }

    if(!moving)
        return;

    float movement_speed_multiplier = 1.0f;
    if(!m_velocity_module->is_grounded())
        movement_speed_multiplier = m_mid_air_movement_speed_multiplier;

    const glm::vec3& impulse = m_velocity_module->impulse();

    glm::vec3 new_impulse = calculate_impulse(impulse, direction * m_movement_speed * _dt * movement_speed_multiplier, m_movement_speed, 10.0f);
    m_velocity_module->set_impulse(new_impulse);
}

void Player_Control_Module::M_process_attack(float _dt)
{
    if(!LR::Window_Controller::instance().is_mouse_button_down(GLFW_MOUSE_BUTTON_LEFT))
        return;

    if(!m_weapon)
        return;

    if(!m_weapon->can_attack())
        return;

    M_prepare_attack_movement();
    M_prepare_attack_rotation();

    m_weapon->attack(parent_object());
}

void Player_Control_Module::M_process_thrust(float _dt)
{
    LR::Window_Controller& wc = LR::Window_Controller::instance();

    bool should_reset = false;
    if(wc.is_key_down(GLFW_KEY_W))
        should_reset = true;
    else if(wc.is_key_down(GLFW_KEY_A))
        should_reset = true;
    else if(wc.is_key_down(GLFW_KEY_S))
        should_reset = true;
    else if(wc.is_key_down(GLFW_KEY_D))
        should_reset = true;
    else if(wc.is_key_down(GLFW_KEY_SPACE))
        should_reset = true;
    else if(wc.is_key_down(GLFW_KEY_LEFT_SHIFT))
        should_reset = true;
    else if(wc.mouse_button_was_pressed(GLFW_MOUSE_BUTTON_LEFT))
        should_reset = true;
    else if( LEti::Math::vector_length_squared(m_velocity_module->impulse()) > 0.05f )
        should_reset = true;

    if(should_reset)
    {
        m_thrust_prepare_timer.reset();
        return;
    }

    if(wc.key_was_pressed(GLFW_KEY_F) && !m_thrust_prepare_timer.is_active())
        m_thrust_prepare_timer.start(m_thrust_prepare_duration);

    bool preparing_before = m_thrust_prepare_timer.is_active();
    m_thrust_prepare_timer.update(_dt);
    bool preparing_after = m_thrust_prepare_timer.is_active();

    if(!(preparing_before && !preparing_after))
        return;

    const glm::vec3& direction = m_camera->settings().direction;
    m_velocity_module->set_impulse(direction * m_thrust_impulse);
}


void Player_Control_Module::M_update_traction(float _dt)
{
    m_dash_cooldown_timer.update(_dt);

    bool dash_active_before = m_dash_duration_timer.is_active();
    m_dash_duration_timer.update(_dt);
    bool dash_active_after = m_dash_duration_timer.is_active();

    if(m_dash_duration_timer.is_active())
        m_velocity_module->set_traction_multiplier(0.0f);
    else
        m_velocity_module->set_traction_multiplier(1.0f);

    if(dash_active_before && !dash_active_after)
    {
        glm::vec3 velocity = m_velocity_module->impulse();
        if(LEti::Math::vector_length_squared(velocity) > m_movement_speed_squared)
        {
            LEti::Math::extend_vector_to_length(velocity, m_movement_speed);
            m_velocity_module->set_impulse(velocity);
        }
    }
}


void Player_Control_Module::M_update_first_person_animation_trasformation()
{
    glm::vec3 local_rotation;
    glm::vec3 local_movement;

    if(m_weapon->is_idling() && !m_weapon_position_kfa.active())
    {
        local_movement = m_weapon->weapon_visual_idle_position();
        local_rotation = m_weapon->weapon_visual_idle_rotation();
    }
    else
    {
        local_movement = m_weapon_position_kfa.current_value();
        local_rotation = m_weapon_rotation_kfa.current_value();
    }

    glm::vec3 global_rotation = -LEti::Math::calculate_angles(m_camera->settings().direction, m_camera->settings().top);
    global_rotation.y += LEti::Math::PI;

    glm::mat4x4 global_rotation_matrix = LEti::Math::calculate_rotation_matrix(global_rotation);
    glm::mat4x4 local_rotation_matrix = LEti::Math::calculate_rotation_matrix(local_rotation);

    glm::mat4x4 result_rotation = global_rotation_matrix * local_rotation_matrix;

    glm::vec3 rotation = LEti::Math::calculate_angles(result_rotation);

    m_first_person_dm_transformation.set_rotation(rotation);

    glm::vec3 position = m_first_person_dm_transformation.rotation_matrix() * glm::vec4(local_movement, 1.0f);
    position += m_camera->settings().position;

    m_first_person_dm_transformation.set_position(position);
}

void Player_Control_Module::M_update_first_person_animation(float _dt)
{
    L_ASSERT(m_first_person_dm);

    m_first_person_dm->update_prev_state();
    m_first_person_dm->update(_dt);
}


void Player_Control_Module::M_reconfigure_camera()
{
    constexpr glm::vec3 default_view_top = {0.0f, 1.0f, 0.0f};
    constexpr glm::vec3 default_view_direction = {0.0f, 0.0f, 1.0f};

    float horizontal_rotation = transformation_data()->rotation().y;

    LR::Camera_3D::Settings& camera_settings = m_camera->settings();
    camera_settings.direction = LEti::Math::rotate_vector(default_view_direction, default_view_top, horizontal_rotation);
    camera_settings.top = default_view_top;

    glm::vec3 flattened_direction = camera_settings.direction;
    flattened_direction.y = 0.0f;
    LEti::Math::shrink_vector_to_1(flattened_direction);

    glm::vec3 perpendicular = LEti::Math::cross_product(camera_settings.direction, camera_settings.top);
    LEti::Math::shrink_vector_to_1(perpendicular);

    camera_settings.direction = LEti::Math::rotate_vector(flattened_direction, perpendicular, m_camera_vertical_angle);
    camera_settings.top = LEti::Math::rotate_vector(camera_settings.direction, perpendicular, LEti::Math::QUARTER_PI);

    m_camera->settings().position = transformation_data()->position();
    m_camera->settings().position.y += m_camera_vertical_offset;

    if(m_thrust_prepare_timer.is_active())
        m_camera->settings().position.y -= (1.0f - (m_thrust_prepare_timer.time_left() / m_thrust_prepare_duration)) * m_max_thrust_camera_decrement;

    m_camera->reconfigure();
}



void Player_Control_Module::update(float _dt)
{
    L_ASSERT(m_camera);

    m_weapon_position_kfa.update(_dt);
    m_weapon_rotation_kfa.update(_dt);

    if(m_weapon)
    {
        m_weapon->update(_dt);

        if(m_weapon->just_started_idling())
            M_on_weapon_started_idling();
    }

    M_process_view(_dt);
    M_process_movement(_dt);
    M_process_attack(_dt);
    M_process_thrust(_dt);

    M_update_traction(_dt);

    M_update_first_person_animation(_dt);
}





Player_Control_Module_Stub::~Player_Control_Module_Stub()
{

}



BUILDER_STUB_DEFAULT_CONSTRUCTION_FUNC(Player_Control_Module_Stub)

BUILDER_STUB_INITIALIZATION_FUNC(Player_Control_Module_Stub)
{
    BUILDER_STUB_PARENT_INITIALIZATION;
    BUILDER_STUB_CAST_PRODUCT;

    L_ASSERT(camera);
    L_ASSERT(objects_controller);
    L_ASSERT(weapons_provider);

    product->inject_camera(camera);
    product->inject_objects_controller(objects_controller);

    product->set_movement_speed(movement_speed);
    product->set_jump_impulse_strength(jump_impulse_strength);
    product->set_mid_air_movement_speed_multiplier(mid_air_movement_speed_multiplier);
    product->set_dash_cooldown(dash_cooldown);
    product->set_dash_duration(dash_duration);
    product->set_dash_impulse(dash_impulse);
    product->set_camera_vertical_offset(camera_vertical_offset);
    product->set_thrust_prepare_duration(thrust_prepare_duration);
    product->set_thrust_impulse(thrust_impulse);
    product->set_max_thrust_camera_decrement(max_thrust_camera_decrement);

    Weapon_Stub* weapon_stub = weapons_provider->get_object<Weapon_Stub>(weapon_name);
    L_ASSERT(weapon_stub);
    product->set_weapon( Weapon_Stub::construct_from(weapon_stub) );
}
