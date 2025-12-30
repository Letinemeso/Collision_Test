#include <Terrain/Terrain_Utility.h>

using namespace Shardis;


Vec_Storage Vec_Storage::from_vec(const glm::vec3& _from)
{
    Vec_Storage result;
    for(unsigned int i = 0; i < 3; ++i)
        result[i] = _from[i] * Float_To_Int_Factor;
    return result;
}

glm::vec3 Vec_Storage::to_vec() const
{
    glm::vec3 result;
    for(unsigned int i = 0; i < 3; ++i)
        result[i] = (float)(m_storage[i]) * Float_To_Int_Factor_Inversed;
    return result;
}


bool Vec_Storage::operator==(const Vec_Storage& _other) const
{
    for(unsigned int i = 0; i < 3; ++i)
    {
        if(m_storage[i] != _other.m_storage[i])
            return false;
    }
    return true;
}

bool Vec_Storage::operator<(const Vec_Storage& _other) const
{
    for(unsigned int i = 0; i < 3; ++i)
    {
        if(m_storage[i] < _other.m_storage[i])
            return true;
        if(m_storage[i] > _other.m_storage[i])
            return false;
    }
    return false;
}

bool Vec_Storage::operator>(const Vec_Storage& _other) const
{
    for(unsigned int i = 0; i < 3; ++i)
    {
        if(m_storage[i] > _other.m_storage[i])
            return true;
        if(m_storage[i] < _other.m_storage[i])
            return false;
    }
    return false;
}





Chunk_Data::Chunk_Data(Chunk_Data&& _from)
{
    coordinates = _from.coordinates;

    mesh_ids = (Points_Ids&&)_from.mesh_ids;
    used_points = (Points_Ids&&)_from.used_points;
}

void Chunk_Data::operator=(Chunk_Data&& _from)
{
    coordinates = _from.coordinates;

    mesh_ids = (Points_Ids&&)_from.mesh_ids;
    used_points = (Points_Ids&&)_from.used_points;
}


void Chunk_Data::add_point(unsigned int _point)
{
    mesh_ids.push(_point);

    if(!used_points.find(_point).is_ok())
        used_points.push(_point);
}





Used_Points_Cache::Used_Points_Cache(unsigned int _size)
{
    m_cache.resize_and_fill(_size, false);
    m_modified_indices.resize(2000);
}



void Used_Points_Cache::mark(unsigned int _index)
{
    m_cache[_index] = true;
    m_modified_indices.push(_index);
}

void Used_Points_Cache::reset()
{
    for(unsigned int i = 0; i < m_modified_indices.size(); ++i)
        m_cache[m_modified_indices[i]] = false;
    m_modified_indices.mark_empty();
}
