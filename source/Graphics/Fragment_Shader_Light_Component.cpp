#include <Graphics/Fragment_Shader_Light_Component.h>

#include <glew.h>

using namespace Shardis;


Fragment_Shader_Light_Component::Fragment_Shader_Light_Component()
{

}

Fragment_Shader_Light_Component::~Fragment_Shader_Light_Component()
{

}



Fragment_Shader_Light_Component::Light_Source_Data* Fragment_Shader_Light_Component::add_light_source()
{
    m_light_sources.push_back({});

    Light_Sources_Container::Iterator just_added_source_it = m_light_sources.end();
    Light_Source_Data& data = *just_added_source_it;
    return &data;
}

Fragment_Shader_Light_Component::Light_Source_Data* Fragment_Shader_Light_Component::add_light_source(const glm::vec3& _point, const glm::vec3& _color, float _max_light_spread_distance)
{
    Light_Source_Data* result = add_light_source();
    result->point = _point;
    result->color = _color;
    result->max_light_spread_distance = _max_light_spread_distance;

    return result;
}

void Fragment_Shader_Light_Component::remove_light_source(Light_Source_Data*& _light_source)
{
    L_ASSERT(_light_source);

    Light_Sources_Container::Iterator maybe_source_it = M_find_light_source(_light_source);
    L_ASSERT(!maybe_source_it.end_reached());

    m_light_sources.erase(maybe_source_it);
    _light_source = nullptr;
}



void Fragment_Shader_Light_Component::init(unsigned int _opengl_program_handle)
{
    LR::Shader_Component::init(_opengl_program_handle);

    m_relevant_light_sources_location = M_extract_uniform_location("relevant_light_sources_amount");

    for(unsigned int i=0; i<Max_Light_Sources; ++i)
    {
        Light_Source_Data_Uniforms& locations = m_light_sources_uniforms[i];

        std::string uniformName = "light_source_data[" + std::to_string(i) + "]";

        locations.color = M_extract_uniform_location(uniformName + ".color");
        locations.point = M_extract_uniform_location(uniformName + ".point");
        locations.max_light_spread_distance = M_extract_uniform_location(uniformName + ".max_light_spread_distance");
    }

    m_min_light_ratio_uniform = M_extract_uniform_location("min_light_ratio");
}



Fragment_Shader_Light_Component::Light_Sources_Container::Iterator Fragment_Shader_Light_Component::M_find_light_source(Light_Source_Data* _light_source)
{
    Light_Sources_Container::Iterator it = m_light_sources.begin();
    while(!it.end_reached())
    {
        Light_Source_Data& data = *it;
        if(&data == _light_source)
            break;

        ++it;
    }

    return it;
}



void Fragment_Shader_Light_Component::update(const LR::Draw_Module* _draw_module)
{
    LR::Shader_Component::update(_draw_module);

    unsigned int relevant_light_sources = 0;
    for(Light_Sources_Container::Iterator it = m_light_sources.begin(); !it.end_reached() && relevant_light_sources < Max_Light_Sources; ++it)
    {
        const Light_Source_Data& light_source_data = *it;
        if(!light_source_data.initialized() || !light_source_data.enabled)
            continue;

        Light_Source_Data_Uniforms& locations = m_light_sources_uniforms[relevant_light_sources];

        glUniform3fv(locations.color, 1, &light_source_data.color[0]);
        glUniform3fv(locations.point, 1, &light_source_data.point[0]);
        glUniform1f(locations.max_light_spread_distance, light_source_data.max_light_spread_distance);

        ++relevant_light_sources;
    }

    glUniform1i(m_relevant_light_sources_location, relevant_light_sources);

    glUniform1f(m_min_light_ratio_uniform, m_min_light_ratio);
}





BUILDER_STUB_DEFAULT_CONSTRUCTION_FUNC(Fragment_Shader_Light_Component_Stub)

BUILDER_STUB_INITIALIZATION_FUNC(Fragment_Shader_Light_Component_Stub)
{
    BUILDER_STUB_PARENT_INITIALIZATION;
    BUILDER_STUB_CAST_PRODUCT;
}
