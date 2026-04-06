#include <Graphics/Renderer_Helper.h>

#include <Stuff/Message_Translator.h>

#include <MDL_Reader.h>

#include <glew.h>
#include <Components/Graphics_Component__Texture.h>
#include <Texture/Texture__Frame_Buffer.h>
#include <Window/Window_Controller.h>

#include <Tools/Delaunay_Triangulation.h>

#include <Messages/Messages.h>

using namespace Shardis;

namespace Shardis
{
    LDS::Vector<float> construct_geometry_data(const LMD::Triangles_Vec& _triangles)
    {
        LDS::Vector<float> result(_triangles.size() * 6);

        for(unsigned int t_i = 0; t_i < _triangles.size(); ++t_i)
        {
            const LMD::Triangle& triangle = _triangles[t_i];

            for(unsigned int i = 0; i < 3; ++i)
            {
                constexpr glm::vec2 Decrement = {1.0f, 1.0f};
                glm::vec2 vertex = (triangle[i] * 2.0f) - Decrement;
                result.push(vertex.x);
                result.push(vertex.y);
            }
        }

        return result;
    }

    LDS::Vector<float> construct_texture_data(const LMD::Triangles_Vec& _triangles)
    {
        LDS::Vector<float> result(_triangles.size() * 6);

        auto calculate_triangle_minmax = [](const LMD::Triangle& _triangle, glm::vec2& _min, glm::vec2& _max)
        {
            _min.x = std::numeric_limits<float>::max();
            _min.y = std::numeric_limits<float>::max();
            _max.x = std::numeric_limits<float>::lowest();
            _max.y = std::numeric_limits<float>::lowest();

            for(unsigned int i = 0; i < 3; ++i)
            {
                if(_min.x > _triangle[i].x)
                    _min.x = _triangle[i].x;
                if(_min.y > _triangle[i].y)
                    _min.y = _triangle[i].y;
                if(_max.x < _triangle[i].x)
                    _max.x = _triangle[i].x;
                if(_max.y < _triangle[i].y)
                    _max.y = _triangle[i].y;
            }
        };

        for(unsigned int t_i = 0; t_i < _triangles.size(); ++t_i)
        {
            const LMD::Triangle& triangle = _triangles[t_i];

            float random_angle = LST::Math::random_number_float(0.0f, LST::Math::DOUBLE_PI);
            glm::mat4x4 rotation_matrix = LST::Math::calculate_rotation_matrix({0.0f, 0.0f, random_angle});

            LMD::Triangle rotated_triangle;
            for(unsigned int i = 0; i < 3; ++i)
                rotated_triangle[i] = rotation_matrix * glm::vec4(triangle[i], 0.0f, 1.0f);

            glm::vec2 min, max;
            calculate_triangle_minmax(rotated_triangle, min, max);

            glm::vec2 center = (min + max) * 0.5f;
            glm::vec2 extents = (max - min) * 0.5f;

            glm::vec2 min_offset = extents;
            glm::vec2 max_offset = glm::vec2(1.0f, 1.0f) - extents;

            glm::vec2 random_position = LST::Math::random_vec2(min_offset, max_offset);

            for(unsigned int i = 0; i < 3; ++i)
            {
                glm::vec2 vertex = rotated_triangle[i] - center + random_position;
                result.push(vertex.x);
                result.push(vertex.y);
            }
        }

        return result;
    }
}


Renderer_Helper::Renderer_Helper()
{
    LR::Draw_Order_Controller::Draw_Layer_Settings layer_settings = m_draw_order_controller.add_layer("Default");
    layer_settings.set_before_draw_function([]()
    {
        Message__Update_Camera msg;
        LST::Message_Translator::instance().translate(msg);
        glEnable(GL_DEPTH_TEST);
    });
    layer_settings.set_visibility_check_function([this](const LEti::Module* _module)
    {
        const LR::Draw_Module* draw_module = LV::cast_variable<LR::Draw_Module>(_module);
        if(!draw_module)
            return true;
        if(!draw_module->border())
            return true;

        return draw_module->border()->intersects(m_base_game_camera->frustum());
    });

    m_brightness_kfa.set_on_update_func([this](const float& _value)
    {
        m_brightness_controller->set_value(_value);
    });
}

Renderer_Helper::~Renderer_Helper()
{
    delete m_base_game_rendering_object;
}



void Renderer_Helper::inject_renderer(LR::Renderer* _ptr)
{
    m_renderer = _ptr;

    m_draw_order_controller.inject_renderer(m_renderer);
}

void Renderer_Helper::inject_base_game_camera(Camera* _ptr)
{
    m_base_game_camera = _ptr;

    m_draw_order_controller.configure_layer("Default").set_camera(m_base_game_camera);
}


void Renderer_Helper::init(const LV::Object_Constructor& _object_constructor, const LR::Shader_Manager* _shader_manager)
{
    LR::Shader_Program* default_shader = _shader_manager->get_shader_program("Shader_Program__Default");
    default_shader->set_uniform_vec3("glow_color", {0.0f, 0.0f, 0.0f});

    LR::Shader_Program* final_shader = _shader_manager->get_shader_program("Shader_Program__Final_Shader");
    LR::Shader* fragment_shader = final_shader->get_shader_of_type(LR::Shader_Type::Fragment);
    L_ASSERT(fragment_shader);
    m_light_controller = fragment_shader->get_shader_component_of_type<Fragment_Shader_Light_Component>();
    L_ASSERT(m_light_controller);

    m_brightness_controller = (LR::Shader_Component__Uniform__Float*)fragment_shader->get_shader_component("Shader_Component__brightness_uniform");
    L_ASSERT(LV::cast_variable<LR::Shader_Component__Uniform__Float>(m_brightness_controller));

    {
        LV::MDL_Reader reader;
        reader.parse_file("../Resources/Shaders/Helpers/Geometry_Rendering_Helper");
        LEti::Object_Stub* stub = (LEti::Object_Stub*)_object_constructor.construct(reader.get_stub("Geometry_Rendering_Helper"));
        m_base_game_rendering_object = LEti::Object_Stub::construct_from(stub);
        delete stub;
    }

    m_default_dm = m_base_game_rendering_object->get_module_of_type<LR::Draw_Module>(0);
    m_fractured_dm = m_base_game_rendering_object->get_module_of_type<LR::Draw_Module>(1);
    L_ASSERT(m_default_dm);
    L_ASSERT(m_fractured_dm);

    M_init_final_dm(m_default_dm);
    M_init_final_dm(m_fractured_dm);

    m_fractured_dm->set_visible(false);
}



void Renderer_Helper::M_init_final_dm(LR::Draw_Module* _module) const
{
    LR::Graphics_Component__Texture* base_game_texture_component = (LR::Graphics_Component__Texture*)_module->get_graphics_component_with_buffer_index(1);
    L_ASSERT(LV::cast_variable<LR::Graphics_Component__Texture>(base_game_texture_component));
    LR::Texture__Frame_Buffer* frame_buffer = (LR::Texture__Frame_Buffer*)base_game_texture_component->texture();
    L_ASSERT(LV::cast_variable<LR::Texture__Frame_Buffer>(frame_buffer));

    LST::Message_Translator::instance().subscribe<LR::Message__Window_Resized>([frame_buffer](LR::Message__Window_Resized&)
    {
        glm::vec2 widow_size = LR::Window_Controller::instance().get_window_size();
        frame_buffer->set_size(widow_size.x, widow_size.y);
    });

    frame_buffer->set_draw_func([this]()
    {
        m_draw_order_controller.draw_layer("Default");
    });
}



void Renderer_Helper::start_brightness_fade(float _start_value, float _end_value, float _duration)
{
    stop_brightness_fade();

    m_brightness_kfa.add_frame(0.0f, _start_value);
    m_brightness_kfa.add_frame(_duration, _end_value);
    m_brightness_kfa.start();
}

void Renderer_Helper::stop_brightness_fade()
{
    m_brightness_kfa.clear();
}

void Renderer_Helper::set_brightness(float _value)
{
    m_brightness_controller->set_value(_value);
}


bool Renderer_Helper::brightness_fade_completed() const
{
    return !m_brightness_kfa.active();
}


bool Renderer_Helper::is_default_view() const
{
    return m_default_dm->visible();
}

void Renderer_Helper::set_default_view()
{
    m_default_dm->set_visible(true);
    m_fractured_dm->set_visible(false);
}

void Renderer_Helper::set_fractured_view()
{
    m_default_dm->set_visible(false);
    m_fractured_dm->set_visible(true);

    constexpr unsigned int Random_Points_Amount = 20;
    constexpr unsigned int Total_Points_Amount = Random_Points_Amount + 8;
    LDS::Vector<glm::vec2> points(Total_Points_Amount);
    points.push({0.0f, 0.0f});
    points.push({1.0f, 0.0f});
    points.push({1.0f, 1.0f});
    points.push({0.0f, 1.0f});
    points.push({LST::Math::random_number_float(0.2f, 0.8f), 0.0f});
    points.push({LST::Math::random_number_float(0.2f, 0.8f), 1.0f});
    points.push({0.0f, LST::Math::random_number_float(0.2f, 0.8f)});
    points.push({1.0f, LST::Math::random_number_float(0.2f, 0.8f)});
    for(unsigned int i = 0; i < Random_Points_Amount; ++i)
        points.push( LST::Math::random_vec2({0.0f, 0.0f}, {1.0f, 1.0f}) );

    LMD::Triangles_Vec triangles = LMD::triangulate(points);

    LDS::Vector<float> geometry_data = construct_geometry_data(triangles);
    LDS::Vector<float> texture_data = construct_texture_data(triangles);

    LR::Graphics_Component__Default* gc_geometry = (LR::Graphics_Component__Default*)m_fractured_dm->get_graphics_component_with_buffer_index(0);
    L_ASSERT(LV::cast_variable<LR::Graphics_Component__Default>(gc_geometry));
    LR::Graphics_Component__Default* gc_texture = (LR::Graphics_Component__Default*)m_fractured_dm->get_graphics_component_with_buffer_index(1);
    L_ASSERT(LV::cast_variable<LR::Graphics_Component__Default>(gc_texture));

    gc_geometry->buffer().resize(geometry_data.size());
    gc_geometry->buffer().copy_array(geometry_data.raw_data(), geometry_data.size());
    gc_texture->buffer().resize(texture_data.size());
    gc_texture->buffer().copy_array(texture_data.raw_data(), texture_data.size());
}



void Renderer_Helper::update(float _dt)
{
    m_brightness_kfa.update(_dt);

    m_base_game_rendering_object->update_previous_state();

    m_base_game_rendering_object->update(_dt);

    m_base_game_camera->update(_dt);
}
