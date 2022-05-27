#include "Space_Splitter_3D.h"

using namespace LEti;



std::list<const Object_3D*> Space_Splitter_3D::m_registred_models;

unsigned int Space_Splitter_3D::m_max_tree_depth = 100;
LEti::Tree<Space_Splitter_3D::Area, 8> Space_Splitter_3D::m_quad_tree;

std::list<Space_Splitter_3D::Collision_Data> Space_Splitter_3D::m_collisions;


bool Space_Splitter_3D::Area::point_is_inside(const glm::vec3 &_point) const
{
	bool result = true;
	if(!left.inf) result = result && left.value < _point.x;
	if(!right.inf) result = result && right.value > _point.x;
	if(!top.inf) result = result && top.value > _point.y;
	if(!bottom.inf) result = result && bottom.value < _point.y;
	if(!close.inf) result = result && close.value > _point.z;
	if(!far.inf) result = result && far.value < _point.z;
	return result;
}

bool Space_Splitter_3D::Area::model_is_inside(const Object_3D* _object) const
{
	const Physical_Model_3D* _model = (const Physical_Model_3D*)(_object->get_physical_model());
	Physical_Model_3D::Volumetric_Rectangular_Border rb = _model->construct_volumetric_rectangular_border();

	if(point_is_inside({rb.left, rb.top, rb.close}) || point_is_inside({rb.left, rb.bottom, rb.close})
			|| point_is_inside({rb.right, rb.top, rb.close}) || point_is_inside({rb.right, rb.bottom, rb.close})
			|| point_is_inside({rb.left, rb.top, rb.far}) || point_is_inside({rb.left, rb.bottom, rb.far})
			|| point_is_inside({rb.right, rb.top, rb.far}) || point_is_inside({rb.right, rb.bottom, rb.far}))
		return true;

	const Physical_Model_3D& pm = *((const Physical_Model_3D*)_object->get_physical_model());

	for(unsigned int pyr=0; pyr<pm.get_pyramids_count(); ++pyr)
		for(unsigned int pol=0; pol < 4; ++pol)
			for(unsigned int vert=0; vert < 3; ++vert)
				if(point_is_inside(pm[pyr][pol][vert])) return true;
	return false;
}


std::pair<const Object_3D*, glm::vec3> Space_Splitter_3D::Area::get_point_to_split() const
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

		if(found) return { *it, result };
		++it;
	}

	return {nullptr, {}};
}



void Space_Splitter_3D::set_max_tree_depth(unsigned int _max_depth)
{
	m_max_tree_depth = _max_depth;
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



void Space_Splitter_3D::split_space_recursive(LEti::Tree<Area, 8>::Iterator _it, unsigned int _level)
{
	const auto& models = _it->models;
	if (models.size() < 3 || _level > m_max_tree_depth) return;

	std::pair<const Object_3D*, glm::vec3> split_data = _it->get_point_to_split();
	glm::vec3 split_point = split_data.second;

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
		next->models.push_back(split_data.first);

		std::list<const Object_3D*>::const_iterator model_it = models.begin();
		while(model_it != models.end())
		{
			if(*model_it == split_data.first) { ++model_it; continue; }
			if(next->model_is_inside(*model_it))
				next->models.push_back(*model_it);
			++model_it;
		}
		split_space_recursive(next, _level + 1);
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

	split_space_recursive(it, 0);

	while(!it.end())
	{
		if(it->models.size() == 2)
		{
			std::list<const Object_3D*>::iterator colliding_model = it->models.begin();
			std::list<const Object_3D*>::iterator next = colliding_model;
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
		}
		++it;
	}
	if(it->models.size() == 2)
	{
		std::list<const Object_3D*>::iterator colliding_model = it->models.begin();
		std::list<const Object_3D*>::iterator next = colliding_model;
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
	}
}



std::list<Space_Splitter_3D::Collision_Data> Space_Splitter_3D::get_collisions()
{
	return m_collisions;
}
