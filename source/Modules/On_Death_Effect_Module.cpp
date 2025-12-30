#include <Modules/On_Death_Effect_Module.h>

using namespace Shardis;


On_Death_Effect_Module::On_Death_Effect_Module()
{
    m_message_handle = LST::Message_Translator::instance().subscribe<Message__On_Entity_Death>([this](Message__On_Entity_Death& _msg)
    {
        if(_msg.almost_dead_entity != parent_object())
            return;

        L_ASSERT(m_objects_controller);
        L_ASSERT(m_effect_stub);

        LEti::Object* effect = LEti::Object_Stub::construct_from(m_effect_stub);
        effect->current_state().set_position(transformation_data()->position() + m_offset);
        effect->update(0.0f);
        effect->update_previous_state();

        m_objects_controller->add_object(effect);
    });
}

On_Death_Effect_Module::~On_Death_Effect_Module()
{
    LST::Message_Translator::instance().unsubscribe(m_message_handle);
}





BUILDER_STUB_DEFAULT_CONSTRUCTION_FUNC(On_Death_Effect_Module_Stub)

BUILDER_STUB_INITIALIZATION_FUNC(On_Death_Effect_Module_Stub)
{
    BUILDER_STUB_PARENT_INITIALIZATION;
    BUILDER_STUB_CAST_PRODUCT;

    L_ASSERT(objects_controller);
    L_ASSERT(effect_stub);

    product->inject_objects_controller(objects_controller);
    product->set_effect_stub(effect_stub);
    product->set_offset(offset);
}



On_Death_Effect_Module_Stub::~On_Death_Effect_Module_Stub()
{
    delete effect_stub;
}
