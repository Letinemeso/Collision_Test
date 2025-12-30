#pragma once

#include <Data_Structures/Vector.h>
#include <Data_Structures/Weighted_Container.h>
#include <Stuff/Timer.h>

#include <Variable_Base.h>
#include <Builder_Stub.h>

#include <Object.h>

#include <Tools/Objects_Storage.h>
#include <Tools/Objects_Controller/Objects_Controller.h>

#include <Terrain/Terrain.h>


namespace Shardis
{

    class Spawn_Controller : public LV::Variable_Base
    {
    public:
        INIT_VARIABLE(Shardis::Spawn_Controller, LV::Variable_Base)

        INIT_FIELDS
        ADD_FIELD_RENAMED(LDS::Vector<std::string>, "entity_ids", m_entity_ids)
        ADD_FIELD_RENAMED(LDS::Vector<unsigned int>, "weights", m_weights)
        ADD_FIELD_RENAMED(float, "min_distance_from_player", m_min_distance_from_player)
        ADD_FIELD_RENAMED(float, "max_distance_from_player", m_max_distance_from_player)
        ADD_FIELD_RENAMED(float, "spawn_frequency", m_spawn_frequency)
        ADD_FIELD_RENAMED(unsigned int, "min_entities_per_group", m_min_entities_per_group)
        ADD_FIELD_RENAMED(unsigned int, "max_entities_per_group", m_max_entities_per_group)
        FIELDS_END

        OVERRIDE_ON_VALUES_ASSIGNED

    private:
        const LMD::Objects_Storage* m_entity_provider = nullptr;
        LMD::Objects_Controller* m_objects_controller = nullptr;
        const Terrain* m_terrain = nullptr;
        const LEti::Object* m_player_object = nullptr;

    private:
        LDS::Vector<std::string> m_entity_ids;
        LDS::Vector<unsigned int> m_weights;

        float m_min_distance_from_player = 0;
        float m_max_distance_from_player = 0;

        float m_spawn_frequency = 0.0f;

        unsigned int m_min_entities_per_group = 0;
        unsigned int m_max_entities_per_group = 0;

    private:
        LDS::Weighted_Container<const LEti::Object_Stub*> m_weighted_objects;

        LST::Timer m_spawn_delay_timer;
        bool m_autospawn_enabled = true;

    public:
        Spawn_Controller();
        ~Spawn_Controller();

    public:
        inline void inject_entity_provider(const LMD::Objects_Storage* _ptr) { m_entity_provider = _ptr; }
        inline void inject_objects_controller(LMD::Objects_Controller* _ptr) { m_objects_controller = _ptr; }
        inline void inject_terrain(const Terrain* _ptr) { m_terrain = _ptr; }
        inline void inject_player_object(const LEti::Object* _ptr) { m_player_object = _ptr; }

        inline void enable_autospawn(bool _value) { m_autospawn_enabled = _value; }

    private:
        Coordinates M_find_most_compatible_cell(const Coordinates& _offset) const;

    public:
        glm::vec3 find_available_position(const glm::vec3& _offset) const;

        void spawn_forcefully(bool _only_one = false);
        void update(float _dt);

    };


}
