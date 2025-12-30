#pragma once

#include <Data_Structures/Vector.h>
#include <Data_Structures/Map.h>
#include <Data_Structures/AVL_Tree.h>

#include <vec3.hpp>

#include <Object.h>

#include <Terrain/Terrain.h>
#include <Terrain/Terrain_Utility.h>


namespace Shardis
{

    class Terrain_Generator
    {
    private:
        using Points = LDS::Vector<glm::vec3>;
        using Points_Cache = LDS::Map<Vec_Storage, unsigned int>;

    private:
        LEti::Object_Stub* m_terrain_object_stub = nullptr;

        long unsigned int m_seed = 0;

        unsigned int m_size_x = 0;
        unsigned int m_size_y = 0;
        unsigned int m_size_z = 0;

        Coordinates m_main_room_min_size;
        Coordinates m_main_room_max_size;
        unsigned int m_secondary_features_amount = 0;
        unsigned int m_secondary_feature_min_size = 0;
        unsigned int m_secondary_feature_max_size = 0;

        float m_cell_size = 1.0f;
        float m_cell_size_halved = m_cell_size * 0.5f;

        float m_min_random_point_stride = 1.0f;
        float m_max_random_point_stride = 3.0f;

    public:
        Terrain_Generator();
        ~Terrain_Generator();

    public:
        inline void set_terrain_object_stub(LEti::Object_Stub* _ptr) { m_terrain_object_stub = _ptr; }
        inline void set_seed(long unsigned int _value) { m_seed = _value; }
        inline void set_size_x(unsigned int _value) { m_size_x = _value; }
        inline void set_size_y(unsigned int _value) { m_size_y = _value; }
        inline void set_size_z(unsigned int _value) { m_size_z = _value; }

        void set_main_room_min_size(const Coordinates& _value) { m_main_room_min_size = _value; }
        void set_main_room_max_size(const Coordinates& _value) { m_main_room_max_size = _value; }
        void set_secondary_features_amount(unsigned int _value) { m_secondary_features_amount = _value; }
        void set_secondary_feature_min_size(unsigned int _value) { m_secondary_feature_min_size = _value; }
        void set_secondary_feature_max_size(unsigned int _value) { m_secondary_feature_max_size = _value; }

        inline void set_cell_size(float _value) { m_cell_size = _value; m_cell_size_halved = _value * 0.5f; }
        inline void set_min_random_point_stride(float _value) { m_min_random_point_stride = _value; }
        inline void set_max_random_point_stride(float _value) { m_max_random_point_stride = _value; }

    private:
        void M_apply_rectangle(Terrain& _terrain, unsigned int _x, unsigned int _y, unsigned int _z, unsigned int _size_x, unsigned int _size_y, unsigned int _size_z, Voxel_Type _voxel_type) const;
        void M_apply_ellipsoid(Terrain& _terrain, unsigned int _x, unsigned int _y, unsigned int _z, unsigned int _radius_x, unsigned int _radius_y, unsigned int _radius_z, Voxel_Type _voxel_type) const;
        void M_apply_flat_ellipse(Terrain& _terrain, unsigned int _x, unsigned int _y, unsigned int _z, unsigned int _radius_x, unsigned int _size_y, unsigned int _radius_z, Voxel_Type _voxel_type) const;
        void M_apply_trajectory(Terrain& _terrain, const LDS::List<glm::vec3>& _trajectory, const LST::Function<void(unsigned int, unsigned int, unsigned int)>& _apply_how) const;

        Coordinates M_construct_random_coords(const Coordinates& _main_room_center) const;
        void M_apply_random_shape(Terrain& _terrain, const Coordinates& _at) const;
        Coordinates M_construct_rooms(Terrain& _terrain) const;

        void M_apply_shape_to_grid(Terrain& _terrain) const;

        unsigned int M_get_or_construct_vector_id(Points& _points, Points_Cache& _cache, const glm::vec3& _vec) const;

        bool M_grid_cell_is_border_left(const Terrain_Grid& _grid, unsigned int _x, unsigned int _y, unsigned int _z) const;
        bool M_grid_cell_is_border_right(const Terrain_Grid& _grid, unsigned int _x, unsigned int _y, unsigned int _z) const;
        bool M_grid_cell_is_border_below(const Terrain_Grid& _grid, unsigned int _x, unsigned int _y, unsigned int _z) const;
        bool M_grid_cell_is_border_above(const Terrain_Grid& _grid, unsigned int _x, unsigned int _y, unsigned int _z) const;
        bool M_grid_cell_is_border_behind(const Terrain_Grid& _grid, unsigned int _x, unsigned int _y, unsigned int _z) const;
        bool M_grid_cell_is_border_infront(const Terrain_Grid& _grid, unsigned int _x, unsigned int _y, unsigned int _z) const;

        void M_append_points(Chunk_Data& _chunk, Points& _points, Points_Cache& _cache, const glm::vec3& _point_1, const glm::vec3& _point_2, const glm::vec3& _point_3, const glm::vec3& _point_4) const;
        void M_append_points_left(Chunk_Data& _chunk, Points& _points, Points_Cache& _cache, const glm::vec3& _center) const;
        void M_append_points_right(Chunk_Data& _chunk, Points& _points, Points_Cache& _cache, const glm::vec3& _center) const;
        void M_append_points_below(Chunk_Data& _chunk, Points& _points, Points_Cache& _cache, const glm::vec3& _center) const;
        void M_append_points_above(Chunk_Data& _chunk, Points& _points, Points_Cache& _cache, const glm::vec3& _center) const;
        void M_append_points_behind(Chunk_Data& _chunk, Points& _points, Points_Cache& _cache, const glm::vec3& _center) const;
        void M_append_points_infront(Chunk_Data& _chunk, Points& _points, Points_Cache& _cache, const glm::vec3& _center) const;
        void M_append_points_if_needed(Chunk_Data& _chunk, Points& _points, Points_Cache& _cache, const Terrain& _terrain, unsigned int _x, unsigned int _y, unsigned int _z) const;

        unsigned int M_calculate_chunks_amount_per_axis(unsigned int _size_per_axis) const;
        unsigned int M_calculate_chunks_amount() const;

        Chunk_Data M_extract_chunk_mesh(Points& _points, Points_Cache& _cache, const Terrain& _terrain, unsigned int _chunk_x, unsigned int _chunk_y, unsigned int _chunk_z) const;

        const Chunk_Data* M_find_chunk(const LDS::Vector<Chunk_Data>& _chunks, const Coordinates& _coordinates) const;
        LDS::Vector<const Chunk_Data*> M_find_neighbour_chunks(const LDS::Vector<Chunk_Data>& _chunks, const Chunk_Data& _neighbour_to) const;
        void M_append_mesh_to_average(const Points& _points,
                                      const Chunk_Data& _chunk,
                                      const glm::vec3& _reference_point,
                                      Used_Points_Cache& _used_points_cache,
                                      glm::vec3& _result_average,
                                      unsigned int& _found_amount) const;
        void M_smooth_chunk(Points& _new_points,
                            const Points& _old_points,
                            LDS::Vector<bool>& _smoothed,
                            Used_Points_Cache& _used_points_cache,
                            const LDS::Vector<Chunk_Data>& _chunks,
                            const Chunk_Data& _chunk) const;
        void M_smooth_points(const LDS::Vector<Chunk_Data>& _chunks, Points& _points) const;

        void M_randomize_points(Points& _points) const;

        LDS::Vector<float> M_construct_geometry_data(const Points_Ids& _mesh_ids, const Points& _points) const;
        LDS::Vector<float> M_construct_texture_data(const Points_Ids& _mesh_ids, const Points& _points) const;
        LDS::Vector<float> M_construct_normals_data(const Points_Ids& _mesh_ids, const Points& _points) const;

    public:
        Terrain* generate();
        LEti::Object* construct_chunk_object(const Terrain& _terrain, const Coordinates& _coordinates) const;

    };

}
