#pragma once

#include <Modules/Physics_Module_2D.h>


class Maybe_Immovable_Physics_Module : public LPhys::Physics_Module_2D
{
public:
    INIT_VARIABLE(Maybe_Immovable_Physics_Module, LPhys::Physics_Module_2D)

private:
    bool m_movable = true;

public:
    inline void set_movable(bool _value) { m_movable = _value; }
    inline bool movable() const { return m_movable; }

};


class Maybe_Immovable_Physics_Module_Stub : public LPhys::Physics_Module_2D_Stub
{
public:
    INIT_VARIABLE(Maybe_Immovable_Physics_Module_Stub, LPhys::Physics_Module_2D_Stub)

    INIT_FIELDS
    ADD_FIELD(bool, movable)
    FIELDS_END

public:
    bool movable = true;

public:
    INIT_BUILDER_STUB(Maybe_Immovable_Physics_Module)

};
