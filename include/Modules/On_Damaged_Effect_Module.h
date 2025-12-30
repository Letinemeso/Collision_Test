#pragma once

#include <Stuff/Message_Translator.h>

#include <Object.h>

#include <Tools/Objects_Storage.h>
#include <Tools/Objects_Controller/Objects_Controller.h>

#include <Messages/Messages.h>


namespace Shardis
{

    class On_Damaged_Effect_Module : public LEti::Module
    {
    public:
        INIT_VARIABLE(Shardis::On_Damaged_Effect_Module, LEti::Module)

    private:
        LMD::Objects_Controller* m_objects_controller = nullptr;
        const LEti::Object_Stub* m_effect_stub = nullptr;

        LST::Message_Translator::Handle<Message__On_Entity_Damaged> m_message_handle;

        glm::vec3 m_offset = {0.0f, 0.0f, 0.0f};

    public:
        On_Damaged_Effect_Module();
        ~On_Damaged_Effect_Module();

    public:
        inline void inject_objects_controller(LMD::Objects_Controller* _ptr) { m_objects_controller = _ptr; }
        inline void set_effect_stub(const LEti::Object_Stub* _ptr) { m_effect_stub = _ptr; }
        inline void set_offset(const glm::vec3& _offset) { m_offset = _offset; }

    public:
        void update(float _dt) override { }

    };


    class On_Damaged_Effect_Module_Stub : public LEti::Module_Stub
    {
    public:
        INIT_VARIABLE(Shardis::On_Damaged_Effect_Module_Stub, LEti::Module_Stub)

        INIT_FIELDS
        ADD_FIELD(glm::vec3, offset)
        FIELDS_END

        INIT_CHILDS
        ADD_CHILD("effect_stub", effect_stub)
        CHILDS_END

    public:
        LMD::Objects_Controller* objects_controller = nullptr;

    public:
        LEti::Object_Stub* effect_stub = nullptr;

    public:
        glm::vec3 offset = {0.0f, 0.0f, 0.0f};

    public:
        INIT_BUILDER_STUB(On_Damaged_Effect_Module)

    public:
        ~On_Damaged_Effect_Module_Stub();

    };

}
