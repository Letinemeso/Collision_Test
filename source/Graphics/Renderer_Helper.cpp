#include <Graphics/Renderer_Helper.h>

#include <Stuff/Message_Translator.h>

#include <MDL_Reader.h>

#include <glew.h>
#include <Components/Graphics_Component__Texture.h>
#include <Texture/Texture__Frame_Buffer.h>
#include <Window/Window_Controller.h>

#include <Messages/Messages.h>

using namespace Shardis;


Renderer_Helper::Renderer_Helper()
{
    m_draw_order_controller.add_layer("Default").set_before_draw_function([]()
    {
        Message__Update_Camera msg;
        LST::Message_Translator::instance().translate(msg);
        glEnable(GL_DEPTH_TEST);
    });

    m_draw_order_controller.add_layer("Main_Game_UI").set_before_draw_function([]()
    {
        glDisable(GL_DEPTH_TEST);
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

void Renderer_Helper::inject_base_game_camera(LR::Camera_Base* _ptr)
{
    m_base_game_camera = _ptr;

    m_draw_order_controller.configure_layer("Default").set_camera(m_base_game_camera);
}


void Renderer_Helper::init(const LV::Object_Constructor& _object_constructor, const LR::Shader_Manager* _shader_manager)
{
    LR::Shader_Program* final_shader = _shader_manager->get_shader_program("Shader_Program__Final_Shader");
    LR::Shader* fragment_shader = final_shader->get_shader_of_type(LR::Shader_Type::Fragment);
    L_ASSERT(fragment_shader);
    m_light_controller = fragment_shader->get_shader_component_of_type<Fragment_Shader_Light_Component>();
    L_ASSERT(m_light_controller);

    {
        LV::MDL_Reader reader;
        reader.parse_file("../Resources/Shaders/Helpers/Geometry_Rendering_Helper");
        LEti::Object_Stub* stub = (LEti::Object_Stub*)_object_constructor.construct(reader.get_stub("Geometry_Rendering_Helper"));
        m_base_game_rendering_object = LEti::Object_Stub::construct_from(stub);
        delete stub;
    }

    LR::Draw_Module* rendering_object_dm = m_base_game_rendering_object->get_module_of_type<LR::Draw_Module>();
    L_ASSERT(rendering_object_dm);

    LR::Graphics_Component__Texture* base_game_texture_component = (LR::Graphics_Component__Texture*)rendering_object_dm->get_graphics_component_with_buffer_index(1);
    L_ASSERT(LV::cast_variable<LR::Graphics_Component__Texture>(base_game_texture_component));
    m_base_game_frame_buffer = (LR::Texture__Frame_Buffer*)base_game_texture_component->texture();
    L_ASSERT(LV::cast_variable<LR::Texture__Frame_Buffer>(m_base_game_frame_buffer));

    LST::Message_Translator::instance().subscribe<LR::Message__Window_Resized>([this](LR::Message__Window_Resized&)
    {
        glm::vec2 widow_size = LR::Window_Controller::instance().get_window_size();
        m_base_game_frame_buffer->set_size(widow_size.x, widow_size.y);
    });

    m_base_game_frame_buffer->set_draw_func([this]()
    {
        m_draw_order_controller.draw_layer("Default");
    });
}



void Renderer_Helper::update(float _dt)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    m_base_game_rendering_object->update_previous_state();

    m_base_game_rendering_object->update(_dt);

    m_draw_order_controller.draw_layer("Main_Game_UI");

    LR::Window_Controller::instance().swap_buffers();
}
