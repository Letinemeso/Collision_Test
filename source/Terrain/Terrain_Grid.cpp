#include <Terrain/Terrain_Grid.h>

using namespace Shardis;


Terrain_Grid::Terrain_Grid()
{

}

Terrain_Grid::~Terrain_Grid()
{

}



unsigned int Terrain_Grid::M_calculate_data_size() const
{
    return m_size.x() * m_size.y() * m_size.z();
}

unsigned int Terrain_Grid::M_calculate_data_index(unsigned int _x, unsigned int _y, unsigned int _z) const
{
    L_ASSERT(_x < m_size.x());
    L_ASSERT(_y < m_size.y());
    L_ASSERT(_z < m_size.z());

    unsigned int result = 0;

    unsigned int layer_size = m_size.x() * m_size.z();
    result += layer_size * _y;
    result += _x * m_size.z();
    result += _z;

    return result;
}

unsigned int Terrain_Grid::M_calculate_data_index(const Coordinates& _at) const
{
    L_ASSERT(_at.valid());

    unsigned int result = 0;

    unsigned int layer_size = m_size.x() * m_size.z();
    result += layer_size * _at.y();
    result += _at.x() * m_size.z();
    result += _at.z();

    return result;
}



void Terrain_Grid::init(unsigned int _size_x, unsigned int _size_y, unsigned int _size_z)
{
    m_size.x() = _size_x;
    m_size.y() = _size_y;
    m_size.z() = _size_z;

    m_data.resize_and_fill( M_calculate_data_size(), Voxel_Type::Filled );
}



Voxel_Type Terrain_Grid::element_at(unsigned int _x, unsigned int _y, unsigned int _z) const
{
    unsigned int index = M_calculate_data_index(_x, _y, _z);
    Voxel_Type value = m_data[index];
    return value;
}

Voxel_Type Terrain_Grid::element_at(const Coordinates& _at) const
{
    unsigned int index = M_calculate_data_index(_at);
    Voxel_Type value = m_data[index];
    return value;
}

void Terrain_Grid::set_element_at(Voxel_Type _value, unsigned int _x, unsigned int _y, unsigned int _z)
{
    unsigned int index = M_calculate_data_index(_x, _y, _z);
    m_data[index] = _value;
}

void Terrain_Grid::set_element_at(Voxel_Type _value, const Coordinates& _at)
{
    unsigned int index = M_calculate_data_index(_at);
    m_data[index] = _value;
}
