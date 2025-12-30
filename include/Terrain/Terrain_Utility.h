#pragma once

#include <vec3.hpp>

#include <Data_Structures/Vector.h>


namespace Shardis
{

    constexpr float Float_To_Int_Factor = 10000.0f;
    constexpr float Float_To_Int_Factor_Inversed = 1.0f / Float_To_Int_Factor;

    using Points_Ids = LDS::Vector<unsigned int>;
    using Points = LDS::Vector<glm::vec3>;

    class Coordinates
    {
    private:
        unsigned int data[3] = { Invalid_Coord, Invalid_Coord, Invalid_Coord };

    public:
        constexpr static unsigned int Invalid_Coord = 0xFFFFFFFF;

        Coordinates(){}
        Coordinates(unsigned int _x, unsigned int _y, unsigned int _z) { x() = _x; y() = _y; z() = _z; }

        inline unsigned int operator[](unsigned int _index) const { L_ASSERT(_index < 3); return data[_index]; }
        inline unsigned int& operator[](unsigned int _index) { L_ASSERT(_index < 3); return data[_index]; }

        inline unsigned int x() const { return data[0]; }
        inline unsigned int& x() { return data[0]; }
        inline unsigned int y() const { return data[1]; }
        inline unsigned int& y() { return data[1]; }
        inline unsigned int z() const { return data[2]; }
        inline unsigned int& z() { return data[2]; }

        inline bool valid() const { for(unsigned int i = 0; i < 3; ++i) { if(data[i] == Invalid_Coord) return false; } return true; }

        inline bool operator==(const Coordinates& _other) const { return x() == _other.x() && y() == _other.y() && z() == _other.z(); }
        inline bool operator!=(const Coordinates& _other) const { return !(*this == _other); }
    };


    class Vec_Storage
    {
    private:
        int m_storage[3];

    public:
        inline int& x() { return m_storage[0]; }
        inline int& y() { return m_storage[1]; }
        inline int& z() { return m_storage[2]; }

        inline int& operator[](unsigned int _index) { L_ASSERT(_index < 3); return m_storage[_index]; }

    public:
        static Vec_Storage from_vec(const glm::vec3& _from);
        glm::vec3 to_vec() const;

        bool operator==(const Vec_Storage& _other) const;
        bool operator<(const Vec_Storage& _other) const;
        bool operator>(const Vec_Storage& _other) const;
    };


    struct Chunk_Data
    {
        Coordinates coordinates;
        Points_Ids mesh_ids;
        Points_Ids used_points;

        Chunk_Data() { }
        Chunk_Data(Chunk_Data&& _from);
        void operator=(Chunk_Data&& _from);

        Chunk_Data(const Chunk_Data&)
        {
            L_ASSERT(false);        //  calling this should be prevented by algorithm, but is required for vector
        }
        void operator=(const Chunk_Data&)
        {
            L_ASSERT(false);        //  calling this should be prevented by algorithm, but is required for vector
        }

        void add_point(unsigned int _point);

        inline bool operator==(const Chunk_Data& _other) { return coordinates == _other.coordinates; }
    };

    using Chunks_Data = LDS::Vector<Chunk_Data>;


    class Used_Points_Cache
    {
    private:
        LDS::Vector<bool> m_cache;
        LDS::Vector<unsigned int> m_modified_indices;

    public:
        Used_Points_Cache(unsigned int _size);

    public:
        inline bool marked(unsigned int _index) { return m_cache[_index]; }

        void mark(unsigned int _index);
        void reset();

    };

}
