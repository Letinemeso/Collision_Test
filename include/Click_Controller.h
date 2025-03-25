#pragma once

#include <Object.h>

#include <Camera/Camera_2D.h>

#include <Collision_Detection/Collision_Detector.h>
#include <Collision_Resolution/Collision_Resolver.h>


class Click_Controller
{
public:
    struct Named_Object_Stub
    {
        std::string name;
        LEti::Object_Stub* stub = nullptr;
    };
    using Stubs_List = LDS::List<Named_Object_Stub>;

private:
    const LR::Camera_2D* m_camera = nullptr;

    Stubs_List m_object_stubs;
    Stubs_List::Iterator m_object_stub_it;

    using Objects_List = LDS::List<LEti::Object*>;
    Objects_List m_objects_list;

    LEti::Object* m_held_object = nullptr;
    glm::vec3 m_holding_point_offset = {0.0f, 0.0f, 0.0f};

    LPhys::Collision_Detector m_collision_detector__objects;
    LPhys::Collision_Resolver m_collision_resolver__objects;
    LPhys::Collision_Detector m_collision_detector__click;
    LEti::Object m_point_object;
    bool m_resolve_collisions = true;

public:
    Click_Controller();
    ~Click_Controller();

public:
    inline void inject_camera(const LR::Camera_2D* _ptr) { m_camera = _ptr; }

public:
    void set_object_stubs(Stubs_List&& _stubs);

private:
    LEti::Object* M_clicked_on_object();

    void M_process_object_rotation();
    void M_process_movement_application();
    void M_process_movability_setting();
    void M_process_object_selection();
    void M_process_object_movement();
    void M_process_object_stub_selection();
    void M_process_object_removal_or_creation();
    void M_process_enable_resolution();

public:
    void update(float _dt);

};
