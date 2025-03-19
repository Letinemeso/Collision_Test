#pragma once

#include <Collision_Resolution/Collision_Resolution__Physics_Module_2D.h>


class Collision_Resolution : public LPhys::Collision_Resolution__Physics_Module_2D
{
public:
    void resolve(const LPhys::Intersection_Data &_id, float _dt = 0.0f) override;

};
