#include <Controllers/Objects_Controller_Extensions/Objects_Controller_Extension__On_Death_Notification.h>

using namespace Shardis;


void Objects_Controller_Extension__On_Death_Notification::on_object_removed(LEti::Object* _object)
{
    Message__On_Entity_Death msg;
    msg.almost_dead_entity = _object;

    LST::Message_Translator::instance().translate(msg);
}
