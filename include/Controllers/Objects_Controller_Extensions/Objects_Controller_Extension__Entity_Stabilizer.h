#pragma once

#include <Stuff/Message_Translator.h>

#include <Object.h>

#include <Tools/Objects_Controller/Objects_Controller_Extension.h>

#include <Messages/Messages.h>


namespace Shardis
{

    class Objects_Controller_Extension__Entity_Stabilizer : public LMD::Objects_Controller_Extension
    {
    private:
        float m_min_stride_per_frame = 0.0f;
        float m_min_stride_per_frame_squared = 0.0f;

    public:
        inline void set_min_stride_per_frame(float _value) { m_min_stride_per_frame = _value; m_min_stride_per_frame_squared = _value * _value; }

    public:
        void on_object_added(LEti::Object* _object) override { }
        void on_object_removed(LEti::Object* _object) override { }

        void reset() override { }
        void update(Objects_List& _objects, float _dt) override;
    };

}
