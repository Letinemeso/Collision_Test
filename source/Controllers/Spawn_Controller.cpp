#include <Controllers/Spawn_Controller.h>

#include <Stuff/Message_Translator.h>

#include <Math_Stuff.h>

#include <Messages/Messages.h>

using namespace Shardis;


ON_VALUES_ASSIGNED_IMPLEMENTATION(Spawn_Controller)
{
    L_ASSERT(m_entity_provider);
    L_ASSERT(m_entity_ids.size() == m_weights.size());

    for(unsigned int i = 0; i < m_entity_ids.size(); ++i)
    {
        const std::string& entity_name = m_entity_ids[i];
        const LEti::Object_Stub* stub = m_entity_provider->get_object<LEti::Object_Stub>(entity_name);
        L_ASSERT(stub);

        m_weighted_objects.add_element(stub, m_weights[i]);
    }

    m_spawn_delay_timer.start(m_spawn_frequency);
}



Spawn_Controller::Spawn_Controller()
{
    LST::Message_Translator::instance().subscribe<Message__On_Entity_Death>([this](Message__On_Entity_Death& _msg)
    {
        if(_msg.almost_dead_entity != m_player_object)
            return;

        m_player_object = nullptr;
    });
}

Spawn_Controller::~Spawn_Controller()
{

}



Coordinates Spawn_Controller::M_find_most_compatible_cell(const Coordinates& _offset) const
{
    if(!_offset.valid())
        return {};

    constexpr unsigned int Max_Offset = 5;
    constexpr unsigned int Negative_Y_Difference_Multiplier = 3;

    Coordinates result;
    int min_y_difference = m_terrain->grid().size_y() * Negative_Y_Difference_Multiplier;

    auto consider_cell = [&](const Coordinates& _coord)
    {
        int y_diff = (int)_coord.y() - (int)_offset.y();
        if(y_diff < 0)
            y_diff *= Negative_Y_Difference_Multiplier;

        if(abs(y_diff) >= abs(min_y_difference))
            return;

        min_y_difference = y_diff;
        result = _coord;
    };


    unsigned int offset_x = 0;
    if(Max_Offset < _offset.x())
        offset_x = _offset.x() - Max_Offset;
    unsigned int offset_z = 0;
    if(Max_Offset < _offset.z())
        offset_z = _offset.z() - Max_Offset;

    unsigned int last_x = _offset.x() + Max_Offset;
    if(m_terrain->grid().size_x() <= last_x)
        last_x = m_terrain->grid().size_x() - 1;
    unsigned int last_z = _offset.z() + Max_Offset;
    if(m_terrain->grid().size_z() <= last_z)
        last_z = m_terrain->grid().size_z() - 1;

    for(unsigned int x = offset_x; x <= last_x; ++x)
    {
        for(unsigned int z = offset_z; z <= last_z; ++z)
        {
            Coordinates leveled_coord = {x, _offset.y(), z};
            Coordinates grounded_coord = m_terrain->find_grounded_cell(leveled_coord);
            if(!grounded_coord.valid())
                continue;

            consider_cell(grounded_coord);
        }
    }

    return result;
}



glm::vec3 Spawn_Controller::find_available_position(const glm::vec3& _offset) const
{
    L_ASSERT(m_terrain);
    L_ASSERT(m_player_object);

    constexpr glm::vec3 Rotation_Axis = {0.0f, 1.0f, 0.0f};
    constexpr glm::vec3 Default_Vector = {1.0f, 0.0f, 0.0f};

    Coordinates offset_cell = m_terrain->calculate_grid_cell(_offset);
    if(!offset_cell.valid())
        return {0.0f, 0.0f, 0.0f};

    float random_distance = LEti::Math::random_number_float(m_min_distance_from_player, m_max_distance_from_player);
    float ranfom_angle = LEti::Math::random_number_float(0.0f, LEti::Math::DOUBLE_PI);
    glm::vec3 random_position = LEti::Math::rotate_vector(Default_Vector * random_distance, Rotation_Axis, ranfom_angle);
    random_position += _offset;
    Coordinates random_cell = m_terrain->calculate_grid_cell(random_position);

    Coordinates grounded_cell = M_find_most_compatible_cell(random_cell);
    if(grounded_cell.valid())
        return m_terrain->calculate_real_coord(grounded_cell);

    return {0.0f, 0.0f, 0.0f};
}


void Spawn_Controller::spawn_forcefully(bool _only_one)
{
    L_ASSERT(m_objects_controller);

    const LEti::Object_Stub* random_stub = m_weighted_objects.get_random();
    glm::vec3 available_position = find_available_position( m_player_object->current_state().position() );

    constexpr glm::vec3 Rotation_Axis = {0.0f, 1.0f, 0.0f};
    constexpr glm::vec3 Default_Offset = {0.5f, 0.0f, 0.0f};

    unsigned int group_size = 1;
    if(!_only_one)
        group_size = LEti::Math::random_number(m_min_entities_per_group, m_max_entities_per_group);

    float angle_offset_per_entity = LEti::Math::DOUBLE_PI / (float)group_size;

    for(unsigned int i = 0; i < group_size; ++i)
    {
        LEti::Object* object = LEti::Object_Stub::construct_from(random_stub);

        glm::vec3 offset = LEti::Math::rotate_vector(Default_Offset, Rotation_Axis, angle_offset_per_entity * (float)i);
        offset += available_position;

        object->current_state().set_position(offset);

        object->update(0.0f);
        object->update_previous_state();

        m_objects_controller->add_object(object);
    }
}

void Spawn_Controller::update(float _dt)
{
    if(!m_player_object)
        return;

    m_spawn_delay_timer.update(_dt);
    if(m_spawn_delay_timer.is_active())
        return;

    if(!m_autospawn_enabled)
        return;

    m_spawn_delay_timer.start(m_spawn_frequency);

    spawn_forcefully(false);
}
