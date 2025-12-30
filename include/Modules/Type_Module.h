#pragma once

#include <Module.h>


namespace Shardis
{

    enum class Object_Type
    {
        Unknown,
        Terrain,
        Player,
        Enemy,
        Player_Attack,
        Enemy_Attack
    };


    class Type_Module : public LEti::Module
    {
    public:
        INIT_VARIABLE(Shardis::Type_Module, LEti::Module)

    private:
        Object_Type m_object_type = Object_Type::Unknown;

    public:
        inline void set_object_type(Object_Type _value) { m_object_type = _value; }
        inline Object_Type object_type() const { return m_object_type; }

    public:
        void update(float _dt) override { }

    };


    class Type_Module_Stub : public LEti::Module_Stub
    {
    public:
        INIT_VARIABLE(Shardis::Type_Module_Stub, LEti::Module_Stub)

        INIT_FIELDS
        ADD_FIELD_RENAMED(std::string, "object_type", object_type_str)
        FIELDS_END

        OVERRIDE_ON_VALUES_ASSIGNED

    public:
        std::string object_type_str;

    public:
        Object_Type object_type = Object_Type::Unknown;

    public:
        INIT_BUILDER_STUB(Type_Module)

    };

}
