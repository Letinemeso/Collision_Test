#pragma once

#include <Object.h>

#include <Camera/Camera_2D.h>

#include <Collision_Detection/Collision_Detector.h>
#include <Collision_Resolution/Collision_Resolver.h>


class Click_Controller
{
private:
    const LR::Camera_2D* m_camera = nullptr;

    LEti::Object_Stub* m_object_stub = nullptr;

private:
    using Objects_List = LDS::List<LEti::Object*>;
    Objects_List m_objects_list;

    LEti::Object* m_held_object = nullptr;

    LPhys::Collision_Detector m_collision_detector__objects;
    LPhys::Collision_Resolver m_collision_resolver__objects;
    LPhys::Collision_Detector m_collision_detector__click;
    LEti::Object m_point_object;

public:
    Click_Controller();
    ~Click_Controller();

public:
    inline void inject_camera(const LR::Camera_2D* _ptr) { m_camera = _ptr; }
    inline void set_object_stub(LEti::Object_Stub* _stub) { delete m_object_stub; m_object_stub = _stub; }

private:
    LEti::Object* M_clicked_on_object();

    void M_process_object_selection();
    void M_process_object_movement();
    void M_process_object_creation();
    void M_process_object_removal();

public:
    void update(float _dt);

};
