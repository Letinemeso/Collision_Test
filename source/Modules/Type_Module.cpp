#include <Modules/Type_Module.h>

using namespace Shardis;


ON_VALUES_ASSIGNED_IMPLEMENTATION(Type_Module_Stub)
{
    if(object_type_str == "Terrain")
        object_type = Object_Type::Terrain;
    else if(object_type_str == "Player")
        object_type = Object_Type::Player;
    else if(object_type_str == "Enemy")
        object_type = Object_Type::Enemy;
    else if(object_type_str == "Player_Attack")
        object_type = Object_Type::Player_Attack;
    else if(object_type_str == "Enemy_Attack")
        object_type = Object_Type::Enemy_Attack;

    L_ASSERT(object_type != Object_Type::Unknown);
}



BUILDER_STUB_DEFAULT_CONSTRUCTION_FUNC(Type_Module_Stub)

BUILDER_STUB_INITIALIZATION_FUNC(Type_Module_Stub)
{
    BUILDER_STUB_PARENT_INITIALIZATION;
    BUILDER_STUB_CAST_PRODUCT;

    product->set_object_type(object_type);
}
