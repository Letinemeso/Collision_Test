#include <Weapons/Generic_Weapon.h>

#include <Modules/Attack_Module.h>

using namespace Shardis;

namespace Shardis
{
    constexpr float Inactivity_Time = 0.5f;
}



unsigned int Generic_Weapon::next_attack() const
{
    return m_next_attack;
}

unsigned int Generic_Weapon::previous_attack() const
{
    if(m_next_attack == 0)
        return m_attacks.size() - 1;

    return m_next_attack - 1;
}

bool Generic_Weapon::can_attack() const
{
    return !m_delay_timer.is_active();
}

bool Generic_Weapon::is_idling() const
{
    return !m_delay_timer.is_active() && !m_inactivity_timer.is_active() && (m_next_attack == 0);
}

bool Generic_Weapon::just_started_idling() const
{
    return m_just_started_idling;
}



void Generic_Weapon::update(float _dt)
{
    m_just_started_idling = false;

    bool inactive_before = m_inactivity_timer.is_active();
    m_inactivity_timer.update(_dt);
    bool inactive_after = m_inactivity_timer.is_active();

    bool delay_before = m_delay_timer.is_active();
    m_delay_timer.update(_dt);
    bool delay_after = m_delay_timer.is_active();

    if(delay_before && !delay_after)
        m_inactivity_timer.start(Inactivity_Time);

    if(inactive_before && !inactive_after)
    {
        m_next_attack = 0;
        m_just_started_idling = true;
    }
}

void Generic_Weapon::attack(LEti::Object* _by)
{
    if(m_delay_timer.is_active())
        return;

    const LEti::Object_Stub* attack_stub = m_attacks[m_next_attack];

    LEti::Object* attack_object = LEti::Object_Stub::construct_from(attack_stub);
    Attack_Module* attack_module = attack_object->get_module_of_type<Attack_Module>();
    L_ASSERT(attack_module);

    LR::Camera_3D* camera = m_main_game_camera;

    attack_module->set_owner(_by);
    attack_module->set_calculate_offset_func([camera]()
    {
        return camera->settings().position;
    });
    attack_module->set_calculate_rotation_func([camera]()
    {
        glm::vec3 rotation = -LEti::Math::calculate_angles(camera->settings().direction, camera->settings().top);
        rotation.y += LEti::Math::PI;

        return rotation;
    });

    attack_object->current_state().set_position(_by->current_state().position());
    attack_object->update(0.0f);
    attack_object->update_previous_state();

    L_ASSERT(m_objects_controller);
    m_objects_controller->add_object(attack_object);

    m_inactivity_timer.reset();
    m_delay_timer.start( attack_duration(m_next_attack) );

    ++m_next_attack;
    if(m_next_attack >= m_attacks.size())
        m_next_attack = 0;
}





BUILDER_STUB_DEFAULT_CONSTRUCTION_FUNC(Generic_Weapon_Stub)

BUILDER_STUB_INITIALIZATION_FUNC(Generic_Weapon_Stub)
{
    BUILDER_STUB_PARENT_INITIALIZATION;
    BUILDER_STUB_CAST_PRODUCT;

    L_ASSERT(attacks.size() == attacks_durations.size());

    Generic_Weapon::Attacks_Vec attacks_vec(attacks.size());

    for(LV::Variable_Base::Childs_List::Const_Iterator it = attacks.begin(); !it.end_reached(); ++it)
    {
        LEti::Object_Stub* attack_stub = (LEti::Object_Stub*)it->child_ptr;
        L_ASSERT(LV::cast_variable<LEti::Object_Stub>(attack_stub));

        attacks_vec.push(attack_stub);
    }

    product->set_attacks((Generic_Weapon::Attacks_Vec&&)attacks_vec);
}



Generic_Weapon_Stub::~Generic_Weapon_Stub()
{
    clear_childs_list(attacks);
}
