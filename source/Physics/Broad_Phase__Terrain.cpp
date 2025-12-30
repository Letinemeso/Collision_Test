#include <Physics/Broad_Phase__Terrain.h>

#include <Modules/Physics_Module__Mesh.h>

using namespace Shardis;


void Broad_Phase__Terrain::reset()
{
    m_movable_modules.mark_empty();
    m_immovable_modules.mark_empty();

    m_possible_collisions.clear();
}

void Broad_Phase__Terrain::add_models(const Objects_List& _objects)
{
    if(m_movable_modules.capacity() < _objects.size())
        m_movable_modules.resize(_objects.size());
    if(m_immovable_modules.capacity() < _objects.size())
        m_immovable_modules.resize(_objects.size());

    for(Objects_List::Const_Iterator it = _objects.begin(); !it.end_reached(); ++it)
    {
        LPhys::Physics_Module* module = *it;

        if(module->is_static())
            m_immovable_modules.push(module);
        else
            m_movable_modules.push(module);
    }
}

void Broad_Phase__Terrain::process()
{
    for(unsigned int immovable_i = 0; immovable_i < m_immovable_modules.size(); ++immovable_i)
    {
        LPhys::Physics_Module* immovable_module = m_immovable_modules[immovable_i];

        for(unsigned int movable_i = 0; movable_i < m_movable_modules.size(); ++movable_i)
        {
            LPhys::Physics_Module* movable_module = m_movable_modules[movable_i];

            m_possible_collisions.push_back({ movable_module, immovable_module });
        }
    }
}
