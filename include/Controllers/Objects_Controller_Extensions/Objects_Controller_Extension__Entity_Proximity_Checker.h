#pragma once

#include <Stuff/Message_Translator.h>

#include <Object.h>

#include <Camera/Camera_3D.h>

#include <Tools/Objects_Controller/Objects_Controller_Extension.h>

#include <Messages/Messages.h>


namespace Shardis
{

class Objects_Controller_Extension__Entity_Proximity_Checker : public LMD::Objects_Controller_Extension
{
private:
    const LR::Camera_3D* m_camera = nullptr;

    float m_max_distance = 0.0f;
    float m_max_distance_squared = 0.0f;

public:
    inline void inject_camera(const LR::Camera_3D* _ptr) { m_camera = _ptr; }

    inline void set_max_distance(float _value) { m_max_distance = _value; m_max_distance_squared = _value * _value; }

public:
    void on_object_added(LEti::Object* _object) override { }
    void on_object_removed(LEti::Object* _object) override { }

    void reset() override { }
    void update(Objects_List& _objects, float _dt) override { }

    bool should_remove_object(LEti::Object* _object) override;
};

}
