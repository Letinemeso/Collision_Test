#pragma once

#include <Stuff/Timer.h>
#include <Stuff/Message_Translator.h>

#include <Module.h>

#include <Messages/Messages.h>


namespace Shardis
{

    class Health_Module : public LEti::Module
    {
    public:
        INIT_VARIABLE(Shardis::Health_Module, LEti::Module)

    private:
        float m_base_health = 0.0f;
        float m_health = 0.0f;

    private:
        using Invulnerability_Timers = LDS::Map<LEti::Object*, LST::Timer>;
        Invulnerability_Timers m_invulnerabilities;

        LST::Message_Translator::Handle<Message__On_Entity_Death> m_message_handle;

    public:
        inline void set_base_health(float _value) { m_base_health = _value; }
        inline void set_health(float _value) { m_health = _value; }

        inline float base_health() const { return m_base_health; }
        inline float health() const { return m_health; }

    public:
        Health_Module();
        ~Health_Module();

    private:
        void M_send_on_damaged_message(float _damage, float _stagger, LEti::Object* _from, const glm::vec3& _damaged_point) const;
        void M_receive_damage(float _damage);

    public:
        void receive_damage(float _damage, float _stagger, LEti::Object* _from);
        void receive_damage(float _damage, float _stagger, LEti::Object* _from, const glm::vec3& _damaged_point);
        void add_invulnerability(float _duration, LEti::Object* _to);

    public:
        bool is_invulnerable_to(LEti::Object* _to) const;

    private:
        void M_update_invulnerabilities(float _dt);

    public:
        void update(float _dt) override;

    };


    class Health_Module_Stub : public LEti::Module_Stub
    {
    public:
        INIT_VARIABLE(Shardis::Health_Module_Stub, LEti::Module_Stub)

        INIT_FIELDS
        ADD_FIELD(float, base_health)
        ADD_FIELD(float, health)
        FIELDS_END

    public:
        float base_health = 0.0f;
        float health = 0.0f;

    public:
        INIT_BUILDER_STUB(Health_Module)

    };

}
