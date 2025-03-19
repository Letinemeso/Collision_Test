#include <Click_Controller.h>

#include <Window/Window_Controller.h>
#include <glfw3.h>
#include <Draw_Modules/Draw_Module.h>

#include <Collision_Detection/Broad_Phase/Binary_Space_Partitioner.h>
#include <Collision_Detection/Narrow_Phase/Dynamic_Narrow_CD.h>
#include <Collision_Detection/Narrow_Phase/Narrow_Phase__Model_Vs_Point.h>
#include <Modules/Physics_Module__Point.h>

#include <Control_Module.h>
#include <Collision_Resolution.h>
#include <Maybe_Immovable_Physics_Module.h>


Click_Controller::Click_Controller()
{
    LPhys::Physics_Module__Point* point_pm = new LPhys::Physics_Module__Point;
    m_point_object.add_module(point_pm);

    {
        LPhys::Binary_Space_Partitioner* bp = new LPhys::Binary_Space_Partitioner;
        bp->set_precision(2);
        m_collision_detector__click.set_broad_phase(bp);
        LPhys::Narrow_Phase__Model_Vs_Point* np = new LPhys::Narrow_Phase__Model_Vs_Point;
        m_collision_detector__click.set_narrow_phase(np);

        m_collision_detector__click.register_module(point_pm);
    }

    {
        LPhys::Binary_Space_Partitioner* bp = new LPhys::Binary_Space_Partitioner;
        bp->set_precision(2);
        m_collision_detector__objects.set_broad_phase(bp);
        LPhys::Dynamic_Narrow_CD* np = new LPhys::Dynamic_Narrow_CD;
        np->set_precision(5);
        m_collision_detector__objects.set_narrow_phase(np);

        m_collision_resolver__objects.set_resolution(new Collision_Resolution);
    }
}

Click_Controller::~Click_Controller()
{
    for(Objects_List::Iterator it = m_objects_list.begin(); !it.end_reached(); ++it)
        delete *it;

    delete m_object_stub;
}



LEti::Object* Click_Controller::M_clicked_on_object()
{
    glm::vec3 clicked_at = m_camera->convert_window_coords({ LR::Window_Controller::get_cursor_position().x, LR::Window_Controller::get_cursor_position().y, 0.0f });
    m_point_object.current_state().set_position(clicked_at);
    m_point_object.update(0.0f);
    m_point_object.update_previous_state();

    m_collision_detector__click.update_with_external_models(m_collision_detector__objects.registred_modules());
    const LPhys::Collision_Detector::Intersection_Data_List& found_collisions = m_collision_detector__click.found_collisions();

    if(found_collisions.size() == 0)
        return nullptr;

    const LPhys::Intersection_Data& id = *found_collisions.begin();

    LPhys::Physics_Module_2D* module = (LPhys::Physics_Module_2D*)(LV::cast_variable<LPhys::Physics_Module_2D>(id.first) ? id.first : id.second);
    return module->parent_object();
}


void Click_Controller::M_process_movement_application()
{
    if(m_held_object)
        return;

    if(!LR::Window_Controller::is_key_down(GLFW_KEY_LEFT_CONTROL))
        return;

    if(!LR::Window_Controller::mouse_button_was_pressed(GLFW_MOUSE_BUTTON_1))
        return;

    LEti::Object* clicked_on = M_clicked_on_object();
    if(!clicked_on)
        return;

    Control_Module* cm = clicked_on->get_module_of_type<Control_Module>();
    if(cm)
    {
        clicked_on->remove_module(cm);
        delete cm;
        return;
    }
    else
    {
        cm = new Control_Module;
        clicked_on->add_module(cm, false);
    }
}

void Click_Controller::M_process_movability_setting()
{
    if(m_held_object)
        return;

    if(!LR::Window_Controller::is_key_down(GLFW_KEY_M))
        return;

    if(!LR::Window_Controller::mouse_button_was_pressed(GLFW_MOUSE_BUTTON_1))
        return;

    LEti::Object* clicked_on = M_clicked_on_object();
    if(!clicked_on)
        return;

    Maybe_Immovable_Physics_Module* pm = clicked_on->get_module_of_type<Maybe_Immovable_Physics_Module>();
    L_ASSERT(pm);
    pm->set_movable(!pm->movable());
}

void Click_Controller::M_process_object_selection()
{
    if(m_held_object)
    {
        if(LR::Window_Controller::mouse_button_was_released(GLFW_MOUSE_BUTTON_1))
            m_held_object = nullptr;
        return;
    }

    if(!LR::Window_Controller::mouse_button_was_pressed(GLFW_MOUSE_BUTTON_1))
        return;

    m_held_object = M_clicked_on_object();
    if(!m_held_object)
        return;

    glm::vec3 clicked_at = m_camera->convert_window_coords({ LR::Window_Controller::get_cursor_position().x, LR::Window_Controller::get_cursor_position().y, 0.0f });
    m_holding_point_offset = m_held_object->current_state().position() - clicked_at;
}

void Click_Controller::M_process_object_movement()
{
    if(!m_held_object)
        return;

    glm::vec3 clicked_at = m_camera->convert_window_coords({ LR::Window_Controller::get_cursor_position().x, LR::Window_Controller::get_cursor_position().y, 0.0f });
    m_held_object->current_state().set_position(clicked_at + m_holding_point_offset);
}

void Click_Controller::M_process_object_creation()
{
    if(m_held_object)
        return;

    if(LR::Window_Controller::is_key_down(GLFW_KEY_LEFT_CONTROL))
        return;

    if(!LR::Window_Controller::mouse_button_was_pressed(GLFW_MOUSE_BUTTON_1))
        return;

    glm::vec3 clicked_at = m_camera->convert_window_coords({ LR::Window_Controller::get_cursor_position().x, LR::Window_Controller::get_cursor_position().y, 0.0f });
    LEti::Object* object = LEti::Object_Stub::construct_from(m_object_stub);
    object->current_state().set_position(clicked_at);
    object->update(0.0f);
    object->update_previous_state();
    m_objects_list.push_back(object);

    object->process_logic_for_modules_of_type<LPhys::Physics_Module_2D>([this](LPhys::Physics_Module_2D* _module)
    {
        m_collision_detector__objects.register_module(_module);
    });
}

void Click_Controller::M_process_object_removal()
{
    if(m_held_object)
        return;

    if(!LR::Window_Controller::mouse_button_was_pressed(GLFW_MOUSE_BUTTON_2))
        return;

    LEti::Object* clicked_on = M_clicked_on_object();
    if(!clicked_on)
        return;

    clicked_on->process_logic_for_modules_of_type<LPhys::Physics_Module_2D>([this](LPhys::Physics_Module_2D* _module)
    {
        m_collision_detector__objects.unregister_module(_module);
    });

    Objects_List::Iterator object_it = m_objects_list.find(clicked_on);
    L_ASSERT(object_it.is_ok());
    m_objects_list.erase(object_it);
}



void Click_Controller::update(float _dt)
{
    for(Objects_List::Iterator it = m_objects_list.begin(); !it.end_reached(); ++it)
    {
        LEti::Object* object = *it;
        object->update_previous_state();
    }

    M_process_movement_application();
    M_process_movability_setting();
    M_process_object_selection();
    M_process_object_movement();
    M_process_object_creation();
    M_process_object_removal();

    for(Objects_List::Iterator it = m_objects_list.begin(); !it.end_reached(); ++it)
    {
        LEti::Object* object = *it;
        object->update(_dt);
    }

    m_collision_detector__objects.update();
    m_collision_resolver__objects.resolve_all(m_collision_detector__objects.found_collisions(), _dt);

    for(Objects_List::Iterator it = m_objects_list.begin(); !it.end_reached(); ++it)
    {
        LEti::Object* object = *it;
        object->process_logic_for_modules_of_type<LR::Draw_Module>([](LR::Draw_Module* _module)
        {
            _module->draw();
        });
    }
}
