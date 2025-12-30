#pragma once

#include <Stuff/Message_Translator.h>

#include <Module.h>

#include <Draw_Modules/Draw_Module.h>

#include <Messages/Messages.h>
#include <Modules/Health_Module.h>


namespace Shardis
{

    class Health_Bar_Module : public LEti::Module
    {
    public:
        INIT_VARIABLE(Shardis::Health_Bar_Module, LEti::Module)

    private:
        LDS::Vector<LR::Draw_Module*> m_stages_dms;

        LR::Draw_Module* m_current_stage_dm = nullptr;
        unsigned int m_current_stage_dm_index = 0;

        LEti::Object* m_player_object = nullptr;
        Health_Module* m_player_health_module = nullptr;

        LST::Message_Translator::Handle<Message__On_Entity_Death> m_message_handle__entity_death;

    public:
        Health_Bar_Module();
        ~Health_Bar_Module();

    public:
        void set_stages_draw_modules(LDS::Vector<LR::Draw_Module*>&& _modules);

    private:
        void M_on_parent_object_set() override;

    private:
        void M_find_player_if_needed();
        void M_select_current_draw_module(unsigned int _index);
        unsigned int M_calculate_stage_index() const;
        void M_update_stage();

    public:
        void update(float _dt) override;

    };


    class Health_Bar_Module_Stub : public LEti::Module_Stub
    {
    public:
        INIT_VARIABLE(Shardis::Health_Bar_Module_Stub, LEti::Module_Stub)

        INIT_CHILDS_LISTS
        ADD_CHILDS_LIST("Stage_Draw_Module__*", &stages_draw_modules)
        CHILDS_LISTS_END

    public:
        LV::Variable_Base::Childs_List stages_draw_modules;

    public:
        INIT_BUILDER_STUB(Health_Bar_Module)

    };

}
