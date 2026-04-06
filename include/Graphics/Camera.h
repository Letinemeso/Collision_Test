#pragma once

#include <Stuff/Key_Frame_Animation.h>

#include <Camera/Camera_3D.h>


namespace Shardis
{

    class Camera : public LR::Camera_3D
    {
    private:
        LR::Camera_3D::Settings m_settings;

    private:
        LST::Key_Frame_Animation<glm::vec3> m_screenshake_kfa;

    public:
        Camera();
        ~Camera();

    public:
        inline LR::Camera_3D::Settings& configurable_settings() { return m_settings; }

    public:
        void set_settings(const Settings& _settings) override;
        void apply_settings_forcefully();
        void start_screenshake(float _max_stride, float _duration, float _intensity);
        void reset_screenshake();

    private:
        void M_apply_screenshake(LR::Camera_3D::Settings& _settings) const;

    public:
        void update(float _dt);

    };

}
