#include <Weapons/Weapon.h>

using namespace Shardis;


BUILDER_STUB_NULL_CONSTRUCTION_FUNC(Weapon_Stub)

BUILDER_STUB_INITIALIZATION_FUNC(Weapon_Stub)
{
    BUILDER_STUB_CAST_PRODUCT;

    L_ASSERT(objects_controller);
    L_ASSERT(main_game_camera);

    product->inject_objects_controller(objects_controller);
    product->inject_main_game_camera(main_game_camera);

    product->set_first_person_animation_stub(first_person_animation);

    product->set_weapon_visual_idle_position(idle_weapon_position);
    product->set_weapon_visual_idle_rotation(idle_weapon_rotation);

    L_ASSERT(start_end_attack_positions.size() % 2 == 0);
    L_ASSERT(start_end_attack_positions.size() == start_end_attack_rotations.size());

    unsigned int pairs_amount = start_end_attack_positions.size() / 2;

    if(pairs_amount == 0)
        return;

    LDS::Vector<Weapon::Start_End_Weapon_Movement> movements(pairs_amount);
    LDS::Vector<Weapon::Start_End_Weapon_Movement> rotations(pairs_amount);

    for(unsigned int i = 0; i < start_end_attack_positions.size(); i += 2)
    {
        movements.push({ start_end_attack_positions[i], start_end_attack_positions[i + 1] });
        rotations.push({ start_end_attack_rotations[i], start_end_attack_rotations[i + 1] });
    }

    product->set_weapon_visual_attack_positions( std::move(movements) );
    product->set_weapon_visual_attack_rotations( std::move(rotations) );

    L_ASSERT(attacks_durations.size() == pairs_amount);

    product->set_attacks_durations(attacks_durations);
}



Weapon_Stub::~Weapon_Stub()
{
    delete first_person_animation;
}
