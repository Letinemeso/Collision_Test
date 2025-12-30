#pragma once

#include <Stuff/Message_Type.h>

#include <Object.h>


namespace Shardis
{

    MESSAGE_TYPE_DECLARATION_START(Message__On_Entity_Death)
    LEti::Object* almost_dead_entity = nullptr;
    MESSAGE_TYPE_DECLARATION_END

    MESSAGE_TYPE_DECLARATION_START(Message__On_Entity_Damaged)
    LEti::Object* damaging_entity = nullptr;
    LEti::Object* damaged_entity = nullptr;
    float health_before = 0.0f;
    float health_after = 0.0f;
    float damage = 0.0f;
    float stagger = 0.0f;
    glm::vec3 damaged_point = {0.0f, 0.0f, 0.0f};
    MESSAGE_TYPE_DECLARATION_END

    MESSAGE_TYPE_DECLARATION_START(Message__Player_Evades)
    MESSAGE_TYPE_DECLARATION_END


    MESSAGE_TYPE_DECLARATION_START(Message__Find_Player)
    LEti::Object* player_object = nullptr;
    MESSAGE_TYPE_DECLARATION_END


    MESSAGE_TYPE_DECLARATION_START(Message__Update_Camera)
    MESSAGE_TYPE_DECLARATION_END

}
