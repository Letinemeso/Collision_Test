#include <Modules/Light_Source_Module.h>

#include <Stuff/Math_Stuff.h>

using namespace Shardis;


Light_Source_Module::Light_Source_Module()
{

}

Light_Source_Module::~Light_Source_Module()
{
    m_light_controller->remove_light_source(m_light_source);
}



void Light_Source_Module::set_light_color(const glm::vec3& _color)
{
    m_light_color = _color;

    if(m_light_source)
        m_light_source->color = _color;
}

void Light_Source_Module::set_max_light_distance(float _max_distance)
{
    m_max_light_distance = _max_distance;

    if(m_light_source)
        m_light_source->max_light_spread_distance = _max_distance;
}


void Light_Source_Module::start_fade(const Fade_Parameters& _parameters)
{
    m_fade_kfa.clear();

    m_fade_kfa.add_frame(0.0f, _parameters.start_max_light_distance);

    if(_parameters.initial_pause > 0.0f)
        m_fade_kfa.add_frame(_parameters.initial_pause, _parameters.start_max_light_distance);

    m_fade_kfa.add_frame(_parameters.duration, _parameters.target_max_light_distance);

    m_fade_kfa.set_run_in_loop(false);
    m_fade_kfa.start();
}

void Light_Source_Module::start_pulse(const Pulse_Parameters& _parameters)
{
    m_queue.clear();
    M_construct_pulse_kfa(_parameters);
}



void Light_Source_Module::M_on_parent_object_set()
{
    L_ASSERT(m_light_controller);

    m_light_source = m_light_controller->add_light_source(transformation_data()->position(), m_light_color, m_max_light_distance);
}


void Light_Source_Module::M_construct_pulse_kfa(const Pulse_Parameters& _parameters)
{
    float min_distance = m_max_light_distance * _parameters.min_distance_ratio;
    float max_distance = m_max_light_distance * _parameters.max_distance_ratio;

    constexpr unsigned int Interpolation_Steps = 12;
    constexpr float Interpolation_Step_Multiplier = 1.0f / (float)Interpolation_Steps;

    float minmax_distance_difference = max_distance - min_distance;
    float minmax_distance_difference_half = minmax_distance_difference * 0.5f;
    float distance_offset = min_distance + minmax_distance_difference_half;

    m_fade_kfa.clear();
    m_fade_kfa.add_frame(0.0f, min_distance);

    for(unsigned int i = 1; i < Interpolation_Steps; ++i)
    {
        float multiplier = Interpolation_Step_Multiplier * (float)i;
        float timestamp = _parameters.duration * multiplier;
        float distance = distance_offset + ( minmax_distance_difference_half * sinf(LST::Math::DOUBLE_PI * multiplier + LST::Math::PI + LST::Math::HALF_PI) );
        m_fade_kfa.add_frame(timestamp, distance);
    }

    m_fade_kfa.add_frame(_parameters.duration, min_distance);

    m_fade_kfa.set_run_in_loop(false);
    m_fade_kfa.start();
}

void Light_Source_Module::M_activate_pulse_if_needed()
{
    if(m_pulse_parameters.min_distance_ratio <= 0.0f || m_pulse_parameters.max_distance_ratio <= 0.0f || m_pulse_parameters.duration <= 0.0f)
        return;
    if(m_fade_kfa.active())
        return;

    m_queue.clear();

    start_fade({m_light_source->max_light_spread_distance, m_max_light_distance * m_pulse_parameters.min_distance_ratio, m_pulse_parameters.duration * 0.5f, 0.0f});

    m_queue.wait([this]()
    {
        return !m_fade_kfa.active();
    });

    m_queue.call([this]()
    {
        M_construct_pulse_kfa(m_pulse_parameters);
        m_fade_kfa.set_run_in_loop(true);
    });
}



void Light_Source_Module::update(float _dt)
{
    glm::vec3 additional_offset = (transformation_data()->rotation_matrix() * transformation_data()->scale_matrix()) * glm::vec4(m_additional_offset, 1.0f);
    m_light_source->point = transformation_data()->position() + additional_offset;

    m_fade_kfa.update(_dt);
    m_queue.process();

    M_activate_pulse_if_needed();

    if(m_fade_kfa.active())
        m_light_source->max_light_spread_distance = m_fade_kfa.current_value();
    else
        m_light_source->max_light_spread_distance = m_max_light_distance;
}





BUILDER_STUB_DEFAULT_CONSTRUCTION_FUNC(Light_Source_Module_Stub)

BUILDER_STUB_INITIALIZATION_FUNC(Light_Source_Module_Stub)
{
    BUILDER_STUB_PARENT_INITIALIZATION;
    BUILDER_STUB_CAST_PRODUCT;

    L_ASSERT(light_controller);

    product->inject_light_controller(light_controller);
    product->set_additional_offset(additional_offset);

    product->set_light_color(light_color);
    product->set_max_light_distance(max_light_distance);

    if(smooth_appearance_duration > 0.0f)
        product->start_fade({ 0.0f, max_light_distance, smooth_appearance_duration, smooth_appearance_initial_pause });

    product->set_pulse_parameters({pulse_min_distance_ratio, pulse_max_distance_ratio, pulse_duration});
}
