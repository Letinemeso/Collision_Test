#include <Controllers/Objects_Controller_Extensions/Objects_Controller_Extension__Physics.h>

using namespace Shardis;


void Objects_Controller_Extension__Physics::on_object_added(LEti::Object* _object)
{
    _object->process_logic_for_modules_of_type<LPhys::Physics_Module>([this](LPhys::Physics_Module* _module)
    {
        if(!m_registration_filter || m_registration_filter(_module))
            m_collision_detector.register_module(_module);
    });
}

void Objects_Controller_Extension__Physics::on_object_removed(LEti::Object* _object)
{
    _object->process_logic_for_modules_of_type<LPhys::Physics_Module>([this](LPhys::Physics_Module* _module)
    {
        if(!m_registration_filter || m_registration_filter(_module))
            m_collision_detector.unregister_module(_module);
    });
}


void Objects_Controller_Extension__Physics::update(Objects_List& _objects, float _dt)
{
    m_collision_detector.update();

    if(m_collision_detector.found_collisions().size() > 0)
    {
        const LPhys::Intersection_Data& id = *m_collision_detector.found_collisions().begin();
        on_collision(id);
    }

    m_collision_resolver.resolve_all(m_collision_detector.found_collisions(), _dt);
}
