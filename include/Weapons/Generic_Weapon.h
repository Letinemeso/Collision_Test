#pragma once

#include <Stuff/Timer.h>

#include <Weapons/Weapon.h>


namespace Shardis
{

    class Generic_Weapon : public Weapon
    {
    public:
        INIT_VARIABLE(Shardis::Generic_Weapon, Shardis::Weapon)

    public:
        using Attacks_Vec = LDS::Vector<const LEti::Object_Stub*>;

    private:
        Attacks_Vec m_attacks;

        LST::Timer m_inactivity_timer;
        LST::Timer m_delay_timer;
        unsigned int m_next_attack = 0;

        bool m_just_started_idling = false;

    public:
        inline void set_attacks(Attacks_Vec&& _attacks) { m_attacks = (Attacks_Vec&&)_attacks; }

    public:
        unsigned int next_attack() const override;
        unsigned int previous_attack() const override;
        bool can_attack() const override;
        bool is_idling() const override;
        bool just_started_idling() const override;

        void update(float _dt) override;
        void attack(LEti::Object* _by) override;

    };


    class Generic_Weapon_Stub : public Weapon_Stub
    {
    public:
        INIT_VARIABLE(Shardis::Generic_Weapon_Stub, Shardis::Weapon_Stub)

        INIT_CHILDS_LISTS
        ADD_CHILDS_LIST("Attack__*", &attacks)
        CHILDS_LISTS_END

    public:
        LV::Variable_Base::Childs_List attacks;

    public:
        INIT_BUILDER_STUB(Generic_Weapon)

    public:
        ~Generic_Weapon_Stub();

    };

}
