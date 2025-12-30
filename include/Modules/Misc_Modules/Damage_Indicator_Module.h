#pragma once

#include <Data_Structures/List.h>
#include <Stuff/Message_Translator.h>

#include <Module.h>

#include <Draw_Modules/Draw_Module.h>
#include <Camera/Camera_3D.h>

#include <Misc_Draw_Modules/Animation/Graphics_Component__Animation/Graphics_Component__Animation.h>

#include <Messages/Messages.h>


namespace Shardis
{

    class Damage_Indicator_Module : public LEti::Module
    {
    public:
        INIT_VARIABLE(Shardis::Damage_Indicator_Module, LEti::Module)

    private:
        struct Indicator_Instance
        {
            LR::Draw_Module* module = nullptr;
            LMD::Graphics_Component__Animation* animation_controller = nullptr;
            LEti::Transformation_Data transformation_data;
            glm::vec3 damage_source;
        };

    private:
        const LR::Camera_3D* m_camera = nullptr;
        LR::Draw_Module_Stub* m_indicator_stub = nullptr;

        using Indicators_List = LDS::List<Indicator_Instance>;
        Indicators_List m_indicators;

        LEti::Object* m_player_object = nullptr;

        LST::Message_Translator::Handle<Message__On_Entity_Death> m_message_handle__on_death;
        LST::Message_Translator::Handle<Message__On_Entity_Damaged> m_message_handle__on_damaged;

    public:
        Damage_Indicator_Module();
        ~Damage_Indicator_Module();

    public:
        inline void inject_camera(const LR::Camera_3D* _ptr) { m_camera = _ptr; }
        inline void set_indicator_stub(LR::Draw_Module_Stub* _ptr) { delete m_indicator_stub; m_indicator_stub = _ptr; }

    private:
        void M_update_indicator_transformation(const glm::vec3& _damage_source_position, LEti::Transformation_Data& _transformation) const;
        void M_add_indicator(const glm::vec3& _from);
        void M_clear_indicators();
        void M_find_player_if_needed();

    public:
        void update(float _dt) override;

    };


    class Damage_Indicator_Module_Stub : public LEti::Module_Stub
    {
    public:
        INIT_VARIABLE(Shardis::Damage_Indicator_Module_Stub, LEti::Module_Stub)

        INIT_CHILDS
        ADD_CHILD("indicator_draw_module", indicator_draw_module)
        CHILDS_END

    public:
        LR::Draw_Module_Stub* indicator_draw_module = nullptr;

    public:
        const LR::Camera_3D* camera = nullptr;

    public:
        INIT_BUILDER_STUB(Damage_Indicator_Module)

    public:
        ~Damage_Indicator_Module_Stub();

    };

}
