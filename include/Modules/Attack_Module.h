#pragma once

#include <Data_Structures/Vector.h>
#include <Stuff/Key_Frame_Animation.h>
#include <Stuff/Message_Translator.h>

#include <Module.h>

#include <Camera/Camera_3D.h>

#include <Messages/Messages.h>


namespace Shardis
{

    class Attack_Module : public LEti::Module
    {
    public:
        INIT_VARIABLE(Shardis::Attack_Module, LEti::Module)

    private:
        float m_damage = 0.0f;
        float m_damage_frequency = 0.0f;
        float m_knockback = 0.0f;
        float m_stagger = 0.0f;

        LST::Key_Frame_Animation<glm::vec3> m_offsets_kfa;
        LST::Key_Frame_Animation<glm::vec3> m_rotation_kfa;

        LEti::Object* m_owner = nullptr;
        LST::Function<glm::vec3()> m_calculate_offset;
        LST::Function<glm::vec3()> m_calculate_rotation;

        LST::Message_Translator::Handle<Message__On_Entity_Death> m_owner_death_message_handle;

    public:
        Attack_Module();
        ~Attack_Module();

    public:
        inline void set_damage(float _value) { m_damage = _value; }
        inline void set_damage_frequency(float _value) { m_damage_frequency = _value; }
        inline void set_knockback(float _value) { m_knockback = _value; }
        inline void set_stagger(float _value) { m_stagger = _value; }

        inline void set_owner(LEti::Object* _ptr) { m_owner = _ptr; }
        inline void set_calculate_offset_func(const LST::Function<glm::vec3()>& _func) { m_calculate_offset = _func; }
        inline void set_calculate_rotation_func(const LST::Function<glm::vec3()>& _func) { m_calculate_rotation = _func; }

        void init_offset_trajectory(const LDS::Vector<glm::vec3>& _offsets, float _attack_speed);
        void init_rotation_trajectory(const LDS::Vector<glm::vec3>& _rotations, float _attack_speed);

    public:
        inline float damage() const { return m_damage; }
        inline float damage_frequency() const { return m_damage_frequency; }
        inline float knockback() const { return m_knockback; }
        inline float stagger() const { return m_stagger; }
        inline LEti::Object* owner() const { return m_owner; }

    private:
        void M_apply_kfas();

    public:
        void update(float _dt) override;

    };


    class Attack_Module_Stub : public LEti::Module_Stub
    {
    public:
        INIT_VARIABLE(Shardis::Attack_Module_Stub, LEti::Module_Stub)

        INIT_FIELDS
        ADD_FIELD(float, damage)
        ADD_FIELD(float, damage_frequency)
        ADD_FIELD(float, knockback)
        ADD_FIELD(float, stagger)
        ADD_FIELD(LDS::Vector<glm::vec3>, offsets)
        ADD_FIELD(LDS::Vector<glm::vec3>, rotations)
        ADD_FIELD(float, attack_speed)
        FIELDS_END

    public:
        float damage = 0.0f;
        float damage_frequency = 0.0f;
        float knockback = 0.0f;
        float stagger = 0.0f;

        LDS::Vector<glm::vec3> offsets;
        LDS::Vector<glm::vec3> rotations;

        float attack_speed = 0.0f;

    public:
        INIT_BUILDER_STUB(Attack_Module)

    };

}
