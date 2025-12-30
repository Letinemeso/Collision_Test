#pragma once

#include <Stuff/Message_Translator.h>

#include <Object.h>

#include <Tools/Objects_Controller/Objects_Controller_Extension.h>

#include <Messages/Messages.h>


namespace Shardis
{

    class Objects_Controller_Extension__On_Death_Notification : public LMD::Objects_Controller_Extension
    {
    public:
        void on_object_added(LEti::Object* _object) override { }
        void on_object_removed(LEti::Object* _object) override;

        void reset() override { }
        void update(Objects_List& _objects, float _dt) override { }
    };

}
