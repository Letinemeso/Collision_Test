#pragma once

#include <Data_Structures/Vector.h>

#include <Terrain/Terrain_Utility.h>


namespace Shardis
{

    enum class Voxel_Type
    {
        Unknown,
        Empty,
        Filled
    };


    class Terrain_Grid
    {
    private:
        LDS::Vector<Voxel_Type> m_data;
        Coordinates m_size;

    public:
        Terrain_Grid();
        ~Terrain_Grid();

    public:
        inline unsigned int size_x() const { return m_size.x(); }
        inline unsigned int size_y() const { return m_size.y(); }
        inline unsigned int size_z() const { return m_size.z(); }
        inline const Coordinates size() const { return m_size; }

    private:
        unsigned int M_calculate_data_size() const;
        unsigned int M_calculate_data_index(unsigned int _x, unsigned int _y, unsigned int _z) const;
        unsigned int M_calculate_data_index(const Coordinates& _at) const;

    public:
        void init(unsigned int _size_x, unsigned int _size_y, unsigned int _size_z);

    public:
        Voxel_Type element_at(unsigned int _x, unsigned int _y, unsigned int _z) const;
        Voxel_Type element_at(const Coordinates& _at) const;
        void set_element_at(Voxel_Type _value, unsigned int _x, unsigned int _y, unsigned int _z);
        void set_element_at(Voxel_Type _value, const Coordinates& _at);

    };

}
