#include "Space_Splitter.h"

using namespace LEti;



std::list<const Object_2D*> Space_Splitter_2D::m_registred_models;
LEti::Tree<Space_Splitter_2D::Area, 4> Space_Splitter_2D::m_quad_tree;


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


glm::vec3 Space_Splitter_2D::Area::get_point_to_split() const
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

	return result;
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



void Space_Splitter_2D::split_space_recursive(LEti::Tree<Area, 4>::Iterator _it)
{
	const auto& models = _it->models;
	if (models.size() < 3) return;

	glm::vec3 split_point = _it->get_point_to_split();

	_it.insert_into_availible_index({_it->left, split_point.x, _it->top, split_point.y});
	_it.insert_into_availible_index({split_point.x, _it->right, _it->top, split_point.y});
	_it.insert_into_availible_index({split_point.x, _it->right, split_point.y, _it->bottom});
	_it.insert_into_availible_index({_it->left, split_point.x, split_point.y, _it->bottom});

	for(unsigned int i=0; i<4; ++i)
	{
		LEti::Tree<Area, 4>::Iterator next = _it;
		next.descend(i);

		std::list<const Object_2D*>::const_iterator model_it = models.begin();
		while(model_it != models.end())
		{
			if(next->model_is_inside(*model_it))
				next->models.push_back(*model_it);
			++model_it;
		}

		split_space_recursive(next);
	}
}


std::list<std::pair<const Object_2D*, const Object_2D*>> Space_Splitter_2D::get_possible_collisions()
{
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

	split_space_recursive(it);

	std::list<std::pair<const Object_2D*, const Object_2D*>> result;
	while(!it.end())
	{
		if(it->models.size() == 2)
		{
			std::pair<const Object_2D*, const Object_2D*> colliding_pair;
			std::list<const Object_2D*>::iterator colliding_model = it->models.begin();
			colliding_pair.first = *colliding_model;
			++colliding_model;
			colliding_pair.second = *colliding_model;
			result.push_back(colliding_pair);
		}
		++it;
	}
	if(it->models.size() == 2)
	{
		std::pair<const Object_2D*, const Object_2D*> colliding_pair;
		std::list<const Object_2D*>::iterator colliding_model = it->models.begin();
		colliding_pair.first = *colliding_model;
		++colliding_model;
		colliding_pair.second = *colliding_model;
		result.push_back(colliding_pair);
	}

	return result;
}
