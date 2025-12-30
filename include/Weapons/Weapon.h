#pragma once

#include <Variable_Base.h>
#include <Builder_Stub.h>

#include <Object.h>

#include <Camera/Camera_3D.h>
#include <Draw_Modules/Draw_Module.h>

#include <Tools/Objects_Controller/Objects_Controller.h>


namespace Shardis
{

    class Weapon : public LV::Variable_Base
    {
    public:
        INIT_VARIABLE(Shardis::Weapon, LV::Variable_Base)

    public:
        struct Start_End_Weapon_Movement
        {
            glm::vec3 start;
            glm::vec3 end;
        };

    protected:
        LMD::Objects_Controller* m_objects_controller = nullptr;
        LR::Camera_3D* m_main_game_camera = nullptr;

    private:
        const LR::Draw_Module_Stub* m_first_person_animation_stub = nullptr;

        glm::vec3 m_weapon_visual_idle_position;
        glm::vec3 m_weapon_visual_idle_rotation;

        LDS::Vector<Start_End_Weapon_Movement> m_weapon_visual_attack_positions;
        LDS::Vector<Start_End_Weapon_Movement> m_weapon_visual_attack_rotations;

        LDS::Vector<float> m_attacks_durations;

    public:
        inline void inject_objects_controller(LMD::Objects_Controller* _ptr) { m_objects_controller = _ptr; }
        inline void inject_main_game_camera(LR::Camera_3D* _ptr) { m_main_game_camera = _ptr; }

        inline void set_first_person_animation_stub(const LR::Draw_Module_Stub* _ptr) { m_first_person_animation_stub = _ptr; }

        inline void set_weapon_visual_idle_position(const glm::vec3& _value) { m_weapon_visual_idle_position = _value; }
        inline void set_weapon_visual_idle_rotation(const glm::vec3& _value) { m_weapon_visual_idle_rotation = _value; }
        inline void set_weapon_visual_attack_positions(LDS::Vector<Start_End_Weapon_Movement>&& _values) { m_weapon_visual_attack_positions = (LDS::Vector<Start_End_Weapon_Movement>&&)_values; }
        inline void set_weapon_visual_attack_rotations(LDS::Vector<Start_End_Weapon_Movement>&& _values) { m_weapon_visual_attack_rotations = (LDS::Vector<Start_End_Weapon_Movement>&&)_values; }

        inline void set_attacks_durations(const LDS::Vector<float>& _values) { m_attacks_durations = _values; }

    public:
        inline const LR::Draw_Module_Stub* first_person_animation_stub() const { return m_first_person_animation_stub; }

        inline const glm::vec3& weapon_visual_idle_position() const { return m_weapon_visual_idle_position; }
        inline const glm::vec3& weapon_visual_idle_rotation() const { return m_weapon_visual_idle_rotation; }
        inline const Start_End_Weapon_Movement& weapon_visual_attack_movement(unsigned int _attack_frame) const
            { L_ASSERT(_attack_frame < m_weapon_visual_attack_positions.size()); return m_weapon_visual_attack_positions[_attack_frame]; }
        inline const Start_End_Weapon_Movement& weapon_visual_attack_rotation(unsigned int _attack_frame) const
            { L_ASSERT(_attack_frame < m_weapon_visual_attack_rotations.size()); return m_weapon_visual_attack_rotations[_attack_frame]; }

        inline float attack_duration(unsigned int _attack_frame) const { return m_attacks_durations[_attack_frame]; }

    public:
        virtual unsigned int next_attack() const = 0;
        virtual unsigned int previous_attack() const = 0;
        virtual bool can_attack() const = 0;
        virtual bool is_idling() const = 0;
        virtual bool just_started_idling() const = 0;

        virtual void update(float _dt) = 0;
        virtual void attack(LEti::Object* _by) = 0;

    };


    class Weapon_Stub : public LV::Builder_Stub
    {
    public:
        INIT_VARIABLE(Shardis::Weapon_Stub, LV::Builder_Stub)

        INIT_FIELDS
        ADD_FIELD(glm::vec3, idle_weapon_position)
        ADD_FIELD(glm::vec3, idle_weapon_rotation)
        ADD_FIELD(LDS::Vector<glm::vec3>, start_end_attack_positions)
        ADD_FIELD(LDS::Vector<glm::vec3>, start_end_attack_rotations)
        ADD_FIELD(LDS::Vector<float>, attacks_durations)
        FIELDS_END

        INIT_CHILDS
        ADD_CHILD("first_person_animation", first_person_animation)
        CHILDS_END

    public:
        LMD::Objects_Controller* objects_controller = nullptr;
        LR::Camera_3D* main_game_camera = nullptr;

    public:
        LR::Draw_Module_Stub* first_person_animation = nullptr;

    public:
        glm::vec3 idle_weapon_position;
        glm::vec3 idle_weapon_rotation;

        LDS::Vector<glm::vec3> start_end_attack_positions;
        LDS::Vector<glm::vec3> start_end_attack_rotations;

        LDS::Vector<float> attacks_durations;

    public:
        INIT_BUILDER_STUB(Weapon)

    public:
        ~Weapon_Stub();

    };

}
