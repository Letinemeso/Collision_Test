#include <Graphics/Camera.h>

#include <Stuff/Math_Stuff.h>

using namespace Shardis;


Camera::Camera()
{

}

Camera::~Camera()
{

}



void Camera::set_settings(const Settings& _settings)
{
    m_settings = _settings;
}

void Camera::apply_settings_forcefully()
{
    LR::Camera_3D::set_settings(m_settings);
}

void Camera::start_screenshake(float _max_stride, float _duration, float _intensity)
{
    L_ASSERT(_max_stride > 0.0f);
    L_ASSERT(_duration > 0.0f);
    L_ASSERT(_intensity > 0.0f);

    glm::vec3 previous_stride = {0.0f, 0.0f, 0.0f};
    if(m_screenshake_kfa.active())
        previous_stride = m_screenshake_kfa.current_value();

    m_screenshake_kfa.clear();
    m_screenshake_kfa.add_frame(0.0f, previous_stride);

    float expected_total_stride = _duration * _intensity;
    float total_duration = 0.0f;

    for(float total_stride = 0.0f; total_stride < expected_total_stride;)
    {
        glm::vec3 random_stride = LST::Math::random_vec3({-1.0f, -1.0f, -1.0f}, {1.0f, 1.0f, 1.0f});
        LST::Math::extend_vector_to_length(random_stride, _max_stride);

        float distance = LST::Math::vector_length(random_stride);
        total_stride += distance;
        float expected_travel_time = distance / _intensity;
        total_duration += expected_travel_time;
        previous_stride = random_stride;

        m_screenshake_kfa.add_frame(total_duration, random_stride);
    }

    float distance_to_start = LST::Math::vector_length(previous_stride);
    float expected_travel_time_to_start = distance_to_start / _intensity;
    m_screenshake_kfa.add_frame(total_duration + expected_travel_time_to_start, {0.0f, 0.0f, 0.0f});
    m_screenshake_kfa.start();
}

void Camera::reset_screenshake()
{
    m_screenshake_kfa.clear();
}



void Camera::M_apply_screenshake(LR::Camera_3D::Settings& _settings) const
{
    if(!m_screenshake_kfa.active())
        return;

    _settings.position += m_screenshake_kfa.current_value();
}



void Camera::update(float _dt)
{
    m_screenshake_kfa.update(_dt);

    LR::Camera_3D::Settings settings = m_settings;
    M_apply_screenshake(settings);

    LR::Camera_3D::set_settings(settings);
}
