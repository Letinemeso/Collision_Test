#include <Controllers/Objects_Controller_Extensions/Objects_Controller_Extension__Gravity_Applier.h>

#include <Math_Stuff.h>

#include <Physics/Rigid_Body/Physics_Module__Rigid_Body.h>

using namespace Shardis;


void Objects_Controller_Extension__Gravity_Applier::update(Objects_List& _objects, float _dt)
{
    for(Objects_List::Iterator it = _objects.begin(); !it.end_reached(); ++it)
    {
        LEti::Object* object = *it;

        LMD::Physics_Module__Rigid_Body* rb = object->get_module_of_type<LMD::Physics_Module__Rigid_Body>();
        if(!rb)
            continue;

        rb->apply_linear_impulse({0.0f, -9.8f * _dt, 0.0f});
    }
}
