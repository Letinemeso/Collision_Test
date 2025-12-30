#include <Modules/Light_Source_Module.h>

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



void Light_Source_Module::M_on_parent_object_set()
{
    L_ASSERT(m_light_controller);

    m_light_source = m_light_controller->add_light_source(transformation_data()->position(), m_light_color, m_max_light_distance);
}



void Light_Source_Module::update(float _dt)
{
    m_light_source->point = transformation_data()->position() + m_additional_offset;
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
}
