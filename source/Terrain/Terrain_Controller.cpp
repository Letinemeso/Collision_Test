#include <Terrain/Terrain_Controller.h>

#include <Tools/Objects_Controller/Objects_Controller_Extension__Removal.h>

using namespace Shardis;

namespace Shardis
{
    constexpr unsigned int Loaded_Chunks_Per_Axis = 6;
    constexpr unsigned int Loaded_Chunks_Per_Axis_Halved = Loaded_Chunks_Per_Axis / 2;
    constexpr unsigned int Loaded_Chunks_Amount = Loaded_Chunks_Per_Axis * Loaded_Chunks_Per_Axis * Loaded_Chunks_Per_Axis;

    constexpr float Operations_Delay = 0.01f;
    constexpr unsigned int Operations_Per_Pass = 5;
    constexpr unsigned int Max_Operations_In_Queue = 300;
}



Terrain_Controller::Terrain_Controller()
{
    LST::Message_Translator::instance().subscribe<Message__On_Entity_Death>([this](Message__On_Entity_Death& _msg)
    {
        if(_msg.almost_dead_entity ==  m_player_object)
            m_player_object = nullptr;
    });
}

Terrain_Controller::~Terrain_Controller()
{
    delete m_terrain;
}



void Terrain_Controller::init(const LV::Object_Constructor& _object_constructor)
{
    LV::MDL_Reader reader;
    reader.parse_file("../Resources/Terrain/Terrain_Object");

    LEti::Object_Stub* terrain_stub = (LEti::Object_Stub*)_object_constructor.construct(reader.get_stub("Terrain_Object"));
    L_ASSERT(LV::cast_variable<LEti::Object_Stub>(terrain_stub));

    m_terrain_generator.set_terrain_object_stub(terrain_stub);

    // m_terrain_generator.set_size_x(500);
    // m_terrain_generator.set_size_y(170);
    // m_terrain_generator.set_size_z(500);

    // m_terrain_generator.set_main_room_min_size({175, 70, 175});
    // m_terrain_generator.set_main_room_max_size({200, 80, 200});
    // m_terrain_generator.set_secondary_features_amount(1000);
    // m_terrain_generator.set_secondary_feature_min_size(10);
    // m_terrain_generator.set_secondary_feature_max_size(40);

    m_terrain_generator.set_size_x(50);
    m_terrain_generator.set_size_y(50);
    m_terrain_generator.set_size_z(50);

    m_terrain_generator.set_main_room_min_size({20, 23, 20});
    m_terrain_generator.set_main_room_max_size({21, 24, 21});
    m_terrain_generator.set_secondary_features_amount(10);
    m_terrain_generator.set_secondary_feature_min_size(10);
    m_terrain_generator.set_secondary_feature_max_size(20);

    m_terrain_generator.set_cell_size(2.5f);

    m_terrain_generator.set_min_random_point_stride(0.15f);
    m_terrain_generator.set_max_random_point_stride(0.5f);
}

void Terrain_Controller::generate_terrain()
{
    delete_terrain();

    // long unsigned int seed = time(nullptr);
    long unsigned int seed = 1765990095;
    m_terrain_generator.set_seed(seed);

    m_terrain = m_terrain_generator.generate();
}

void Terrain_Controller::delete_terrain()
{
    delete m_terrain;
    m_terrain = nullptr;
}



void Terrain_Controller::M_find_player_if_needed()
{
    if(m_player_object)
        return;

    Message__Find_Player msg;
    LST::Message_Translator::instance().translate(msg);

    if(!msg.player_object)
        return;

    m_player_object = msg.player_object;
}


LDS::Vector<Coordinates> Terrain_Controller::M_calculate_chunks_around_center(const Coordinates& _center)
{
    LDS::Vector<Coordinates> result(Loaded_Chunks_Amount);

    Coordinates map_size = m_terrain->calculate_size_in_chunks();

    Coordinates area_offset = {0, 0, 0};
    Coordinates area_limit = map_size;
    for(unsigned int i = 0; i < 3; ++i)
        area_limit[i] -= 1;

    for(unsigned int i = 0; i < 3; ++i)
    {
        if(_center[i] > Loaded_Chunks_Per_Axis_Halved)
            area_offset[i] = _center[i] - Loaded_Chunks_Per_Axis_Halved;

        if(_center[i] + Loaded_Chunks_Per_Axis_Halved < map_size[i])
            area_limit[i] = _center[i] + Loaded_Chunks_Per_Axis_Halved;
    }

    for(unsigned int x = area_offset.x(); x <= area_limit.x(); ++x)
    {
        for(unsigned int y = area_offset.y(); y <= area_limit.y(); ++y)
        {
            for(unsigned int z = area_offset.z(); z <= area_limit.z(); ++z)
            {
                result.push({x, y, z});
            }
        }
    }

    return result;
}

LDS::Vector<Coordinates> Terrain_Controller::M_find_missing_chunks(const LDS::Vector<Coordinates>& _expected_chunks)
{
    LDS::Vector<Coordinates> result;

    Terrain::Terrain_Objects& loaded_chunks = m_terrain->terrain_objects();
    if(loaded_chunks.size() == 0)
        return _expected_chunks;

    LDS::Vector<bool> found(_expected_chunks.size(), false);

    for(Terrain::Terrain_Objects::Iterator it = loaded_chunks.begin(); !it.end_reached(); ++it)
    {
        const Coordinates& coordinates = it->coordinates;

        LDS::Vector<Coordinates>::Const_Iterator maybe_found_chunk = _expected_chunks.find(coordinates);
        if(maybe_found_chunk.is_ok())
            found[maybe_found_chunk.index()] = true;
    }

    for(unsigned int i = 0; i < found.size(); ++i)
    {
        if(!found[i])
            result.push(_expected_chunks[i]);
    }

    return result;
}

void Terrain_Controller::M_remove_unexpected_chunks(const LDS::Vector<Coordinates>& _expected_chunks)
{
    L_ASSERT(m_objects_controller);

    Terrain::Terrain_Objects& loaded_chunks = m_terrain->terrain_objects();
    if(loaded_chunks.size() == 0)
        return;

    Terrain::Terrain_Objects::Iterator it = loaded_chunks.begin();
    while(!it.end_reached())
    {
        const Coordinates& coordinates = it->coordinates;

        if(_expected_chunks.find(coordinates).is_ok())
        {
            ++it;
            continue;
        }

        LEti::Object* object = it->object;

        LMD::Message__Request_Object_Deletion msg;
        msg.object = object;
        LST::Message_Translator::instance().translate(msg);

        Chunk_Load_Queue::Iterator maybe_queued_chunk_it = m_load_queue.find(coordinates);
        if(maybe_queued_chunk_it.is_ok())
            m_load_queue.erase(maybe_queued_chunk_it);

        it = loaded_chunks.erase_and_iterate_forward(it);
    }
}

void Terrain_Controller::M_load_missing_expected_chunks(const LDS::Vector<Coordinates>& _expected_chunks)
{
    LDS::Vector<Coordinates> missing_chunks = M_find_missing_chunks(_expected_chunks);

    for(unsigned int i = 0; i < missing_chunks.size(); ++i)
    {
        const Coordinates& coordinates = missing_chunks[i];
        if(!m_load_queue.find(coordinates).is_ok())
            m_load_queue.push_back(coordinates);
    }
}

void Terrain_Controller::M_update_loaded_chunks(const Coordinates& _center)
{
    LDS::Vector<Coordinates> expected_chunks = M_calculate_chunks_around_center(_center);

    M_remove_unexpected_chunks(expected_chunks);
    M_load_missing_expected_chunks(expected_chunks);
}


void Terrain_Controller::M_load_chunk(const Coordinates& _coordinate)
{
    if(m_terrain->terrain_object_exists(_coordinate))
        return;

    LEti::Object* object = m_terrain_generator.construct_chunk_object(*m_terrain, _coordinate);
    if(!object)
        return;

    Terrain::Terrain_Objects& loaded_chunks = m_terrain->terrain_objects();
    loaded_chunks.push_back({_coordinate, object});
    m_objects_controller->add_object(object);
}


void Terrain_Controller::M_process_queue(float _dt)
{
    m_load_queue_delay.update(_dt);
    if(m_load_queue_delay.is_active())
        return;

    if(m_load_queue.size() == 0)
        return;

    m_load_queue_delay.start(Operations_Delay);

    for(unsigned int i = 0; i < Operations_Per_Pass && m_load_queue.size() > 0; ++i)
    {
        const Coordinates& coordinates = m_load_queue.front();
        M_load_chunk(coordinates);
        m_load_queue.pop_front();
    }
}



void Terrain_Controller::process_whole_queue(unsigned int _operations_to_process)
{
    for(unsigned int i = 0; i < _operations_to_process && m_load_queue.size() > 0; ++i)
    {
        const Coordinates& coordinates = m_load_queue.front();
        M_load_chunk(coordinates);
        m_load_queue.pop_front();
    }
}

void Terrain_Controller::update(float _dt)
{
    M_find_player_if_needed();

    if(!m_player_object)
        return;

    m_load_queue_delay.update(_dt);
    M_process_queue(_dt);

    Coordinates center = m_terrain->calculate_chunk_coordinates(m_player_object->current_state().position());
    if(!center.valid())
        return;

    if(center == m_current_center && m_current_center.valid())
        return;

    M_update_loaded_chunks(center);
    if(!m_current_center.valid())
        process_whole_queue();
    else if(m_load_queue.size() > Max_Operations_In_Queue)
        process_whole_queue();

    m_current_center = center;
}
