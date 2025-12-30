#include <Modules/Attack_Module.h>

#include <gtc/quaternion.hpp>

#include <Tools/Objects_Controller/Objects_Controller.h>
#include <Tools/Objects_Controller/Objects_Controller_Extension__Removal.h>

#include <Tools/Entity_Tools.h>

using namespace Shardis;


Attack_Module::Attack_Module()
{
    m_owner_death_message_handle = LST::Message_Translator::instance().subscribe<Message__On_Entity_Death>([this](Message__On_Entity_Death& _msg)
    {
        if(m_owner != _msg.almost_dead_entity)
            return;

        m_owner = nullptr;

        LMD::Message__Request_Object_Deletion msg;
        msg.object = parent_object();

        LST::Message_Translator::instance().translate(msg);
    });
}

Attack_Module::~Attack_Module()
{
    LST::Message_Translator::instance().unsubscribe(m_owner_death_message_handle);
}



void Attack_Module::init_offset_trajectory(const LDS::Vector<glm::vec3>& _offsets, float _attack_speed)
{
    L_ASSERT(_attack_speed > 0.0001f);
    L_ASSERT(_offsets.size() > 0);

    m_offsets_kfa.clear();

    float time_per_frame = _attack_speed / (float)_offsets.size();

    for(unsigned int i = 0; i < _offsets.size(); ++i)
    {
        float time = time_per_frame * (float)i;
        m_offsets_kfa.add_frame(time, _offsets[i]);
    }

    m_offsets_kfa.set_run_in_loop(false);
    m_offsets_kfa.start();
}

void Attack_Module::init_rotation_trajectory(const LDS::Vector<glm::vec3>& _rotations, float _attack_speed)
{
    L_ASSERT(_attack_speed > 0.0001f);
    L_ASSERT(_rotations.size() > 0);

    m_rotation_kfa.clear();

    float time_per_frame = _attack_speed / (float)_rotations.size();

    for(unsigned int i = 0; i < _rotations.size(); ++i)
    {
        float time = time_per_frame * (float)i;
        m_rotation_kfa.add_frame(time, _rotations[i]);
    }

    m_rotation_kfa.set_run_in_loop(false);
    m_rotation_kfa.start();
}



void Attack_Module::M_apply_kfas()
{
    L_ASSERT(m_calculate_offset);
    L_ASSERT(m_calculate_rotation);

    glm::vec3 base_rotation_angles = m_calculate_rotation();
    glm::vec3 additional_offset = m_rotation_kfa.current_value();

    glm::quat base_rotation_quat = base_rotation_angles;
    glm::quat additional_offset_quat = additional_offset;

    glm::quat rotation_quat = base_rotation_quat * additional_offset_quat;
    glm::vec3 rotation = glm::eulerAngles(rotation_quat);

    transformation_data()->set_rotation( rotation );

    glm::vec3 offset = m_offsets_kfa.current_value();
    offset = transformation_data()->rotation_matrix() * glm::vec4(offset, 1.0f);

    transformation_data()->set_position( m_calculate_offset() + offset );
}



void Attack_Module::update(float _dt)
{
    if(!m_owner)
        return;

    m_offsets_kfa.update(_dt);
    m_rotation_kfa.update(_dt);

    if(m_offsets_kfa.active() && m_rotation_kfa.active())
    {
        M_apply_kfas();
        return;
    }

    LMD::Message__Request_Object_Deletion msg;
    msg.object = parent_object();

    LST::Message_Translator::instance().translate(msg);
}





BUILDER_STUB_DEFAULT_CONSTRUCTION_FUNC(Attack_Module_Stub)

BUILDER_STUB_INITIALIZATION_FUNC(Attack_Module_Stub)
{
    BUILDER_STUB_PARENT_INITIALIZATION;
    BUILDER_STUB_CAST_PRODUCT;

    L_ASSERT(offsets.size() > 0);
    L_ASSERT(rotations.size() > 0);

    product->set_damage(damage);
    product->set_damage_frequency(damage_frequency);
    product->set_knockback(knockback);
    product->set_stagger(stagger);

    product->init_offset_trajectory(offsets, attack_speed);
    product->init_rotation_trajectory(rotations, attack_speed);
}
