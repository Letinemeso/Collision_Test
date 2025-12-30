#include <Modules/Misc_Modules/Damage_Indicator_Module.h>

#include <Math_Stuff.h>

using namespace Shardis;


Damage_Indicator_Module::Damage_Indicator_Module()
{
    m_message_handle__on_death = LST::Message_Translator::instance().subscribe<Message__On_Entity_Death>([this](Message__On_Entity_Death& _msg)
    {
        if(_msg.almost_dead_entity != m_player_object)
            return;

        m_player_object = nullptr;
        M_clear_indicators();
    });

    m_message_handle__on_damaged = LST::Message_Translator::instance().subscribe<Message__On_Entity_Damaged>([this](Message__On_Entity_Damaged& _msg)
    {
        if(!m_player_object)
            return;
        if(_msg.damaged_entity != m_player_object)
            return;

        glm::vec3 damage_source = m_player_object->current_state().position();
        if(_msg.damaging_entity)
            damage_source = _msg.damaging_entity->current_state().position();

        M_add_indicator(damage_source);
    });
}

Damage_Indicator_Module::~Damage_Indicator_Module()
{
    LST::Message_Translator::instance().unsubscribe(m_message_handle__on_death);
    LST::Message_Translator::instance().unsubscribe(m_message_handle__on_damaged);

    M_clear_indicators();

    delete m_indicator_stub;
}



void Damage_Indicator_Module::M_update_indicator_transformation(const glm::vec3& _damage_source_position, LEti::Transformation_Data& _transformation) const
{
    if(!m_player_object)
        return;

    glm::vec3 vec_to_damage_source = m_player_object->current_state().position() - _damage_source_position;
    vec_to_damage_source.y = 0.0f;
    if(LEti::Math::vector_length_squared(vec_to_damage_source) < 1e-6f)
    {
        _transformation.set_rotation({0.0f, 0.0f, 0.0f});
        return;
    }

    glm::vec3 camera_direction_flattened = m_camera->settings().direction;
    camera_direction_flattened.y = 0;
    if(LEti::Math::vector_length_squared(camera_direction_flattened) < 1e-6f)
    {
        _transformation.set_rotation({0.0f, 0.0f, 0.0f});
        return;
    }

    LEti::Math::shrink_vector_to_1(vec_to_damage_source);
    LEti::Math::shrink_vector_to_1(camera_direction_flattened);

    float angle_cos = LEti::Math::angle_cos_between_vectors(camera_direction_flattened, vec_to_damage_source);
    float angle = acos(angle_cos);

    if(LEti::Math::cross_product(camera_direction_flattened, vec_to_damage_source).y < 0.0f)
        angle = LEti::Math::DOUBLE_PI - angle;

    angle += LEti::Math::HALF_PI + LEti::Math::PI;

    _transformation.set_rotation({0.0f, 0.0f, angle});
}

void Damage_Indicator_Module::M_add_indicator(const glm::vec3& _from)
{
    m_indicators.push_back({});

    Indicator_Instance& instance = *m_indicators.end();

    instance.module = LR::Draw_Module_Stub::construct_from(m_indicator_stub);
    instance.animation_controller = (LMD::Graphics_Component__Animation*)instance.module->get_graphics_component_with_buffer_index(1);
    L_ASSERT(LV::cast_variable<LMD::Graphics_Component__Animation>(instance.animation_controller));
    instance.damage_source = _from;
    instance.transformation_data = *transformation_data();

    instance.module->set_transformation_data(&instance.transformation_data);
    instance.module->set_transformation_data_prev_state(&instance.transformation_data);

    LEti::Transformation_Data* instance_transformation_data = &instance.transformation_data;

    instance.animation_controller->set_on_prepare_func([this, _from, instance_transformation_data](const LR::Graphics_Component*)
    {
        M_update_indicator_transformation(_from, *instance_transformation_data);
    });
}

void Damage_Indicator_Module::M_clear_indicators()
{
    for(Indicators_List::Iterator it = m_indicators.begin(); !it.end_reached(); ++it)
        delete it->module;
    m_indicators.clear();
}

void Damage_Indicator_Module::M_find_player_if_needed()
{
    if(m_player_object)
        return;

    Message__Find_Player msg;
    LST::Message_Translator::instance().translate(msg);

    m_player_object = msg.player_object;
}



void Damage_Indicator_Module::update(float _dt)
{
    M_find_player_if_needed();

    Indicators_List::Iterator it = m_indicators.begin();
    while(!it.end_reached())
    {
        Indicator_Instance& instance = *it;

        instance.module->update(_dt);

        if(!instance.animation_controller->paused())
        {
            ++it;
            continue;
        }

        delete it->module;
        it = m_indicators.erase_and_iterate_forward(it);
    }
}





BUILDER_STUB_DEFAULT_CONSTRUCTION_FUNC(Damage_Indicator_Module_Stub)

BUILDER_STUB_INITIALIZATION_FUNC(Damage_Indicator_Module_Stub)
{
    BUILDER_STUB_PARENT_INITIALIZATION;
    BUILDER_STUB_CAST_PRODUCT;

    product->inject_camera(camera);

    LR::Draw_Module_Stub* indicator_dm_stub_copy = LR::Draw_Module_Stub::construct_copy_from(indicator_draw_module);
    product->set_indicator_stub(indicator_dm_stub_copy);
}



Damage_Indicator_Module_Stub::~Damage_Indicator_Module_Stub()
{
    delete indicator_draw_module;
}
