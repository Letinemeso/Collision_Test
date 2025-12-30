#include <Terrain/Terrain.h>

using namespace Shardis;


Terrain::Terrain()
{

}

Terrain::~Terrain()
{

}



void Terrain::clear_terrain_objects()
{
    m_terrain_objects.clear();
}


bool Terrain::terrain_object_exists(const Coordinates& _coordinates) const
{
    for(Terrain_Objects::Const_Iterator it = m_terrain_objects.begin(); !it.end_reached(); ++it)
    {
        const Coordinates& coordinates = it->coordinates;
        if(coordinates == _coordinates)
            return true;
    }
    return false;
}


Coordinates Terrain::calculate_grid_cell(const glm::vec3& _real_coord) const
{
    glm::vec3 size_halved = calculate_map_size() * 0.5f;
    glm::vec3 coord_modified = _real_coord + size_halved;
    coord_modified /= m_cell_size;

    for(unsigned int i = 0; i < 3; ++i)
    {
        if(coord_modified[i] < 0.0f)
            return {};
    }

    Coordinates result;
    for(unsigned int i = 0; i < 3; ++i)
    {
        result[i] = (unsigned int)coord_modified[i];
        if(result[i] >= m_grid.size()[i])
            return {};
    }

    return result;
}

glm::vec3 Terrain::calculate_real_coord(const Coordinates& _grid_cell) const
{
    L_ASSERT(_grid_cell.valid());

    glm::vec3 result;
    result.x = (float)_grid_cell.x() * m_cell_size;
    result.y = (float)_grid_cell.y() * m_cell_size;
    result.z = (float)_grid_cell.z() * m_cell_size;

    glm::vec3 map_size_halved = calculate_map_size() * 0.5f;
    result -= map_size_halved;

    return result;
}

glm::vec3 Terrain::calculate_map_size() const
{
    glm::vec3 result;
    result.x = (float)m_grid.size_x() * m_cell_size;
    result.y = (float)m_grid.size_y() * m_cell_size;
    result.z = (float)m_grid.size_z() * m_cell_size;
    return result;
}

glm::vec3 Terrain::get_closest_grounded_point(const glm::vec3& _closest_to) const
{
    Coordinates grid_cell = calculate_grid_cell(_closest_to);
    if(!grid_cell.valid())
        grid_cell = {m_grid.size().x() / 2, m_grid.size().y() / 2, m_grid.size().z() / 2};

    Coordinates grounded_cell = find_grounded_cell(grid_cell);
    if(!grounded_cell.valid())
        return {0.0f, 0.0f, 0.0f};

    return calculate_real_coord(grounded_cell);
}

Coordinates Terrain::find_grounded_cell(const Coordinates& _cell) const
{
    L_ASSERT(_cell.valid());

    if(m_grid.element_at(_cell) == Voxel_Type::Empty)
    {
        Coordinates coord = _cell;
        for(unsigned int y = coord.y() - 1; y > 0; --y)
        {
            coord.y() = y;
            Voxel_Type type = m_grid.element_at(coord);
            if(type == Voxel_Type::Filled)
                return { coord.x(), y + 1, coord.z() };
        }

        return _cell;
    }

    Coordinates coord = _cell;
    for(unsigned int y = coord.y() - 1; y > 0; --y)
    {
        coord.y() = y;
        Voxel_Type type = m_grid.element_at(coord);
        if(type == Voxel_Type::Empty)
            break;
    }

    for(unsigned int y = coord.y() - 1; y > 0; --y)
    {
        coord.y() = y;
        Voxel_Type type = m_grid.element_at(coord);
        if(type == Voxel_Type::Filled)
            return { coord.x(), y + 1, coord.z() };
    }

    coord = _cell;
    for(unsigned int y = coord.y() + 1; y < m_grid.size_y(); ++y)
    {
        coord.y() = y;
        Voxel_Type type = m_grid.element_at(coord);
        if(type == Voxel_Type::Empty)
            return coord;
    }

    return _cell;
}

glm::vec3 Terrain::get_start_point() const
{
    glm::vec3 point = calculate_real_coord(m_start_point);
    return get_closest_grounded_point(point);
}


Coordinates Terrain::calculate_size_in_chunks() const
{
    Coordinates result = m_grid.size();

    for(unsigned int i = 0; i < 3; ++i)
        result[i] = (result[i] / Chunk_Size) + (result[i] % Chunk_Size > 0);

    return result;
}

Coordinates Terrain::calculate_chunk_coordinates(const Coordinates& _grid_cell) const
{
    if(!_grid_cell.valid())
        return {};

    Coordinates result;
    for(unsigned int i = 0; i < 3; ++i)
        result[i] = _grid_cell[i] / Chunk_Size;

    return result;
}

Coordinates Terrain::calculate_chunk_coordinates(const glm::vec3& _real_coord) const
{
    return calculate_chunk_coordinates( calculate_grid_cell(_real_coord) );
}
