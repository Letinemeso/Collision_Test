#include <Maybe_Immovable_Physics_Module.h>


BUILDER_STUB_DEFAULT_CONSTRUCTION_FUNC(Maybe_Immovable_Physics_Module_Stub)

BUILDER_STUB_INITIALIZATION_FUNC(Maybe_Immovable_Physics_Module_Stub)
{
    BUILDER_STUB_PARENT_INITIALIZATION;
    BUILDER_STUB_CAST_PRODUCT;

    product->set_movable(movable);
}
