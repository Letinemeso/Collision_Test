#include <Controllers/Objects_Controller_Extensions/Objects_Controller_Extension__Entity_Proximity_Checker.h>

#include <Math_Stuff.h>

#include <Modules/Type_Module.h>

using namespace Shardis;


bool Objects_Controller_Extension__Entity_Proximity_Checker::should_remove_object(LEti::Object* _object)
{
    Type_Module* type_module = _object->get_module_of_type<Type_Module>();
    if(!type_module)
        return false;
    if(type_module->object_type() != Object_Type::Enemy)
        return false;

    const glm::vec3& camera_position = m_camera->settings().position;
    const glm::vec3& object_position = _object->current_state().position();

    float distance_squared = LEti::Math::vector_length_squared(camera_position - object_position);

    return distance_squared > m_max_distance_squared;
}
