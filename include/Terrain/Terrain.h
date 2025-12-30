#pragma once

#include <vec3.hpp>

#include <Object.h>

#include <Terrain/Terrain_Grid.h>


namespace Shardis
{

    constexpr unsigned int Chunk_Size = 10;


    class Terrain
    {
    public:
        struct Chunk
        {
            Coordinates coordinates;
            LEti::Object* object = nullptr;
        };

        using Terrain_Objects = LDS::List<Chunk>;

    private:
        Terrain_Grid m_grid;
        float m_cell_size = 0.0f;

        Points m_points;
        Chunks_Data m_chunks_data;

        Terrain_Objects m_terrain_objects;

        Coordinates m_start_point;

    public:
        Terrain();
        ~Terrain();

    public:
        inline void set_cell_size(float _value) { m_cell_size = _value; }
        inline void set_terrain_objects(Terrain_Objects&& _objects) { clear_terrain_objects(); m_terrain_objects = (Terrain_Objects&&)_objects; }
        inline void set_start_point(const Coordinates& _value) { m_start_point = _value; }
        inline void set_points(Points&& _points) { m_points = (Points&&)_points; }
        inline void set_chunks_data(Chunks_Data&& _chunks) { m_chunks_data = (Chunks_Data&&)_chunks; }

        inline Terrain_Grid& grid() { return m_grid; }
        inline const Terrain_Grid& grid() const { return m_grid; }
        inline float cell_size() const { return m_cell_size; }
        inline Terrain_Objects& terrain_objects() { return m_terrain_objects; }
        inline const Terrain_Objects& terrain_objects() const { return m_terrain_objects; }
        inline const Points& points() const { return m_points; }
        inline const Chunks_Data& chunks() const { return m_chunks_data; }

    public:
        void clear_terrain_objects();

        bool terrain_object_exists(const Coordinates& _coordinates) const;

        Coordinates calculate_grid_cell(const glm::vec3& _real_coord) const;
        glm::vec3 calculate_real_coord(const Coordinates& _grid_cell) const;
        glm::vec3 calculate_map_size() const;
        glm::vec3 get_closest_grounded_point(const glm::vec3& _closest_) const;
        Coordinates find_grounded_cell(const Coordinates& _cell) const;
        glm::vec3 get_start_point() const;

        Coordinates calculate_size_in_chunks() const;
        Coordinates calculate_chunk_coordinates(const Coordinates& _grid_cell) const;
        Coordinates calculate_chunk_coordinates(const glm::vec3& _real_coord) const;

    };

}
