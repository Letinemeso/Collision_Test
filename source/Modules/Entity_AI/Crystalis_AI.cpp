#include <Modules/Entity_AI/Crystalis_AI.h>

#include <Math_Stuff.h>

#include <Modules/Health_Module.h>
#include <Modules/Type_Module.h>
#include <Tools/Entity_Tools.h>

using namespace Shardis;

namespace Shardis
{
    constexpr float Mid_Air_Movement_Speed_Multiplier = 0.1f;

    constexpr float Target_Search_Frequency = 1.0f;
    constexpr float Approach_Distance_Multiplier = 0.8f;
    constexpr float Knockback_Force = 0.3f;

    constexpr float Movement_Check_Delay = 0.1f;
    constexpr float Expected_Movement_Per_Check = 0.1f;
    constexpr float Expected_Movement_Squared_Per_Check = Expected_Movement_Per_Check * Expected_Movement_Per_Check;
    constexpr float Jump_Impulse_Strength = 0.4f;
}


Crystalis_AI::Crystalis_AI()
{
    m_message_handle__death = LST::Message_Translator::instance().subscribe<Message__On_Entity_Death>([this](Message__On_Entity_Death& _msg)
    {
        if(_msg.almost_dead_entity == m_target)
            m_target = nullptr;
    });

    m_message_handle__damage = LST::Message_Translator::instance().subscribe<Message__On_Entity_Damaged>([this](Message__On_Entity_Damaged& _msg)
    {
        if(_msg.damaged_entity != parent_object())
            return;

        M_reset();

        float stagger_duration = M_calculate_stagger_duration(_msg.stagger);
        m_stagger_timer.start(stagger_duration);
    });

    m_message_handle__player_evades = LST::Message_Translator::instance().subscribe<Message__Player_Evades>([this](Message__Player_Evades& _msg)
    {
        if(!m_target)
            return;

        if(M_is_attacking())
            m_lost_target = true;
    });

    M_construct_behavior_tree();

    m_no_attack_timer.start(m_expected_attack_frequency);
}

Crystalis_AI::~Crystalis_AI()
{
    LST::Message_Translator::instance().unsubscribe(m_message_handle__death);
    LST::Message_Translator::instance().unsubscribe(m_message_handle__damage);
    LST::Message_Translator::instance().unsubscribe(m_message_handle__player_evades);

    delete m_behavior_tree;

    delete m_draw_module__walk;
    delete m_draw_module__attack;
    delete m_draw_module__stagger;
}



void Crystalis_AI::set_draw_module__walk(LMD::Draw_Module__Vertex_Animation* _ptr)
{
    m_draw_module__walk = _ptr;
    m_draw_module__walk->set_visible(false);
}

void Crystalis_AI::set_draw_module__attack(LMD::Draw_Module__Vertex_Animation* _ptr)
{
    m_draw_module__attack = _ptr;
    m_draw_module__attack->set_visible(false);
}

void Crystalis_AI::set_draw_module__stagger(LMD::Draw_Module__Vertex_Animation* _ptr)
{
    m_draw_module__stagger = _ptr;
    m_draw_module__stagger->set_visible(false);
}



void Crystalis_AI::M_construct_behavior_tree()
{
    LGL::Sequence<>* main_sequence = new LGL::Sequence<>;
    m_behavior_tree = main_sequence;

    LGL::Action<>* reset = new LGL::Action<>([this]()
    {
        return M_reset();
    });
    main_sequence->add_child(reset);

    LGL::Action<>* wait_for_stagger_end = new LGL::Action<>([this]()
    {
        return M_wait_for_stagger_end();
    });
    main_sequence->add_child(wait_for_stagger_end);

    LGL::Action<>* find_target = new LGL::Action<>([this]()
    {
        return M_find_target();
    });
    main_sequence->add_child(find_target);

    LGL::Selector<>* action_selector = new LGL::Selector<>;
    main_sequence->add_child(action_selector);
    {

        LGL::Action<>* teleport_to_target = new LGL::Action<>([this]()
        {
            return M_teleport_to_target();
        });
        action_selector->add_child(teleport_to_target);

        LGL::Action<>* attack_target = new LGL::Action<>([this]()
        {
            return M_attack_target();
        });
        action_selector->add_child(attack_target);

        LGL::Action<>* follow_target = new LGL::Action<>([this]()
        {
            return M_follow_target();
        });
        action_selector->add_child(follow_target);
    }
}


LGL::BT_Execution_Result Crystalis_AI::M_reset()
{
    m_movement_direction = {0.0f, 0.0f, 0.0f};
    return LGL::BT_Execution_Result::Success;
}

LGL::BT_Execution_Result Crystalis_AI::M_wait_for_stagger_end()
{
    if(!m_stagger_timer.is_active())
        return LGL::BT_Execution_Result::Success;

    if(m_current_draw_module == m_draw_module__stagger)
        return LGL::BT_Execution_Result::In_Progress;

    M_select_draw_module(m_draw_module__stagger);
    m_draw_module__stagger->start();

    return LGL::BT_Execution_Result::In_Progress;
}

LGL::BT_Execution_Result Crystalis_AI::M_teleport_to_target()
{
    if(!m_target)
        return LGL::BT_Execution_Result::Fail;

    if(M_is_attacking())
        return LGL::BT_Execution_Result::Fail;

    if(m_no_attack_timer.is_active())
        return LGL::BT_Execution_Result::Fail;

    m_no_attack_timer.start(m_expected_attack_frequency);

    const glm::vec3& target_position = m_target->current_state().position();

    glm::vec3 vec_to_target = target_position - transformation_data()->position();
    if(LEti::Math::vector_length_squared(vec_to_target) < m_min_distance_to_teleport_squared)
        return LGL::BT_Execution_Result::Fail;

    L_ASSERT(m_spawn_controller);

    glm::vec new_position = m_spawn_controller->find_available_position(target_position);
    new_position.y += 2.0f;

    transformation_data()->set_position(new_position);
    parent_object()->update_previous_transformation_data();

    return LGL::BT_Execution_Result::Success;
}

LGL::BT_Execution_Result Crystalis_AI::M_find_target()
{
    if(m_target)
        return LGL::BT_Execution_Result::Success;

    L_ASSERT(m_objects_controller);

    if(m_target_search_timer.is_active())
        return LGL::BT_Execution_Result::Fail;

    static const LST::Function<bool(LEti::Object*)> condition = [](LEti::Object* _object)
    {
        Type_Module* type_module = _object->get_module_of_type<Type_Module>();
        if(!type_module)
            return false;

        return type_module->object_type() == Object_Type::Player;
    };

    m_target = m_objects_controller->get_suitable_object(condition);

    m_target_search_timer.start(Target_Search_Frequency);

    if(m_target)
        return LGL::BT_Execution_Result::Success;
    return LGL::BT_Execution_Result::Fail;
}

LGL::BT_Execution_Result Crystalis_AI::M_attack_target()
{
    if(M_is_attacking() && !m_draw_module__attack->paused())
        return LGL::BT_Execution_Result::In_Progress;

    glm::vec3 vector_between = m_target->current_state().position() - transformation_data()->position();
    float distance_squared = LEti::Math::vector_length_squared(vector_between);

    if(distance_squared > m_attack_distance_squared * Approach_Distance_Multiplier)
        return LGL::BT_Execution_Result::Fail;

    m_draw_module__attack->start(1);
    M_select_draw_module(m_draw_module__attack);
    m_dealt_damage_this_attack = false;
    m_lost_target = false;

    m_no_attack_timer.start(m_expected_attack_frequency);

    return LGL::BT_Execution_Result::Success;
}

LGL::BT_Execution_Result Crystalis_AI::M_follow_target()
{
    L_ASSERT(m_target);

    glm::vec3 direction = m_target->current_state().position() - transformation_data()->position();
    direction.y = 0.0f;
    if(LEti::Math::vector_length_squared(direction) < 0.0001f)
        return LGL::BT_Execution_Result::Fail;

    LEti::Math::shrink_vector_to_1(direction);

    m_movement_direction = direction;

    M_select_draw_module(m_draw_module__walk);

    return LGL::BT_Execution_Result::Success;
}


void Crystalis_AI::M_select_draw_module(LR::Draw_Module* _module)
{
    if(m_current_draw_module)
        m_current_draw_module->set_visible(false);

    m_current_draw_module = _module;

    if(m_current_draw_module)
        m_current_draw_module->set_visible(true);
}


void Crystalis_AI::M_move(float _dt)
{
    if(LEti::Math::vector_length_squared(m_movement_direction) < 0.0001f)
        return;

    glm::vec3 additional_impulse = m_movement_direction * m_movement_speed * _dt;
    if(!m_velocity_module->is_grounded())
        additional_impulse *= Mid_Air_Movement_Speed_Multiplier;

    glm::vec3 new_impulse = calculate_impulse(m_velocity_module->impulse(), additional_impulse, m_movement_speed, 10.0f);
    m_velocity_module->set_impulse(new_impulse);
}

void Crystalis_AI::M_jump_if_stuck(float _dt)
{
    if(!m_target)
        return;

    if(m_current_draw_module != m_draw_module__walk)
        return;

    m_movement_check_timer.update(_dt);
    if(m_movement_check_timer.is_active())
        return;

    m_movement_check_timer.start(Movement_Check_Delay);

    if(!m_velocity_module->is_grounded())
        return;

    const glm::vec3& current_position = transformation_data()->position();
    float traversed_distance_squared = LEti::Math::vector_length_squared(current_position - m_stored_position);

    m_stored_position = current_position;

    if(traversed_distance_squared > Expected_Movement_Squared_Per_Check)
        return;

    glm::vec3 jump_impulse = {0.0f, Jump_Impulse_Strength, 0.0f};
    m_velocity_module->add_impulse(jump_impulse);
    m_velocity_module->mark_grounded(false);
}

void Crystalis_AI::M_rotate_to_face_target()
{
    if(!m_target)
        return;

    if(m_stagger_timer.is_active())
        return;

    if(M_is_attacking() && m_lost_target)
        return;

    constexpr glm::vec3 Top_Vector = {0.0f, 1.0f, 0.0f};

    glm::vec3 vec_to_target = m_target->current_state().position() - transformation_data()->position();
    vec_to_target.y = 0.0f;
    LEti::Math::shrink_vector_to_1(vec_to_target);

    if(LEti::Math::vector_length_squared(vec_to_target) < 1e-6f)
        return;

    glm::vec3 rotation = LEti::Math::calculate_angles(vec_to_target, Top_Vector);
    rotation.y = LEti::Math::DOUBLE_PI - rotation.y;

    transformation_data()->set_rotation(rotation);
}

void Crystalis_AI::M_deal_damage_to_target()
{
    if(!m_target)
        return;

    if(m_dealt_damage_this_attack)
        return;

    if(m_current_draw_module != m_draw_module__attack)
        return;

    if(m_draw_module__attack->current_frame() < m_attack_frame)
        return;

    m_dealt_damage_this_attack = true;

    if(!M_is_target_in_damage_area())
        return;

    Health_Module* health_module = m_target->get_module_of_type<Health_Module>();
    L_ASSERT(health_module);

    health_module->receive_damage(m_attack_damage, 0.0f, parent_object());

    Velocity_Module* target_velocity_module = m_target->get_module_of_type<Velocity_Module>();
    L_ASSERT(target_velocity_module);

    glm::vec3 vec_to_target = m_target->current_state().position() - transformation_data()->position();
    vec_to_target.y = 0.0f;
    LEti::Math::extend_vector_to_length(vec_to_target, Knockback_Force);

    target_velocity_module->add_impulse(vec_to_target);
}


bool Crystalis_AI::M_is_attacking() const
{
    return m_current_draw_module == m_draw_module__attack;
}

bool Crystalis_AI::M_is_target_in_damage_area() const
{
    glm::vec3 vec_to_target = m_target->current_state().position() - transformation_data()->position();
    if(LEti::Math::vector_length_squared(vec_to_target) > m_attack_distance_squared)
        return false;

    vec_to_target.y = 0.0f;
    LEti::Math::shrink_vector_to_1(vec_to_target);

    constexpr glm::vec3 Default_Direction = {0.0f, 0.0f, -1.0f};
    glm::vec3 look_direction = transformation_data()->rotation_matrix() * glm::vec4(Default_Direction, 1.0f);

    constexpr float Min_Projection_Value = 0.8f;
    float projection = LEti::Math::dot_product(look_direction, vec_to_target);
    if(projection < Min_Projection_Value)
        return false;

    return true;
}

float Crystalis_AI::M_calculate_stagger_duration(float _stagger) const
{
    if(_stagger < m_min_stagger)
        return 0.0f;
    if(_stagger >= m_effective_stagger)
        return m_stagger_duration;

    float effective_stagger_diff = m_effective_stagger - m_min_stagger;
    _stagger -= m_min_stagger;

    float stagger_ratio = _stagger / effective_stagger_diff;
    return m_stagger_duration * stagger_ratio;
}



void Crystalis_AI::M_on_parent_object_set()
{
    Parent_Type::M_on_parent_object_set();

    m_velocity_module = parent_object()->get_module_of_type<Velocity_Module>();
    L_ASSERT(m_velocity_module);

    m_draw_module__walk->set_transformation_data(transformation_data());
    m_draw_module__walk->set_transformation_data_prev_state(transformation_data_prev_state());
    m_draw_module__walk->set_parent_object(parent_object());
    m_draw_module__attack->set_transformation_data(transformation_data());
    m_draw_module__attack->set_transformation_data_prev_state(transformation_data_prev_state());
    m_draw_module__attack->set_parent_object(parent_object());
    m_draw_module__stagger->set_transformation_data(transformation_data());
    m_draw_module__stagger->set_transformation_data_prev_state(transformation_data_prev_state());
    m_draw_module__stagger->set_parent_object(parent_object());
}



void Crystalis_AI::update(float _dt)
{
    m_target_search_timer.update(_dt);
    m_stagger_timer.update(_dt);
    m_no_attack_timer.update(_dt);

    m_behavior_tree->process();

    M_move(_dt);
    M_jump_if_stuck(_dt);
    M_rotate_to_face_target();
    M_deal_damage_to_target();

    if(m_current_draw_module)
        m_current_draw_module->update(_dt);
}





BUILDER_STUB_DEFAULT_CONSTRUCTION_FUNC(Crystalis_AI_Stub)

BUILDER_STUB_INITIALIZATION_FUNC(Crystalis_AI_Stub)
{
    BUILDER_STUB_PARENT_INITIALIZATION;
    BUILDER_STUB_CAST_PRODUCT;

    L_ASSERT(objects_controller);
    L_ASSERT(spawn_controller_getter);

    L_ASSERT(movement_speed > 0.0f);

    L_ASSERT(draw_module__walk);
    L_ASSERT(draw_module__attack);

    product->inject_objects_controller(objects_controller);
    product->inject_spawn_controller(spawn_controller_getter());

    product->set_movement_speed(movement_speed);
    product->set_attack_distance(attack_distance);
    product->set_attack_damage(attack_damage);
    product->set_attack_frame(attack_frame);

    product->set_min_stagger(min_stagger);
    product->set_effective_stagger(effective_stagger);
    product->set_stagger_duration(stagger_duration);

    product->set_expected_attack_frequency(expected_attack_frequency);
    product->set_min_distance_to_teleport(min_distance_to_teleport);

    product->set_draw_module__walk( LMD::Draw_Module_Stub__Vertex_Animation::construct_from(draw_module__walk) );
    product->set_draw_module__attack( LMD::Draw_Module_Stub__Vertex_Animation::construct_from(draw_module__attack) );
    product->set_draw_module__stagger( LMD::Draw_Module_Stub__Vertex_Animation::construct_from(draw_module__stagger) );
}



Crystalis_AI_Stub::~Crystalis_AI_Stub()
{
    delete draw_module__walk;
    delete draw_module__attack;
    delete draw_module__stagger;
}
