#pragma once

#include <Stuff/Message_Translator.h>
#include <Stuff/Timer.h>

#include <Module.h>

#include <Draw_Modules/Draw_Module.h>

#include <Misc_Draw_Modules/Vertex_Animation/Draw_Module__Vertex_Animation.h>
#include <Tools/Objects_Controller/Objects_Controller.h>

#include <Behavior_Tree/Behavior_Tree_Stuff.h>

#include <Messages/Messages.h>
#include <Controllers/Spawn_Controller.h>
#include <Modules/Velocity_Module.h>


namespace Shardis
{

    class Crystalis_AI : public LEti::Module
    {
    public:
        INIT_VARIABLE(Shardis::Crystalis_AI, LEti::Module)

    private:
        LMD::Objects_Controller* m_objects_controller = nullptr;
        const Spawn_Controller* m_spawn_controller = nullptr;

    private:
        float m_movement_speed = 0.0f;
        float m_attack_distance = 0.0f;
        float m_attack_distance_squared = 0.0f;
        float m_attack_damage = 0.0f;
        unsigned int m_attack_frame = 0;

        float m_min_stagger = 0.0f;
        float m_effective_stagger = 0.0f;
        float m_stagger_duration = 0.0f;

        float m_expected_attack_frequency = 0.0f;
        float m_min_distance_to_teleport = 0.0f;
        float m_min_distance_to_teleport_squared = 0.0f;

        LMD::Draw_Module__Vertex_Animation* m_draw_module__walk = nullptr;
        LMD::Draw_Module__Vertex_Animation* m_draw_module__attack = nullptr;
        LMD::Draw_Module__Vertex_Animation* m_draw_module__stagger = nullptr;

    private:
        LGL::Node_Base<>* m_behavior_tree = nullptr;

        LEti::Object* m_target = nullptr;
        LST::Timer m_target_search_timer;

        LST::Timer m_stagger_timer;

        LST::Message_Translator::Handle<Message__On_Entity_Death> m_message_handle__death;
        LST::Message_Translator::Handle<Message__On_Entity_Damaged> m_message_handle__damage;
        LST::Message_Translator::Handle<Message__Player_Evades> m_message_handle__player_evades;

        glm::vec3 m_movement_direction = {0.0f, 0.0f, 0.0f};
        glm::vec3 m_stored_position = {0.0f, 0.0f, 0.0f};
        LST::Timer m_movement_check_timer;
        LST::Timer m_no_attack_timer;

        Velocity_Module* m_velocity_module = nullptr;

        LR::Draw_Module* m_current_draw_module = nullptr;

        bool m_dealt_damage_this_attack = false;
        bool m_lost_target = false;

    public:
        Crystalis_AI();
        ~Crystalis_AI();

    public:
        inline void inject_objects_controller(LMD::Objects_Controller* _ptr) { m_objects_controller = _ptr; }
        inline void inject_spawn_controller(const Spawn_Controller* _ptr) { m_spawn_controller = _ptr; }

        inline void set_movement_speed(float _value) { m_movement_speed = _value; }
        inline void set_attack_distance(float _value) { m_attack_distance = _value; m_attack_distance_squared = _value * _value; }
        inline void set_attack_damage(float _value) { m_attack_damage = _value; }
        inline void set_attack_frame(unsigned int _value) { m_attack_frame = _value; }

        inline void set_min_stagger(unsigned int _value) { m_min_stagger = _value; }
        inline void set_effective_stagger(unsigned int _value) { m_effective_stagger = _value; }
        inline void set_stagger_duration(unsigned int _value) { m_stagger_duration = _value; }

        inline void set_expected_attack_frequency(float _value) { m_expected_attack_frequency = _value; }
        inline void set_min_distance_to_teleport(float _value) { m_min_distance_to_teleport = _value; m_min_distance_to_teleport_squared = _value * _value; m_no_attack_timer.start(_value); }

        void set_draw_module__walk(LMD::Draw_Module__Vertex_Animation* _ptr);
        void set_draw_module__attack(LMD::Draw_Module__Vertex_Animation* _ptr);
        void set_draw_module__stagger(LMD::Draw_Module__Vertex_Animation* _ptr);

    private:
        void M_construct_behavior_tree();

        LGL::BT_Execution_Result M_reset();
        LGL::BT_Execution_Result M_wait_for_stagger_end();
        LGL::BT_Execution_Result M_teleport_to_target();
        LGL::BT_Execution_Result M_find_target();
        LGL::BT_Execution_Result M_attack_target();
        LGL::BT_Execution_Result M_follow_target();

        void M_select_draw_module(LR::Draw_Module* _module);

        void M_move(float _dt);
        void M_jump_if_stuck(float _dt);
        void M_rotate_to_face_target();
        void M_deal_damage_to_target();

        bool M_is_attacking() const;
        bool M_is_target_in_damage_area() const;
        float M_calculate_stagger_duration(float _stagger) const;

    private:
        void M_on_parent_object_set() override;

    public:
        void update(float _dt) override;

    };


    class Crystalis_AI_Stub : public LEti::Module_Stub
    {
    public:
        INIT_VARIABLE(Shardis::Crystalis_AI_Stub, LEti::Module_Stub)

        INIT_FIELDS
        ADD_FIELD(float, movement_speed)
        ADD_FIELD(float, attack_distance)
        ADD_FIELD(float, attack_damage)
        ADD_FIELD(unsigned int, attack_frame)
        ADD_FIELD(float, min_stagger)
        ADD_FIELD(float, effective_stagger)
        ADD_FIELD(float, stagger_duration)
        ADD_FIELD(float, expected_attack_frequency)
        ADD_FIELD(float, min_distance_to_teleport)
        FIELDS_END

        INIT_CHILDS
        ADD_CHILD("draw_module__walk", draw_module__walk)
        ADD_CHILD("draw_module__attack", draw_module__attack)
        ADD_CHILD("draw_module__stagger", draw_module__stagger)
        CHILDS_END

    public:
        LMD::Objects_Controller* objects_controller = nullptr;
        LST::Function<const Spawn_Controller*()> spawn_controller_getter;

    public:
        float movement_speed = 0.0f;
        float attack_distance = 0.0f;
        float attack_damage = 0.0f;
        unsigned int attack_frame = 0;

        float min_stagger = 0.0f;
        float effective_stagger = 0.0f;
        float stagger_duration = 0.0f;

        float expected_attack_frequency = 0.0f;
        float min_distance_to_teleport = 0.0f;

    public:
        LMD::Draw_Module_Stub__Vertex_Animation* draw_module__walk = nullptr;
        LMD::Draw_Module_Stub__Vertex_Animation* draw_module__attack = nullptr;
        LMD::Draw_Module_Stub__Vertex_Animation* draw_module__stagger = nullptr;

    public:
        INIT_BUILDER_STUB(Crystalis_AI)

    public:
        ~Crystalis_AI_Stub();

    };

}
