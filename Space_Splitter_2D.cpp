#include "Space_Splitter_2D.h"

using namespace LEti;


#include "Debug_Drawable_Frame.h"

Debug_Drawable_Frame* df = nullptr;
Object_2D* sp_ind = nullptr;

void LEti::init_frame(const char* _obj_name)
{
	df = new Debug_Drawable_Frame;
	df->init("debug_frame");

	sp_ind = new Object_2D;
	sp_ind->init("flat_indicator_red");
}

std::list<const Object_2D*> Space_Splitter_2D::m_registred_models;

unsigned int Space_Splitter_2D::m_max_tree_depth = 100;
LEti::Tree<Space_Splitter_2D::Area, 4> Space_Splitter_2D::m_quad_tree;

std::list<Space_Splitter_2D::Collision_Data> Space_Splitter_2D::m_collisions;


bool Space_Splitter_2D::Area::point_is_inside(const glm::vec3 &_point) const
{
	bool result = true;
	if(!left.inf) result = result && left.value < _point.x;
	if(!right.inf) result = result && right.value > _point.x;
	if(!top.inf) result = result && top.value > _point.y;
	if(!bottom.inf) result = result && bottom.value < _point.y;
	return result;
}

bool Space_Splitter_2D::Area::model_is_inside(const Object_2D* _object) const
{
	const Physical_Model_2D* _model = (const Physical_Model_2D*)(_object->get_physical_model());
	Physical_Model_2D::Rectangular_Border rb = _model->construct_rectangular_border();

	return point_is_inside({rb.left, rb.top, 0.0f}) || point_is_inside({rb.left, rb.bottom, 0.0f})
			|| point_is_inside({rb.right, rb.top, 0.0f}) || point_is_inside({rb.right, rb.bottom, 0.0f});
}


std::pair<const Object_2D*, glm::vec3> Space_Splitter_2D::Area::get_point_to_split() const
{
	glm::vec3 result;

	std::list<const Object_2D*>::iterator it = m_registred_models.begin();
	while(it != m_registred_models.end())
	{
		const Physical_Model_2D* ptr = (const Physical_Model_2D*)((*it)->get_physical_model());
		Physical_Model_2D::Rectangular_Border rb = ptr->construct_rectangular_border();
		bool found = true;

		if(point_is_inside({rb.left, rb.top, 0.0f})) result = {rb.left, rb.top, 0.0f};
		else if(point_is_inside({rb.left, rb.bottom, 0.0f})) result = {rb.left, rb.bottom, 0.0f};
		else if(point_is_inside({rb.right, rb.top, 0.0f})) result = {rb.right, rb.top, 0.0f};
		else if(point_is_inside({rb.right, rb.bottom, 0.0f})) result = {rb.right, rb.bottom, 0.0f};
		else found = false;

		if(found) break;
		++it;
	}

	sp_ind->set_pos(result.x, result.y, result.z);
	sp_ind->draw();

	return {*it, result};
}



void Space_Splitter_2D::set_max_tree_depth(unsigned int _max_depth)
{
	m_max_tree_depth = _max_depth;
}


void Space_Splitter_2D::register_object(const Object_2D *_model)
{
	std::list<const Object_2D*>::iterator check = m_registred_models.begin();
	while(check != m_registred_models.end())
	{
		ASSERT(*check == _model);
		++check;
	}

	m_registred_models.push_back(_model);
}

void Space_Splitter_2D::unregister_object(const Object_2D *_model)
{
	std::list<const Object_2D*>::iterator it = m_registred_models.begin();
	while(it != m_registred_models.end())
	{
		if(*it == _model) break;
		++it;
	}
	ASSERT(it == m_registred_models.end());
	m_registred_models.erase(it);
}



void Space_Splitter_2D::split_space_recursive(LEti::Tree<Area, 4>::Iterator _it, unsigned int _level)
{
	const auto& models = _it->models;
	if (models.size() < 3 || _level > m_max_tree_depth) return;

	std::pair<const Object_2D*, glm::vec3> split_point_pair = _it->get_point_to_split();
	glm::vec3 split_point = split_point_pair.second;
	float x = split_point.x;
	float y = split_point.y;

	Debug_Drawable_Frame& dfl = *df;
	_it.insert_into_availible_index({_it->left, split_point.x, _it->top, split_point.y});
	_it.insert_into_availible_index({split_point.x, _it->right, _it->top, split_point.y});
	_it.insert_into_availible_index({split_point.x, _it->right, split_point.y, _it->bottom});
	_it.insert_into_availible_index({_it->left, split_point.x, split_point.y, _it->bottom});


	for(unsigned int i=0; i<4; ++i)
	{
		LEti::Tree<Area, 4>::Iterator next = _it;
		next.descend(i);
		next->models.push_back(split_point_pair.first);

		float left = next->left.inf ? -1000 : next->left.value;
		float right = next->right.inf ? 1000 : next->right.value;
		float top = next->top.inf ? 1000 : next->top.value;
		float bottom = next->bottom.inf ? -1000 : next->bottom.value;

		dfl.clear_points();
		dfl.set_point(0, {left, top, 0.0f});
		dfl.set_point(1, {right, top, 0.0f});
		dfl.set_point(2, {right, bottom, 0.0f});
		dfl.set_point(3, {left, next->bottom.value, 0.0f});

		dfl.clear_sequence().set_sequence_element(0, 0).set_sequence_element(1, 1).set_sequence_element(2, 2).set_sequence_element(3, 3);

		dfl.update();
		dfl.draw();

		std::list<const Object_2D*>::const_iterator model_it = models.begin();
		while(model_it != models.end())
		{
			if(*model_it == split_point_pair.first) { ++model_it; continue; }
			if(next->model_is_inside(*model_it))
			{
//				dfl.clear_points().set_point(0, {next->left.value, next->top.value, 0.0f}).set_point(1, {next->right.value, next->top.value, 0.0f})
//						.set_point(2, {next->right.value, next->bottom.value, 0.0f}).set_point(3, {next->left.value, next->bottom.value, 0.0f});
//				dfl.clear_sequence().set_sequence_element(0, 0).set_sequence_element(1, 1).set_sequence_element(2, 2).set_sequence_element(3, 3);
//				dfl.update();
//				dfl.draw();

				next->models.push_back(*model_it);
			}
			++model_it;
		}

		split_space_recursive(next, _level + 1);
	}
}


void Space_Splitter_2D::update()
{
	m_collisions.clear();

	LEti::Tree<Area, 4>::Iterator it = m_quad_tree.create_iterator();
	if(it.valid()) it.delete_branch();
	it = m_quad_tree.create_iterator();

	it.insert_into_availible_index({{}, {}, {}, {}});

	std::list<const Object_2D*>::iterator model_it = m_registred_models.begin();
	while(model_it != m_registred_models.end())
	{
		it->models.push_back(*model_it);
		++model_it;
	}

	split_space_recursive(it, 0);

	while(!it.end())
	{
		if(it->models.size() == 2)
		{
//			std::pair<const Object_2D*, const Object_2D*> colliding_pair;
			std::list<const Object_2D*>::iterator colliding_model = it->models.begin();
			std::list<const Object_2D*>::iterator next = colliding_model;
			++next;
			Collision_Data cd(*colliding_model, *next, (*colliding_model)->is_colliding_with_other(*(*next)));
			if(cd.collision_data)
			{
				std::list<Collision_Data>::iterator it = m_collisions.begin();
				while(it != m_collisions.end())
				{
					if(*it == cd) break;
					++it;
				}
				if(it == m_collisions.end()) m_collisions.push_back(cd);
			}

//			colliding_pair.first = *colliding_model;
//			++colliding_model;
//			colliding_pair.second = *colliding_model;
//			Collision_Data cd(colliding_pair.first, colliding_pair.second, colliding_pair.first->is_colliding_with_other(*colliding_pair.second));
//			if(cd.collision_data)
//			{
//				std::list<Collision_Data>::iterator it = m_collisions.begin();
//				while(it != m_collisions.end())
//				{


//					if(*it == cd) break;
//					++it;
//				}
//				if(it == m_collisions.end()) m_collisions.push_back(cd);
//			}
		}
		++it;
	}
	if(it->models.size() == 2)
	{
//		std::pair<const Object_2D*, const Object_2D*> colliding_pair;
		std::list<const Object_2D*>::iterator colliding_model = it->models.begin();
		std::list<const Object_2D*>::iterator next = colliding_model;
		++next;
		Collision_Data cd(*colliding_model, *next, (*colliding_model)->is_colliding_with_other(*(*next)));
		if(cd.collision_data)
		{
			std::list<Collision_Data>::iterator it = m_collisions.begin();
			while(it != m_collisions.end())
			{
				if(*it == cd) break;
				++it;
			}
			if(it == m_collisions.end()) m_collisions.push_back(cd);
		}
//		colliding_pair.first = *colliding_model;
//		++colliding_model;
//		colliding_pair.second = *colliding_model;
//		Collision_Data cd(colliding_pair.first, colliding_pair.second, colliding_pair.first->is_colliding_with_other(*colliding_pair.second));
//		if(cd.collision_data)
//		{
//			std::list<Collision_Data>::iterator it = m_collisions.begin();
//			while(it != m_collisions.end())
//			{
//				if(*it == cd) break;
//				++it;
//			}
//			if(it == m_collisions.end()) m_collisions.push_back(cd);
//		}
	}
}



const std::list<Space_Splitter_2D::Collision_Data>& Space_Splitter_2D::get_collisions()
{
	return m_collisions;
}
