#pragma once

#include <Stuff/Key_Frame_Animation.h>

#include <Module.h>

#include <Camera/Camera_3D.h>
#include <Draw_Modules/Draw_Module.h>

#include <Tools/Objects_Storage.h>
#include <Tools/Objects_Controller/Objects_Controller.h>

#include <Modules/Velocity_Module.h>
#include <Modules/Light_Source_Module.h>
#include <Messages/Messages.h>
#include <Weapons/Weapon.h>


namespace Shardis
{

    class Player_Control_Module : public LEti::Module
    {
    public:
        INIT_VARIABLE(Shardis::Player_Control_Module, LEti::Module)

    private:
        float m_movement_speed = 0.0f;
        float m_movement_speed_squared = 0.0f;
        float m_jump_impulse_strength = 0.0f;
        float m_mid_air_movement_speed_multiplier = 0.0f;
        float m_dash_cooldown = 0.0f;
        float m_dash_duration = 0.0f;
        float m_dash_impulse = 0.0f;
        float m_thrust_prepare_duration = 0.0f;
        float m_thrust_impulse = 0.0f;
        float m_max_thrust_camera_decrement = 0.0f;

        float m_camera_vertical_offset = 0.0f;

    private:
        LR::Camera_3D* m_camera = nullptr;
        LMD::Objects_Controller* m_objects_controller = nullptr;

        Weapon* m_weapon = nullptr;
        LR::Draw_Module* m_first_person_dm = nullptr;
        LEti::Transformation_Data m_first_person_dm_transformation;
        LST::Key_Frame_Animation<glm::vec3> m_weapon_position_kfa;
        LST::Key_Frame_Animation<glm::vec3> m_weapon_rotation_kfa;

        Velocity_Module* m_velocity_module = nullptr;
        Light_Source_Module* m_light_source_module = nullptr;

        LST::Message_Translator::Handle<Message__Update_Camera> m_message_handle__update_camera;
        LST::Message_Translator::Handle<Message__On_Entity_Damaged> m_message_handle__damaged;

        LST::Timer m_dash_cooldown_timer;
        LST::Timer m_dash_duration_timer;

        LST::Timer m_thrust_prepare_timer;

    private:
        float m_camera_vertical_angle = 0.0f;

    public:
        Player_Control_Module();
        ~Player_Control_Module();

    public:
        inline void inject_camera(LR::Camera_3D* _ptr) { m_camera = _ptr; }
        inline void inject_objects_controller(LMD::Objects_Controller* _ptr) { m_objects_controller = _ptr; }

        inline void set_movement_speed(float _value) { m_movement_speed = _value; m_movement_speed_squared = _value * _value; }
        inline void set_jump_impulse_strength(float _value) { m_jump_impulse_strength = _value; }
        inline void set_mid_air_movement_speed_multiplier(float _value) { m_mid_air_movement_speed_multiplier = _value; }
        inline void set_dash_cooldown(float _value) { m_dash_cooldown = _value; }
        inline void set_dash_duration(float _value) { m_dash_duration = _value; }
        inline void set_dash_impulse(float _value) { m_dash_impulse = _value; }
        inline void set_thrust_prepare_duration(float _value) { m_thrust_prepare_duration = _value; }
        inline void set_thrust_impulse(float _value) { m_thrust_impulse = _value; }
        inline void set_max_thrust_camera_decrement(float _value) { m_max_thrust_camera_decrement = _value; }
        inline void set_camera_vertical_offset(float _value) { m_camera_vertical_offset = _value; }

        void set_weapon(Weapon* _ptr);

    private:
        void M_on_parent_object_set() override;

    private:
        const glm::vec3& M_get_start_weapon_position() const;
        const glm::vec3& M_get_start_weapon_rotation() const;

        void M_prepare_attack_movement();
        void M_prepare_attack_rotation();
        void M_on_weapon_started_idling();

    private:
        void M_process_view(float _dt);
        void M_process_movement(float _dt);
        void M_process_attack(float _dt);
        void M_process_thrust(float _dt);

        void M_update_traction(float _dt);

        void M_update_first_person_animation_trasformation();
        void M_update_first_person_animation(float _dt);

        void M_reconfigure_camera();

    public:
        void update(float _dt) override;

    };


    class Player_Control_Module_Stub : public LEti::Module_Stub
    {
    public:
        INIT_VARIABLE(Shardis::Player_Control_Module_Stub, LEti::Module_Stub)

        INIT_FIELDS
        ADD_FIELD(float, movement_speed)
        ADD_FIELD(float, jump_impulse_strength)
        ADD_FIELD(float, mid_air_movement_speed_multiplier)
        ADD_FIELD(float, dash_cooldown)
        ADD_FIELD(float, dash_duration)
        ADD_FIELD(float, dash_impulse)
        ADD_FIELD(float, thrust_prepare_duration)
        ADD_FIELD(float, thrust_impulse)
        ADD_FIELD(float, max_thrust_camera_decrement)
        ADD_FIELD(float, camera_vertical_offset)
        ADD_FIELD(std::string, weapon_name)
        FIELDS_END

    public:
        LR::Camera_3D* camera = nullptr;
        LMD::Objects_Controller* objects_controller = nullptr;
        LMD::Objects_Storage* weapons_provider = nullptr;

    public:
        float movement_speed = 0.0f;
        float jump_impulse_strength = 0.0f;
        float mid_air_movement_speed_multiplier = 0.0f;
        float dash_cooldown = 0.0f;
        float dash_duration = 0.0f;
        float dash_impulse = 0.0f;
        float thrust_prepare_duration = 0.0f;
        float thrust_impulse = 0.0f;
        float max_thrust_camera_decrement = 0.0f;

        float camera_vertical_offset = 0.0f;

        std::string weapon_name;

    public:
        ~Player_Control_Module_Stub();

    public:
        INIT_BUILDER_STUB(Player_Control_Module)

    };

}
