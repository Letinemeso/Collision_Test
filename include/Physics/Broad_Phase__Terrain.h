#pragma once

#include <Collision_Detection/Broad_Phase/Broad_Phase_Interface.h>


namespace Shardis
{

    class Broad_Phase__Terrain : public LPhys::Broad_Phase_Interface
    {
    private:
        LDS::Vector<LPhys::Physics_Module*> m_movable_modules;
        LDS::Vector<LPhys::Physics_Module*> m_immovable_modules;

    public:
        void reset() override;
        void add_models(const Objects_List& _objects) override;
        void process() override;

    };

}
