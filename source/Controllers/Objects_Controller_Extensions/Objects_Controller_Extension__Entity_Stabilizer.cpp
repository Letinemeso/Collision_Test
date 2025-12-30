#include <Controllers/Objects_Controller_Extensions/Objects_Controller_Extension__Entity_Stabilizer.h>

#include <Math_Stuff.h>

using namespace Shardis;


void Objects_Controller_Extension__Entity_Stabilizer::update(Objects_List& _objects, float _dt)
{
    for(Objects_List::Iterator it = _objects.begin(); !it.end_reached(); ++it)
    {
        LEti::Object* object = *it;

        const glm::vec3& previous_position = object->previous_state().position();
        const glm::vec3& current_position = object->current_state().position();

        float stride_squared = LEti::Math::vector_length_squared(current_position - previous_position);
        if(stride_squared >= m_min_stride_per_frame_squared || stride_squared < 1e-6f)
            continue;

        object->current_state().set_position(previous_position);
        object->update(0.0f);
    }
}
