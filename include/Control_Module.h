#pragma once

#include <Module.h>


class Control_Module : public LEti::Module
{
public:
    INIT_VARIABLE(Control_Module, LEti::Module)

private:
    float m_speed = 200.0f;

public:
    void update(float _dt) override;

};
