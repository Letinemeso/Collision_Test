#include <Modules/Health_Module.h>

#include <Stuff/Message_Translator.h>

#include <Tools/Objects_Controller/Objects_Controller_Extension__Removal.h>

#include <Messages/Messages.h>

using namespace Shardis;


Health_Module::Health_Module()
{
    m_message_handle = LST::Message_Translator::instance().subscribe<Message__On_Entity_Death>([this](Message__On_Entity_Death& _msg)
    {
        Invulnerability_Timers::Iterator maybe_it = m_invulnerabilities.find(_msg.almost_dead_entity);
        if(!maybe_it.is_ok())
            return;

        m_invulnerabilities.erase(maybe_it);
    });
}

Health_Module::~Health_Module()
{
    LST::Message_Translator::instance().unsubscribe(m_message_handle);
}



void Health_Module::M_send_on_damaged_message(float _damage, float _stagger, LEti::Object* _from, const glm::vec3& _damaged_point) const
{
    Message__On_Entity_Damaged damaged_msg;
    damaged_msg.damaging_entity = _from;
    damaged_msg.damaged_entity = parent_object();
    damaged_msg.damage = _damage;
    damaged_msg.stagger = _stagger;
    damaged_msg.health_before = m_health;
    damaged_msg.health_after = m_health - _damage;
    if(damaged_msg.health_after < 0.0f)
        damaged_msg.health_after = 0.0f;
    damaged_msg.damaged_point = _damaged_point;
    LST::Message_Translator::instance().translate(damaged_msg);
}

void Health_Module::M_receive_damage(float _damage)
{
    m_health -= _damage;

    if(m_health > 0.0f)
        return;

    m_health = 0.0f;

    LMD::Message__Request_Object_Deletion request_death_msg;
    request_death_msg.object = parent_object();

    LST::Message_Translator::instance().translate(request_death_msg);
}



void Health_Module::receive_damage(float _damage, float _stagger, LEti::Object* _from)
{
    M_send_on_damaged_message(_damage, _stagger, _from, transformation_data()->position());
    M_receive_damage(_damage);
}

void Health_Module::receive_damage(float _damage, float _stagger, LEti::Object* _from, const glm::vec3& _damaged_point)
{
    M_send_on_damaged_message(_damage, _stagger, _from, _damaged_point);
    M_receive_damage(_damage);
}

void Health_Module::add_invulnerability(float _duration, LEti::Object* _to)
{
    L_ASSERT(_to);

    if(_duration < 1e-6f)
        return;

    Invulnerability_Timers::Iterator maybe_it = m_invulnerabilities.find(_to);
    if(!maybe_it.is_ok())
    {
        maybe_it = m_invulnerabilities.insert_and_get_iterator(_to, LST::Timer(_duration));
        return;
    }

    LST::Timer& timer = *maybe_it;

    if(timer.time_left() > _duration)
        return;

    timer.start(_duration);
}



bool Health_Module::is_invulnerable_to(LEti::Object* _to) const
{
    Invulnerability_Timers::Const_Iterator maybe_it = m_invulnerabilities.find(_to);
    return maybe_it.is_ok();
}



void Health_Module::M_update_invulnerabilities(float _dt)
{
    Invulnerability_Timers::Iterator it = m_invulnerabilities.iterator();
    while(!it.end_reached())
    {
        LST::Timer& timer = *it;

        timer.update(_dt);

        if(timer.is_active())
            ++it;
        else
            it = m_invulnerabilities.erase_and_iterate_forward(it);
    }
}



void Health_Module::update(float _dt)
{
    M_update_invulnerabilities(_dt);
}





BUILDER_STUB_DEFAULT_CONSTRUCTION_FUNC(Health_Module_Stub)

BUILDER_STUB_INITIALIZATION_FUNC(Health_Module_Stub)
{
    BUILDER_STUB_PARENT_INITIALIZATION;
    BUILDER_STUB_CAST_PRODUCT;

    product->set_base_health(base_health);
    product->set_health(health);
}
