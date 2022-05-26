#include "Space_Splitter.h"

using namespace LEti;



std::list<const Object_2D*> Space_Splitter_2D::m_registred_models;
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

	split_space_recursive(it);

	while(!it.end())
	{
		if(it->models.size() == 2)
		{
			std::pair<const Object_2D*, const Object_2D*> colliding_pair;
			std::list<const Object_2D*>::iterator colliding_model = it->models.begin();
			colliding_pair.first = *colliding_model;
			++colliding_model;
			colliding_pair.second = *colliding_model;
			Collision_Data cd(colliding_pair.first, colliding_pair.second, colliding_pair.first->is_colliding_with_other(*colliding_pair.second));
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
		Collision_Data cd(colliding_pair.first, colliding_pair.second, colliding_pair.first->is_colliding_with_other(*colliding_pair.second));
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
	}
}



const std::list<Space_Splitter_2D::Collision_Data>& Space_Splitter_2D::get_collisions()
{
	return m_collisions;
}










std::list<const Object_3D*> Space_Splitter_3D::m_registred_models;
LEti::Tree<Space_Splitter_3D::Area, 8> Space_Splitter_3D::m_quad_tree;

std::list<Space_Splitter_3D::Collision_Data> Space_Splitter_3D::m_collisions;


bool Space_Splitter_3D::Area::point_is_inside(const glm::vec3 &_point) const
{
	bool result = true;
	if(!left.inf) result = result && left.value < _point.x;
	if(!right.inf) result = result && right.value > _point.x;
	if(!top.inf) result = result && top.value > _point.y;
	if(!bottom.inf) result = result && bottom.value < _point.y;
	if(!close.inf) result = result && close.value < _point.z;
	if(!far.inf) result = result && far.value > _point.z;
	return result;
}

bool Space_Splitter_3D::Area::model_is_inside(const Object_3D* _object) const
{
	const Physical_Model_3D* _model = (const Physical_Model_3D*)(_object->get_physical_model());
	Physical_Model_3D::Volumetric_Rectangular_Border rb = _model->construct_volumetric_rectangular_border();

	return point_is_inside({rb.left, rb.top, rb.close}) || point_is_inside({rb.left, rb.bottom, rb.close})
			|| point_is_inside({rb.right, rb.top, rb.close}) || point_is_inside({rb.right, rb.bottom, rb.close})
			|| point_is_inside({rb.left, rb.top, rb.far}) || point_is_inside({rb.left, rb.bottom, rb.far})
			|| point_is_inside({rb.right, rb.top, rb.far}) || point_is_inside({rb.right, rb.bottom, rb.far});
}


glm::vec3 Space_Splitter_3D::Area::get_point_to_split() const
{
	glm::vec3 result;

	std::list<const Object_3D*>::iterator it = m_registred_models.begin();
	while(it != m_registred_models.end())
	{
		const Physical_Model_3D* ptr = (const Physical_Model_3D*)((*it)->get_physical_model());
		Physical_Model_3D::Volumetric_Rectangular_Border rb = ptr->construct_volumetric_rectangular_border();
		bool found = true;

		if(point_is_inside({rb.left, rb.top, rb.far})) result = {rb.left, rb.top, rb.far};
		else if(point_is_inside({rb.left, rb.bottom, rb.far})) result = {rb.left, rb.bottom, rb.far};
		else if(point_is_inside({rb.right, rb.top, rb.far})) result = {rb.right, rb.top, rb.far};
		else if(point_is_inside({rb.right, rb.bottom, rb.far})) result = {rb.right, rb.bottom, rb.far};
		else if(point_is_inside({rb.left, rb.top, rb.close})) result = {rb.left, rb.top, rb.close};
		else if(point_is_inside({rb.left, rb.bottom, rb.close})) result = {rb.left, rb.bottom, rb.close};
		else if(point_is_inside({rb.right, rb.top, rb.close})) result = {rb.right, rb.top, rb.close};
		else if(point_is_inside({rb.right, rb.bottom, rb.close})) result = {rb.right, rb.bottom, rb.close};
		else found = false;

		if(found) break;
		++it;
	}

	return result;
}



void Space_Splitter_3D::register_object(const Object_3D *_model)
{
	std::list<const Object_3D*>::iterator check = m_registred_models.begin();
	while(check != m_registred_models.end())
	{
		ASSERT(*check == _model);
		++check;
	}

	m_registred_models.push_back(_model);
}

void Space_Splitter_3D::unregister_object(const Object_3D *_model)
{
	std::list<const Object_3D*>::iterator it = m_registred_models.begin();
	while(it != m_registred_models.end())
	{
		if(*it == _model) break;
		++it;
	}
	ASSERT(it == m_registred_models.end());
	m_registred_models.erase(it);
}



void Space_Splitter_3D::split_space_recursive(LEti::Tree<Area, 8>::Iterator _it)
{
	const auto& models = _it->models;
	if (models.size() < 3) return;

	glm::vec3 split_point = _it->get_point_to_split();

	_it.insert_into_availible_index({_it->left, split_point.x, _it->top, split_point.y, split_point.z, _it->close});
	_it.insert_into_availible_index({split_point.x, _it->right, _it->top, split_point.y, split_point.z, _it->close});
	_it.insert_into_availible_index({split_point.x, _it->right, split_point.y, _it->bottom, split_point.z, _it->close});
	_it.insert_into_availible_index({_it->left, split_point.x, split_point.y, _it->bottom, split_point.z, _it->close});

	_it.insert_into_availible_index({_it->left, split_point.x, _it->top, split_point.y, _it->far, split_point.z});
	_it.insert_into_availible_index({split_point.x, _it->right, _it->top, split_point.y, _it->far, split_point.z});
	_it.insert_into_availible_index({split_point.x, _it->right, split_point.y, _it->bottom, _it->far, split_point.z});
	_it.insert_into_availible_index({_it->left, split_point.x, split_point.y, _it->bottom, _it->far, split_point.z});

	for(unsigned int i=0; i<8; ++i)
	{
		LEti::Tree<Area, 8>::Iterator next = _it;
		next.descend(i);

		std::list<const Object_3D*>::const_iterator model_it = models.begin();
		while(model_it != models.end())
		{
			if(next->model_is_inside(*model_it))
				next->models.push_back(*model_it);
			++model_it;
		}

		split_space_recursive(next);
	}
}


void Space_Splitter_3D::update()
{
	m_collisions.clear();

	LEti::Tree<Area, 8>::Iterator it = m_quad_tree.create_iterator();
	if(it.valid()) it.delete_branch();
	it = m_quad_tree.create_iterator();

	it.insert_into_availible_index({{}, {}, {}, {}, {}, {}});

	std::list<const Object_3D*>::iterator model_it = m_registred_models.begin();
	while(model_it != m_registred_models.end())
	{
		it->models.push_back(*model_it);
		++model_it;
	}

	split_space_recursive(it);

	while(!it.end())
	{
		if(it->models.size() == 2)
		{
			std::pair<const Object_3D*, const Object_3D*> colliding_pair;
			std::list<const Object_3D*>::iterator colliding_model = it->models.begin();
			colliding_pair.first = *colliding_model;
			++colliding_model;
			colliding_pair.second = *colliding_model;
			Collision_Data cd(colliding_pair.first, colliding_pair.second, colliding_pair.first->is_colliding_with_other(*colliding_pair.second));
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
		}
		++it;
	}
	if(it->models.size() == 2)
	{
		std::pair<const Object_3D*, const Object_3D*> colliding_pair;
		std::list<const Object_3D*>::iterator colliding_model = it->models.begin();
		colliding_pair.first = *colliding_model;
		++colliding_model;
		colliding_pair.second = *colliding_model;
		Collision_Data cd(colliding_pair.first, colliding_pair.second, colliding_pair.first->is_colliding_with_other(*colliding_pair.second));
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
	}
}



std::list<Space_Splitter_3D::Collision_Data> Space_Splitter_3D::get_collisions()
{
	return m_collisions;
}
