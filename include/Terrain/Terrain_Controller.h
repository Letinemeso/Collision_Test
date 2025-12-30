#pragma once

#include <Stuff/Message_Translator.h>
#include <Stuff/Timer.h>

#include <Object_Constructor.h>

#include <Object.h>

#include <Tools/Objects_Controller/Objects_Controller.h>

#include <Terrain/Terrain_Utility.h>
#include <Terrain/Terrain_Generator.h>
#include <Terrain/Terrain.h>

#include <Messages/Messages.h>


namespace Shardis
{

    class Terrain_Controller
    {
    private:
        LMD::Objects_Controller* m_objects_controller = nullptr;

    private:
        Terrain_Generator m_terrain_generator;
        Terrain* m_terrain = nullptr;

        const LEti::Object* m_player_object = nullptr;

        Coordinates m_current_center;
        LDS::Vector<Coordinates> m_loaded_chunks;

        using Chunk_Load_Queue = LDS::List<Coordinates>;
        Chunk_Load_Queue m_load_queue;
        LST::Timer m_load_queue_delay;

    public:
        Terrain_Controller();
        ~Terrain_Controller();

    public:
        inline void inject_objects_controller(LMD::Objects_Controller* _ptr) { m_objects_controller = _ptr; }

        inline Terrain* terrain() { return m_terrain; }

    public:
        void init(const LV::Object_Constructor& _object_constructor);
        void generate_terrain();
        void delete_terrain();

    private:
        void M_find_player_if_needed();

        LDS::Vector<Coordinates> M_calculate_chunks_around_center(const Coordinates& _center);
        LDS::Vector<Coordinates> M_find_missing_chunks(const LDS::Vector<Coordinates>& _expected_chunks);
        void M_remove_unexpected_chunks(const LDS::Vector<Coordinates>& _expected_chunks);
        void M_load_missing_expected_chunks(const LDS::Vector<Coordinates>& _expected_chunks);
        void M_update_loaded_chunks(const Coordinates& _center);

        void M_load_chunk(const Coordinates& _coordinate);

        void M_process_queue(float _dt);

    public:
        void process_whole_queue(unsigned int _operations_to_process = 0xFFFFFFFF);
        void update(float _dt);

    };

}
