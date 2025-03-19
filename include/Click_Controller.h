#pragma once

#include <Object.h>

#include <Camera/Camera_2D.h>


class Click_Controller
{
private:
    const LR::Camera_2D* m_camera = nullptr;

    LEti::Object_Stub* m_object_stub = nullptr;

private:
    using Objects_List = LDS::List<LEti::Object*>;
    Objects_List m_objects_list;

    LEti::Object* m_held_object = nullptr;

public:
    Click_Controller();
    ~Click_Controller();

public:
    inline void inject_camera(const LR::Camera_2D* _ptr) { m_camera = _ptr; }
    inline void set_object_stub(LEti::Object_Stub* _stub) { delete m_object_stub; m_object_stub = _stub; }

private:
    void M_process_mouse_click();

public:
    void update(float _dt);

};
