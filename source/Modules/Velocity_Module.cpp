#include <Modules/Velocity_Module.h>

#include <Math_Stuff.h>

using namespace Shardis;

namespace Shardis
{
    constexpr float Grounded_Duration = 0.075f;
}


void Velocity_Module::mark_grounded(bool _grounded)
{
    if(_grounded)
        m_grounded_timer.start(Grounded_Duration);
    else
        m_grounded_timer.reset();
}



void Velocity_Module::update(float _dt)
{
    transformation_data()->move(m_impulse);

    m_impulse += stride_per_second() * _dt;

    if(LEti::Math::vector_length_squared(m_impulse) > m_max_impulse_squared)
        LEti::Math::extend_vector_to_length(m_impulse, m_max_impulse);

    m_grounded_timer.update(_dt);
}





BUILDER_STUB_DEFAULT_CONSTRUCTION_FUNC(Velocity_Module_Stub)

BUILDER_STUB_INITIALIZATION_FUNC(Velocity_Module_Stub)
{
    BUILDER_STUB_PARENT_INITIALIZATION;
    BUILDER_STUB_CAST_PRODUCT;

    product->set_max_impulse(max_impulse);
}
