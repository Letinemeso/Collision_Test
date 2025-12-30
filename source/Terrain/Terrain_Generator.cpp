#include <Terrain/Terrain_Generator.h>

#include <Stuff/Thread_Pool.h>

#include <MDL_Reader.h>

#include <Math_Stuff.h>

#include <Draw_Modules/Draw_Module.h>
#include <Components/Graphics_Component__Default.h>

#include <Modules/Physics_Module__Mesh.h>

#include <Tools/Spline.h>

using namespace Shardis;


Terrain_Generator::Terrain_Generator()
{

}

Terrain_Generator::~Terrain_Generator()
{
    delete m_terrain_object_stub;
}



void Terrain_Generator::M_apply_rectangle(Terrain& _terrain,
                                                       unsigned int _x, unsigned int _y, unsigned int _z,
                                                       unsigned int _size_x, unsigned int _size_y, unsigned int _size_z,
                                                       Voxel_Type _voxel_type) const
{
    unsigned int size_halved_x = _size_x / 2;
    unsigned int size_halved_y = _size_y / 2;
    unsigned int size_halved_z = _size_z / 2;

    unsigned int offset_x = 0;
    unsigned int offset_y = 0;
    unsigned int offset_z = 0;

    if(_x > size_halved_x)
        offset_x = _x - size_halved_x;
    if(_y > size_halved_y)
        offset_y = _y - size_halved_y;
    if(_z > size_halved_z)
        offset_z = _z - size_halved_z;

    unsigned int target_x = offset_x + _size_x;
    unsigned int target_y = offset_y + _size_y;
    unsigned int target_z = offset_z + _size_z;

    if(target_x >= m_size_x)
        target_x = m_size_x - 1;
    if(target_y >= m_size_y)
        target_y = m_size_y - 1;
    if(target_z >= m_size_z)
        target_z = m_size_z - 1;

    Terrain_Grid& grid = _terrain.grid();

    for(unsigned int x = offset_x; x <= target_x; ++x)
    {
        for(unsigned int y = offset_y; y <= target_y; ++y)
        {
            for(unsigned int z = offset_z; z <= target_z; ++z)
            {
                grid.set_element_at(_voxel_type, x, y, z);
            }
        }
    }
}

void Terrain_Generator::M_apply_ellipsoid(Terrain& _terrain,
                                                       unsigned int _x, unsigned int _y, unsigned int _z,
                                                       unsigned int _radius_x, unsigned int _radius_y, unsigned int _radius_z,
                                                       Voxel_Type _voxel_type) const
{
    unsigned int size_halved_x = _radius_x;
    unsigned int size_halved_y = _radius_y;
    unsigned int size_halved_z = _radius_z;

    unsigned int offset_x = 0;
    unsigned int offset_y = 0;
    unsigned int offset_z = 0;

    if(_x > size_halved_x)
        offset_x = _x - size_halved_x;
    if(_y > size_halved_y)
        offset_y = _y - size_halved_y;
    if(_z > size_halved_z)
        offset_z = _z - size_halved_z;

    unsigned int target_x = offset_x + (_radius_x * 2);
    unsigned int target_y = offset_y + (_radius_y * 2);
    unsigned int target_z = offset_z + (_radius_z * 2);

    if(target_x >= m_size_x)
        target_x = m_size_x - 1;
    if(target_y >= m_size_y)
        target_y = m_size_y - 1;
    if(target_z >= m_size_z)
        target_z = m_size_z - 1;

    glm::vec3 center = { (float)_x, (float)_y, (float)_z };
    glm::vec3 radiuses_squared = { (float)(_radius_x * _radius_x), (float)(_radius_y * _radius_y), (float)(_radius_z * _radius_z) };

    auto point_inside_ellipsoid = [&center, &radiuses_squared](const glm::vec3& _point)->bool
    {
        glm::vec3 point_based_squared = _point - center;
        point_based_squared *= point_based_squared;

        float distance = (point_based_squared.x / radiuses_squared.x) + (point_based_squared.y / radiuses_squared.y) + (point_based_squared.z / radiuses_squared.z);
        return distance < 1.0;
    };

    Terrain_Grid& grid = _terrain.grid();

    for(unsigned int x = offset_x; x <= target_x; ++x)
    {
        for(unsigned int y = offset_y; y <= target_y; ++y)
        {
            for(unsigned int z = offset_z; z <= target_z; ++z)
            {
                glm::vec3 voxel_point = {(float)x, (float)y, (float)z};
                if(!point_inside_ellipsoid(voxel_point))
                    continue;

                grid.set_element_at(_voxel_type, x, y, z);
            }
        }
    }
}

void Terrain_Generator::M_apply_flat_ellipse(Terrain& _terrain,
                                             unsigned int _x, unsigned int _y, unsigned int _z,
                                             unsigned int _radius_x, unsigned int _size_y, unsigned int _radius_z,
                                             Voxel_Type _voxel_type) const
{
    unsigned int size_halved_x = _radius_x;
    unsigned int size_halved_y = _size_y / 2;
    unsigned int size_halved_z = _radius_z;

    unsigned int offset_x = 0;
    unsigned int offset_y = 0;
    unsigned int offset_z = 0;

    if(_x > size_halved_x)
        offset_x = _x - size_halved_x;
    if(_y > size_halved_y)
        offset_y = _y - size_halved_y;
    if(_z > size_halved_z)
        offset_z = _z - size_halved_z;

    unsigned int target_x = offset_x + (_radius_x * 2);
    unsigned int target_y = offset_y + _size_y;
    unsigned int target_z = offset_z + (_radius_z * 2);

    if(target_x >= m_size_x)
        target_x = m_size_x - 1;
    if(target_y >= m_size_y)
        target_y = m_size_y - 1;
    if(target_z >= m_size_z)
        target_z = m_size_z - 1;

    glm::vec2 center = { (float)_x, (float)_z };
    glm::vec2 radiuses_squared = { (float)(_radius_x * _radius_x), (float)(_radius_z * _radius_z) };

    auto point_inside_ellipsoid = [&center, &radiuses_squared](const glm::vec2& _point)->bool
    {
        glm::vec2 point_based_squared = _point - center;
        point_based_squared *= point_based_squared;

        float distance = (point_based_squared.x / radiuses_squared.x) + (point_based_squared.y / radiuses_squared.y);
        return distance < 1.0;
    };

    Terrain_Grid& grid = _terrain.grid();

    for(unsigned int x = offset_x; x <= target_x; ++x)
    {
        for(unsigned int y = offset_y; y <= target_y; ++y)
        {
            for(unsigned int z = offset_z; z <= target_z; ++z)
            {
                glm::vec2 voxel_point = {(float)x, (float)z};
                if(!point_inside_ellipsoid(voxel_point))
                    continue;

                grid.set_element_at(_voxel_type, x, y, z);
            }
        }
    }
}

void Terrain_Generator::M_apply_trajectory(Terrain& _terrain, const LDS::List<glm::vec3>& _trajectory, const LST::Function<void(unsigned int, unsigned int, unsigned int)>& _apply_how) const
{
    for(LDS::List<glm::vec3>::Const_Iterator it = _trajectory.begin(); !it.end_reached(); ++it)
    {
        const glm::vec3& point = *it;

        Coordinates voxel = _terrain.calculate_grid_cell(point);
        if(!voxel.valid())
            continue;

        _apply_how(voxel.x(), voxel.y(), voxel.z());
    }
}


Coordinates Terrain_Generator::M_construct_random_coords(const Coordinates& _main_room_center) const
{
    Coordinates result;

    for(unsigned int i = 0; i < 3; i += 2)
    {
        result[i] = LEti::Math::random_number(m_main_room_min_size[i] / 10, m_main_room_min_size[i]);
        if( LEti::Math::random_bool() )
            result[i] += _main_room_center[i];
        else
            result[i] = _main_room_center[i] - result[i];
    }

    result.y() = _main_room_center.y();

    return result;
}

void Terrain_Generator::M_apply_random_shape(Terrain& _terrain, const Coordinates& _at) const
{
    Coordinates random_size;
    for(unsigned int i = 0; i < 3; ++i)
        random_size[i] = LEti::Math::random_number(m_secondary_feature_min_size, m_secondary_feature_max_size);

    Voxel_Type voxel_type;
    if(LEti::Math::random_bool())
        voxel_type = Voxel_Type::Empty;
    else
        voxel_type = Voxel_Type::Filled;

    constexpr unsigned int Ellipsoid_Height_Divider = 6;

    unsigned int random_shape = LEti::Math::random_number(0, 2);
    if(random_shape == 0)
        M_apply_ellipsoid(_terrain, _at.x(), _at.y(), _at.z(), random_size.x() / 2, random_size.y() / Ellipsoid_Height_Divider, random_size.z() / 2, voxel_type);
    if(random_shape == 1)
        M_apply_flat_ellipse(_terrain, _at.x(), _at.y(), _at.z(), random_size.x() / 2, random_size.y(), random_size.z() / 2, voxel_type);
}

Coordinates Terrain_Generator::M_construct_rooms(Terrain& _terrain) const
{
    Coordinates center = { m_size_x / 2, m_size_y / 2, m_size_z / 2 };
    Coordinates main_room_size;
    for(unsigned int i = 0; i < 3; ++i)
        main_room_size[i] = LEti::Math::random_number(m_main_room_min_size[i], m_main_room_max_size[i]);

    M_apply_flat_ellipse(_terrain, center.x(), center.y(), center.z(), main_room_size.x(), main_room_size.y(), main_room_size.z(), Voxel_Type::Empty);

    for(unsigned int i = 0; i < m_secondary_features_amount; ++i)
    {
        Coordinates random_coord = M_construct_random_coords(center);
        random_coord = _terrain.find_grounded_cell(random_coord);

        M_apply_random_shape(_terrain, random_coord);
    }

    return center;
}


void Terrain_Generator::M_apply_shape_to_grid(Terrain& _terrain) const
{
    // M_apply_empty_space__rectangle(_terrain, m_size_x / 2, m_size_y / 2, m_size_z / 2, m_size_x - 6, m_size_y - 6, m_size_z - 6);
    // M_apply_empty_space__ellipsoid(_terrain, m_size_x / 2, m_size_y / 2, m_size_z / 2, 20, 20, 20);
    // _terrain.set_start_point({m_size_x / 2, m_size_y / 2, m_size_z / 2});

    Coordinates start = M_construct_rooms(_terrain);
    _terrain.set_start_point(start);
}


unsigned int Terrain_Generator::M_get_or_construct_vector_id(Points& _points, Points_Cache& _cache, const glm::vec3& _vec) const
{
    Vec_Storage vec_as_int_storage = Vec_Storage::from_vec(_vec);
    Points_Cache::Iterator maybe_cached_it = _cache.find( vec_as_int_storage );
    if(maybe_cached_it.is_ok())
        return *maybe_cached_it;

    _cache.insert(vec_as_int_storage, _points.size());
    _points.push(_vec);

    return _points.size() - 1;
}



bool Terrain_Generator::M_grid_cell_is_border_left(const Terrain_Grid& _grid, unsigned int _x, unsigned int _y, unsigned int _z) const
{
    return _x > 0 && _grid.element_at(_x - 1, _y, _z) == Voxel_Type::Empty;
}

bool Terrain_Generator::M_grid_cell_is_border_right(const Terrain_Grid& _grid, unsigned int _x, unsigned int _y, unsigned int _z) const
{
    return _x + 1 < m_size_x && _grid.element_at(_x + 1, _y, _z) == Voxel_Type::Empty;
}

bool Terrain_Generator::M_grid_cell_is_border_below(const Terrain_Grid& _grid, unsigned int _x, unsigned int _y, unsigned int _z) const
{
    return _y > 0 && _grid.element_at(_x, _y - 1, _z) == Voxel_Type::Empty;
}

bool Terrain_Generator::M_grid_cell_is_border_above(const Terrain_Grid& _grid, unsigned int _x, unsigned int _y, unsigned int _z) const
{
    return _y + 1 < m_size_y && _grid.element_at(_x, _y + 1, _z) == Voxel_Type::Empty;
}

bool Terrain_Generator::M_grid_cell_is_border_behind(const Terrain_Grid& _grid, unsigned int _x, unsigned int _y, unsigned int _z) const
{
    return _z > 0 && _grid.element_at( _x, _y, _z - 1) == Voxel_Type::Empty;
}

bool Terrain_Generator::M_grid_cell_is_border_infront(const Terrain_Grid& _grid, unsigned int _x, unsigned int _y, unsigned int _z) const
{
    return _z + 1 < m_size_z && _grid.element_at(_x, _y, _z + 1) == Voxel_Type::Empty;
}


void Terrain_Generator::M_append_points(Chunk_Data& _chunk, Points& _points, Points_Cache& _cache, const glm::vec3& _point_1, const glm::vec3& _point_2, const glm::vec3& _point_3, const glm::vec3& _point_4) const
{
    unsigned int id_1 = M_get_or_construct_vector_id(_points, _cache, _point_1);
    unsigned int id_2 = M_get_or_construct_vector_id(_points, _cache, _point_2);
    unsigned int id_3 = M_get_or_construct_vector_id(_points, _cache, _point_3);
    unsigned int id_4 = M_get_or_construct_vector_id(_points, _cache, _point_4);

    _chunk.add_point(id_1);
    _chunk.add_point(id_2);
    _chunk.add_point(id_3);
    _chunk.add_point(id_1);
    _chunk.add_point(id_3);
    _chunk.add_point(id_4);
}

void Terrain_Generator::M_append_points_left(Chunk_Data& _chunk, Points& _points, Points_Cache& _cache, const glm::vec3& _center) const
{
    glm::vec3 point_1 = _center + glm::vec3(-m_cell_size_halved, m_cell_size_halved, m_cell_size_halved);
    glm::vec3 point_2 = _center + glm::vec3(-m_cell_size_halved, m_cell_size_halved, -m_cell_size_halved);
    glm::vec3 point_3 = _center + glm::vec3(-m_cell_size_halved, -m_cell_size_halved, -m_cell_size_halved);
    glm::vec3 point_4 = _center + glm::vec3(-m_cell_size_halved, -m_cell_size_halved, m_cell_size_halved);

    M_append_points(_chunk, _points, _cache, point_1, point_2, point_3, point_4);
}

void Terrain_Generator::M_append_points_right(Chunk_Data& _chunk, Points& _points, Points_Cache& _cache, const glm::vec3& _center) const
{
    glm::vec3 point_1 = _center + glm::vec3(m_cell_size_halved, m_cell_size_halved, m_cell_size_halved);
    glm::vec3 point_2 = _center + glm::vec3(m_cell_size_halved, m_cell_size_halved, -m_cell_size_halved);
    glm::vec3 point_3 = _center + glm::vec3(m_cell_size_halved, -m_cell_size_halved, -m_cell_size_halved);
    glm::vec3 point_4 = _center + glm::vec3(m_cell_size_halved, -m_cell_size_halved, m_cell_size_halved);

    M_append_points(_chunk, _points, _cache, point_4, point_3, point_2, point_1);
}

void Terrain_Generator::M_append_points_below(Chunk_Data& _chunk, Points& _points, Points_Cache& _cache, const glm::vec3& _center) const
{
    glm::vec3 point_1 = _center + glm::vec3(m_cell_size_halved, -m_cell_size_halved, m_cell_size_halved);
    glm::vec3 point_2 = _center + glm::vec3(-m_cell_size_halved, -m_cell_size_halved, m_cell_size_halved);
    glm::vec3 point_3 = _center + glm::vec3(-m_cell_size_halved, -m_cell_size_halved, -m_cell_size_halved);
    glm::vec3 point_4 = _center + glm::vec3(m_cell_size_halved, -m_cell_size_halved, -m_cell_size_halved);

    M_append_points(_chunk, _points, _cache, point_1, point_2, point_3, point_4);
}

void Terrain_Generator::M_append_points_above(Chunk_Data& _chunk, Points& _points, Points_Cache& _cache, const glm::vec3& _center) const
{
    glm::vec3 point_1 = _center + glm::vec3(m_cell_size_halved, m_cell_size_halved, m_cell_size_halved);
    glm::vec3 point_2 = _center + glm::vec3(-m_cell_size_halved, m_cell_size_halved, m_cell_size_halved);
    glm::vec3 point_3 = _center + glm::vec3(-m_cell_size_halved, m_cell_size_halved, -m_cell_size_halved);
    glm::vec3 point_4 = _center + glm::vec3(m_cell_size_halved, m_cell_size_halved, -m_cell_size_halved);

    M_append_points(_chunk, _points, _cache, point_4, point_3, point_2, point_1);
}

void Terrain_Generator::M_append_points_behind(Chunk_Data& _chunk, Points& _points, Points_Cache& _cache, const glm::vec3& _center) const
{
    glm::vec3 point_1 = _center + glm::vec3(m_cell_size_halved, m_cell_size_halved, -m_cell_size_halved);
    glm::vec3 point_2 = _center + glm::vec3(-m_cell_size_halved, m_cell_size_halved, -m_cell_size_halved);
    glm::vec3 point_3 = _center + glm::vec3(-m_cell_size_halved, -m_cell_size_halved, -m_cell_size_halved);
    glm::vec3 point_4 = _center + glm::vec3(m_cell_size_halved, -m_cell_size_halved, -m_cell_size_halved);

    M_append_points(_chunk, _points, _cache, point_4, point_3, point_2, point_1);
}

void Terrain_Generator::M_append_points_infront(Chunk_Data& _chunk, Points& _points, Points_Cache& _cache, const glm::vec3& _center) const
{
    glm::vec3 point_1 = _center + glm::vec3(m_cell_size_halved, m_cell_size_halved, m_cell_size_halved);
    glm::vec3 point_2 = _center + glm::vec3(-m_cell_size_halved, m_cell_size_halved, m_cell_size_halved);
    glm::vec3 point_3 = _center + glm::vec3(-m_cell_size_halved, -m_cell_size_halved, m_cell_size_halved);
    glm::vec3 point_4 = _center + glm::vec3(m_cell_size_halved, -m_cell_size_halved, m_cell_size_halved);

    M_append_points(_chunk, _points, _cache, point_1, point_2, point_3, point_4);
}

void Terrain_Generator::M_append_points_if_needed(Chunk_Data& _chunk, Points& _points, Points_Cache& _cache, const Terrain& _terrain, unsigned int _x, unsigned int _y, unsigned int _z) const
{
    const Terrain_Grid& grid = _terrain.grid();
    glm::vec3 center = _terrain.calculate_real_coord({_x, _y, _z});

    if(M_grid_cell_is_border_left(grid, _x, _y, _z))
        M_append_points_left(_chunk, _points, _cache, center);
    if(M_grid_cell_is_border_right(grid, _x, _y, _z))
        M_append_points_right(_chunk, _points, _cache, center);
    if(M_grid_cell_is_border_below(grid, _x, _y, _z))
        M_append_points_below(_chunk, _points, _cache, center);
    if(M_grid_cell_is_border_above(grid, _x, _y, _z))
        M_append_points_above(_chunk, _points, _cache, center);
    if(M_grid_cell_is_border_behind(grid, _x, _y, _z))
        M_append_points_behind(_chunk, _points, _cache, center);
    if(M_grid_cell_is_border_infront(grid, _x, _y, _z))
        M_append_points_infront(_chunk, _points, _cache, center);
}


unsigned int Terrain_Generator::M_calculate_chunks_amount_per_axis(unsigned int _size_per_axis) const
{
    return (_size_per_axis / Chunk_Size) + (_size_per_axis % Chunk_Size > 0);
}

unsigned int Terrain_Generator::M_calculate_chunks_amount() const
{
    unsigned int chunks_per_x = M_calculate_chunks_amount_per_axis(m_size_x);
    unsigned int chunks_per_y = M_calculate_chunks_amount_per_axis(m_size_y);
    unsigned int chunks_per_z = M_calculate_chunks_amount_per_axis(m_size_z);

    return chunks_per_x * chunks_per_y * chunks_per_z;
}


Chunk_Data Terrain_Generator::M_extract_chunk_mesh(Points& _points, Points_Cache& _cache, const Terrain& _terrain, unsigned int _chunk_x, unsigned int _chunk_y, unsigned int _chunk_z) const
{
    Chunk_Data result;
    result.mesh_ids.resize(200);

    unsigned int offset_x = _chunk_x * Chunk_Size;
    unsigned int offset_y = _chunk_y * Chunk_Size;
    unsigned int offset_z = _chunk_z * Chunk_Size;

    unsigned int limit_x = offset_x + Chunk_Size;
    if(limit_x > m_size_x)
        limit_x = m_size_x;
    unsigned int limit_y = offset_y + Chunk_Size;
    if(limit_y > m_size_y)
        limit_y = m_size_y;
    unsigned int limit_z = offset_z + Chunk_Size;
    if(limit_z > m_size_z)
        limit_z = m_size_z;

    const Terrain_Grid& grid = _terrain.grid();

    for(unsigned int x = offset_x; x < limit_x; ++x)
    {
        for(unsigned int y = offset_y; y < limit_y; ++y)
        {
            for(unsigned int z = offset_z; z < limit_z; ++z)
            {
                if(grid.element_at(x, y, z) != Voxel_Type::Filled)
                    continue;

                M_append_points_if_needed(result, _points, _cache, _terrain, x, y, z);
            }
        }
    }

    return result;
}


const Chunk_Data* Terrain_Generator::M_find_chunk(const LDS::Vector<Chunk_Data>& _chunks, const Coordinates& _coordinates) const
{
    for(unsigned int i = 0; i < _chunks.size(); ++i)
    {
        const Chunk_Data& chunk = _chunks[i];

        if(chunk.coordinates == _coordinates)
            return &chunk;
    }

    return nullptr;
}

LDS::Vector<const Chunk_Data*> Terrain_Generator::M_find_neighbour_chunks(const LDS::Vector<Chunk_Data>& _chunks, const Chunk_Data& _neighbour_to) const
{
    LDS::Vector<const Chunk_Data*> result(8);

    unsigned int x_offset = 0;
    if(_neighbour_to.coordinates.x() > 0)
        x_offset = _neighbour_to.coordinates.x() - 1;
    unsigned int y_offset = 0;
    if(_neighbour_to.coordinates.y() > 0)
        y_offset = _neighbour_to.coordinates.y() - 1;
    unsigned int z_offset = 0;
    if(_neighbour_to.coordinates.z() > 0)
        z_offset = _neighbour_to.coordinates.z() - 1;

    for(unsigned int x = x_offset; x <= _neighbour_to.coordinates.x() + 1; ++x)
    {
        for(unsigned int y = y_offset; y <= _neighbour_to.coordinates.y() + 1; ++y)
        {
            for(unsigned int z = z_offset; z <= _neighbour_to.coordinates.z() + 1; ++z)
            {
                Coordinates current_coordinates = {x, y, z};
                if(current_coordinates == _neighbour_to.coordinates)
                    continue;

                const Chunk_Data* chunk = M_find_chunk(_chunks, current_coordinates);
                if(!chunk)
                    continue;

                result.push(chunk);
            }
        }
    }

    return result;
}

void Terrain_Generator::M_append_mesh_to_average(const Points& _points,
                                                 const Chunk_Data& _chunk,
                                                 const glm::vec3& _reference_point,
                                                 Used_Points_Cache& _used_points_cache,
                                                 glm::vec3& _result_average,
                                                 unsigned int& _found_amount) const
{
    float max_distance = m_cell_size * LEti::Math::SQRT_3 * 1.05f;
    float max_distance_squared = max_distance * max_distance;

    for(unsigned int i = 0; i < _chunk.used_points.size(); ++i)
    {
        unsigned int index = _chunk.used_points[i];

        if(_used_points_cache.marked(index))
            continue;
        _used_points_cache.mark(index);

        const glm::vec3& point = _points[index];

        float distance_squared = LEti::Math::vector_length_squared(point - _reference_point);
        if(distance_squared > max_distance_squared)
            continue;
        if(distance_squared < 1e-6f)
            continue;

        _result_average += point;
        ++_found_amount;
    }
}

void Terrain_Generator::M_smooth_chunk(Points& _new_points,
                                       const Points& _old_points,
                                       LDS::Vector<bool>& _smoothed,
                                       Used_Points_Cache& _used_points_cache,
                                       const LDS::Vector<Chunk_Data>& _chunks,
                                       const Chunk_Data& _chunk) const
{
    constexpr float Increment_Multiplier = 1.1f;

    LDS::Vector<const Chunk_Data*> neighbour_chunks = M_find_neighbour_chunks(_chunks, _chunk);

    for(unsigned int i = 0; i < _chunk.used_points.size(); ++i)
    {
        unsigned int point_index = _chunk.used_points[i];

        if(_smoothed[point_index])
            continue;
        _smoothed[point_index] = true;

        glm::vec3 average_point = {0.0f, 0.0f, 0.0f};
        unsigned int points_found = 0;
        const glm::vec3& reference = _old_points[point_index];
        _used_points_cache.reset();

        M_append_mesh_to_average(_old_points, _chunk, reference, _used_points_cache, average_point, points_found);
        for(unsigned int n_i = 0; n_i < neighbour_chunks.size(); ++n_i)
            M_append_mesh_to_average(_old_points, *neighbour_chunks[n_i], reference, _used_points_cache, average_point, points_found);

        average_point /= (float)points_found;
        glm::vec3 increment = average_point - reference;
        increment *= Increment_Multiplier;

        _new_points[point_index] = reference + increment;
    }
}

void Terrain_Generator::M_smooth_points(const LDS::Vector<Chunk_Data>& _chunks, Points& _points) const
{
    L_LOG("Terrain Generation", std::string("Chunks to smooth: ") + std::to_string(_chunks.size()) + " (" + std::to_string(_points.size()) + " points)");

    LST::Stopwatch total_stopwatch;
    total_stopwatch.start();

    LST::Thread_Pool thread_pool;
    unsigned int chunks_per_thread = _chunks.size() / thread_pool.threads_amount();
    L_ASSERT(chunks_per_thread > 0);

    using Smoothed_Check_Vector = LDS::Vector<bool>;
    LDS::Vector<Smoothed_Check_Vector> smoothed(thread_pool.threads_amount(), { _points.size(), false });
    LDS::Vector<Points> smoothed_points(thread_pool.threads_amount(), { _points.size(), {} });

    for(unsigned int thr_i = 0; thr_i < thread_pool.threads_amount(); ++thr_i)
    {
        unsigned int chunk_offset = chunks_per_thread * thr_i;
        unsigned int chunk_limit = chunks_per_thread * (thr_i + 1);
        if(chunk_limit > _chunks.size())
            chunk_limit = _chunks.size();

        thread_pool.add_task([thr_i, chunk_offset, chunk_limit, &_chunks, &_points, &smoothed, &smoothed_points, this]()
        {
            Used_Points_Cache used_points_cache(_points.size());
            Smoothed_Check_Vector& current_smoothed = smoothed[thr_i];
            Points& current_points = smoothed_points[thr_i];

            for(unsigned int i = chunk_offset; i < chunk_limit; ++i)
                M_smooth_chunk(current_points, _points, current_smoothed, used_points_cache, _chunks, _chunks[i]);
        });
    }

    thread_pool.wait_for_completion();

    Points result(_points.size(), {});

    for(unsigned int i = 0; i < _points.size(); ++i)
    {
        unsigned int times_smoothed = 0;
        glm::vec3 result_point = {0.0f, 0.0f, 0.0f};

        for(unsigned int thr_i = 0; thr_i < thread_pool.threads_amount(); ++thr_i)
        {
            if(!smoothed[thr_i][i])
                continue;

            ++times_smoothed;
            result_point += smoothed_points[thr_i][i];
        }

        if(times_smoothed == 0)
        {
            result[i] = _points[i];
            continue;
        }

        result_point /= (float)times_smoothed;
        result[i] = result_point;
    }

    total_stopwatch.stop();
    L_LOG("Terrain Generation", std::string("Smoothing took: ") + std::to_string(total_stopwatch.duration()));

    _points = (Points&&)result;
}


void Terrain_Generator::M_randomize_points(Points& _points) const
{
    constexpr glm::vec3 Default_Vector = {1.0f, 0.0f, 0.0f};

    for(unsigned int p_i = 0; p_i < _points.size(); ++p_i)
    {
        glm::vec3& point = _points[p_i];

        glm::vec3 random_axis;
        for(unsigned int i = 0; i < 3; ++i)
            random_axis[i] = LEti::Math::random_number_float(0.0f, 1.0f);
        LEti::Math::shrink_vector_to_1(random_axis);

        float random_angle = LEti::Math::random_number_float(0.0f, LEti::Math::DOUBLE_PI);

        glm::vec3 random_direction = LEti::Math::rotate_vector(Default_Vector, random_axis, random_angle);

        float random_distance = LEti::Math::random_number_float(m_min_random_point_stride, m_max_random_point_stride);

        point -= random_direction * random_distance;
    }
}


LDS::Vector<float> Terrain_Generator::M_construct_geometry_data(const Points_Ids& _mesh_ids, const Points& _points) const
{
    LDS::Vector<float> result(_mesh_ids.size() * 3);

    for(unsigned int i = 0; i < _mesh_ids.size(); ++i)
    {
        const glm::vec3& point = _points[_mesh_ids[i]];

        result.push(point.x);
        result.push(point.y);
        result.push(point.z);
    }

    return result;
}

LDS::Vector<float> Terrain_Generator::M_construct_texture_data(const Points_Ids& _mesh_ids, const Points& _points) const
{
    LDS::Vector<float> result(_mesh_ids.size() * 2);

    unsigned int polygons_amount = _mesh_ids.size() / 3;

    for(unsigned int i = 0; i < polygons_amount; ++i)
    {
        // const glm::vec3& point = _points[_mesh_ids[i]];

        result.push(0.0f);
        result.push(0.0f);

        result.push(1.0f);
        result.push(0.0f);

        result.push(1.0f);
        result.push(1.0f);
    }

    return result;
}

LDS::Vector<float> Terrain_Generator::M_construct_normals_data(const Points_Ids& _mesh_ids, const Points& _points) const
{
    LDS::Vector<float> result(_mesh_ids.size() * 3);

    for(unsigned int i = 0; i < _mesh_ids.size(); i += 3)
    {
        const glm::vec3& point_1 = _points[_mesh_ids[i]];
        const glm::vec3& point_2 = _points[_mesh_ids[i + 1]];
        const glm::vec3& point_3 = _points[_mesh_ids[i + 2]];

        glm::vec3 normal = LEti::Math::cross_product(point_2 - point_1, point_3 - point_1);
        LEti::Math::shrink_vector_to_1(normal);

        for(unsigned int r_i = 0; r_i < 3; ++r_i)
        {
            result.push(normal.x);
            result.push(normal.y);
            result.push(normal.z);
        }
    }

    return result;
}



Terrain* Terrain_Generator::generate()
{
    L_ASSERT(m_terrain_object_stub);
    L_ASSERT(m_size_x > 0);
    L_ASSERT(m_size_y > 0);
    L_ASSERT(m_size_z > 0);
    L_ASSERT(m_cell_size > 0.0f);

    L_LOG("Terrain Generation", std::string("generating map with seed ") + std::to_string(m_seed));
    srand(m_seed);

    Terrain* result = new Terrain;
    result->set_cell_size(m_cell_size);

    Terrain_Grid& grid = result->grid();
    grid.init(m_size_x, m_size_y, m_size_z);

    M_apply_shape_to_grid(*result);

    unsigned int chunks_per_x = M_calculate_chunks_amount_per_axis(m_size_x);
    unsigned int chunks_per_y = M_calculate_chunks_amount_per_axis(m_size_y);
    unsigned int chunks_per_z = M_calculate_chunks_amount_per_axis(m_size_z);
    unsigned int chunk_amount = chunks_per_x * chunks_per_y * chunks_per_z;

    Points_Cache cache;
    Points points(1000);
    Chunks_Data chunks(chunk_amount);

    for(unsigned int x = 0; x < chunks_per_x; ++x)
    {
        for(unsigned int y = 0; y < chunks_per_y; ++y)
        {
            for(unsigned int z = 0; z < chunks_per_z; ++z)
            {
                Chunk_Data chunk = M_extract_chunk_mesh(points, cache, *result, x, y, z);
                if(chunk.mesh_ids.size() == 0)
                    continue;

                chunk.coordinates = {x, y, z};
                chunks.push((Chunk_Data&&)chunk);
            }
        }
    }

    M_smooth_points(chunks, points);
    M_randomize_points(points);

    result->set_points((Points&&)points);
    result->set_chunks_data((Chunks_Data&&)chunks);

    return result;
}

LEti::Object* Terrain_Generator::construct_chunk_object(const Terrain& _terrain, const Coordinates& _coordinates) const
{
    const Chunk_Data* maybe_chunk = M_find_chunk(_terrain.chunks(), _coordinates);
    if(!maybe_chunk)
        return nullptr;

    LEti::Object* result = LEti::Object_Stub::construct_from(m_terrain_object_stub);

    L_ASSERT(maybe_chunk->mesh_ids.size() > 0);

    const Points_Ids& mesh_ids = maybe_chunk->mesh_ids;
    const Points& points = _terrain.points();

    LDS::Vector<float> geometry_data = M_construct_geometry_data(mesh_ids, points);
    LDS::Vector<float> texture_data = M_construct_texture_data(mesh_ids, points);
    LDS::Vector<float> normals_data = M_construct_normals_data(mesh_ids, points);

    LR::Draw_Module* dm = result->get_module_of_type<LR::Draw_Module>();
    L_ASSERT(dm);
    LR::Graphics_Component__Default* gc_geometry = (LR::Graphics_Component__Default*)dm->get_graphics_component_with_buffer_index(0);
    L_ASSERT(LV::cast_variable<LR::Graphics_Component__Default>(gc_geometry));
    LR::Graphics_Component__Default* gc_texture = (LR::Graphics_Component__Default*)dm->get_graphics_component_with_buffer_index(1);
    L_ASSERT(LV::cast_variable<LR::Graphics_Component__Default>(gc_texture));
    LR::Graphics_Component__Default* gc_normals = (LR::Graphics_Component__Default*)dm->get_graphics_component_with_buffer_index(2);
    L_ASSERT(LV::cast_variable<LR::Graphics_Component__Default>(gc_normals));

    dm->apply_draw_rule();
    gc_geometry->buffer().resize(geometry_data.size());
    gc_geometry->buffer().copy_array(geometry_data.raw_data(), geometry_data.size());
    gc_texture->buffer().resize(texture_data.size());
    gc_texture->buffer().copy_array(texture_data.raw_data(), texture_data.size());
    gc_normals->buffer().resize(normals_data.size());
    gc_normals->buffer().copy_array(normals_data.raw_data(), normals_data.size());

    LPhys::Physics_Module__Mesh* pm = result->get_module_of_type<LPhys::Physics_Module__Mesh>();
    L_ASSERT(pm);

    pm->setup_base_data(geometry_data.raw_data(), geometry_data.size(), nullptr);
    pm->update_physical_model();

    result->update(0.0f);
    result->update_previous_state();

    return result;
}
