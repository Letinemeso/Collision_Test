#include <Modules/Misc_Modules/Health_Bar_Module.h>

using namespace Shardis;

namespace Shardis
{
    constexpr unsigned int No_Stage_DM_Index = 0xFFFFFFFF;
}


Health_Bar_Module::Health_Bar_Module()
{
    m_message_handle__entity_death = LST::Message_Translator::instance().subscribe<Message__On_Entity_Death>([this](Message__On_Entity_Death& _msg)
    {
        if(_msg.almost_dead_entity != m_player_object)
            return;

        m_player_object = nullptr;
        m_player_health_module = nullptr;
    });
}

Health_Bar_Module::~Health_Bar_Module()
{
    for(unsigned int i = 0; i < m_stages_dms.size(); ++i)
        delete m_stages_dms[i];

    LST::Message_Translator::instance().unsubscribe(m_message_handle__entity_death);
}



void Health_Bar_Module::set_stages_draw_modules(LDS::Vector<LR::Draw_Module*>&& _modules)
{
    m_stages_dms = (LDS::Vector<LR::Draw_Module*>&&)_modules;

    for(unsigned int i = 0; i < m_stages_dms.size(); ++i)
        m_stages_dms[i]->set_visible(false);
}



void Health_Bar_Module::M_on_parent_object_set()
{
    for(unsigned int i = 0; i < m_stages_dms.size(); ++i)
    {
        LR::Draw_Module* module = m_stages_dms[i];

        module->set_transformation_data(transformation_data());
        module->set_transformation_data_prev_state(transformation_data_prev_state());
        module->set_parent_object(parent_object());
    }
}



void Health_Bar_Module::M_find_player_if_needed()
{
    if(m_player_object)
        return;

    Message__Find_Player msg;
    LST::Message_Translator::instance().translate(msg);

    if(!msg.player_object)
        return;

    m_player_object = msg.player_object;
    m_player_health_module = m_player_object->get_module_of_type<Health_Module>();
}

void Health_Bar_Module::M_select_current_draw_module(unsigned int _index)
{
    if(_index == m_current_stage_dm_index)
        return;

    if(m_current_stage_dm)
        m_current_stage_dm->set_visible(false);

    if(_index == No_Stage_DM_Index)
    {
        m_current_stage_dm = nullptr;
        m_current_stage_dm_index = No_Stage_DM_Index;
        return;
    }

    m_current_stage_dm_index = _index;
    m_current_stage_dm = m_stages_dms[_index];

    m_current_stage_dm->set_visible(true);
}

unsigned int Health_Bar_Module::M_calculate_stage_index() const
{
    L_ASSERT(m_player_health_module);

    if(m_player_health_module->health() == 0)
        return No_Stage_DM_Index;

    float health_ratio = (float)m_player_health_module->health() / (float)m_player_health_module->base_health();
    unsigned int dm_index = (float)m_stages_dms.size() * health_ratio;
    if(dm_index >= m_stages_dms.size())
        dm_index = m_stages_dms.size() - 1;

    return dm_index;
}

void Health_Bar_Module::M_update_stage()
{
    if(!m_player_object)
        return;

    unsigned int current_index = M_calculate_stage_index();

    M_select_current_draw_module(current_index);
}



void Health_Bar_Module::update(float _dt)
{
    M_find_player_if_needed();

    M_update_stage();

    if(m_current_stage_dm)
        m_current_stage_dm->update(_dt);
}





BUILDER_STUB_DEFAULT_CONSTRUCTION_FUNC(Health_Bar_Module_Stub)

BUILDER_STUB_INITIALIZATION_FUNC(Health_Bar_Module_Stub)
{
    BUILDER_STUB_PARENT_INITIALIZATION;
    BUILDER_STUB_CAST_PRODUCT;

    unsigned int stages_amount = stages_draw_modules.size();
    LDS::Vector<LR::Draw_Module*> dms(stages_amount);

    for(LV::Variable_Base::Childs_List::Const_Iterator it = stages_draw_modules.begin(); !it.end_reached(); ++it)
    {
        LR::Draw_Module_Stub* stub = (LR::Draw_Module_Stub*)it->child_ptr;
        L_ASSERT(LV::cast_variable<LR::Draw_Module_Stub>(stub));

        dms.push( LR::Draw_Module_Stub::construct_from(stub) );
    }

    product->set_stages_draw_modules( (LDS::Vector<LR::Draw_Module*>&&)dms );
}
