#pragma once

#include <Stuff/Function_Wrapper.h>

#include <Camera/Camera_3D.h>


namespace Shardis
{

    class Camera_Controller
    {
    private:
        LR::Camera_3D* m_camera = nullptr;

        LST::Function<const glm::vec3&()> m_position_getter_func;
        bool m_follow_mode = false;

    public:
        inline void inject_camera(LR::Camera_3D* _ptr) { m_camera = _ptr; }

        inline void set_position_getter_func(const LST::Function<const glm::vec3&()>& _value) { m_position_getter_func = _value; }

    private:
        void M_process_utility(float _dt);
        void M_process_movement(float _dt);
        void M_process_view(float _dt);

    public:
        void update(float _dt);

    };

}
