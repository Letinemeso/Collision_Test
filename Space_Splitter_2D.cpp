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
std::list<Space_Splitter_2D::Point> Space_Splitter_2D::m_models_points;

unsigned int Space_Splitter_2D::m_max_tree_depth = 100;
LEti::Tree<Space_Splitter_2D::Area, 4> Space_Splitter_2D::m_quad_tree;

std::list<Space_Splitter_2D::Collision_Data> Space_Splitter_2D::m_collisions;

Timer Space_Splitter_2D::m_timer;



bool Space_Splitter_2D::Area::point_is_inside(const Point *_point) const
{
	bool result = true;

	if(!right.inf) result = result && _point->pos.x <= right.value;
	if(!left.inf) result = result && _point->pos.x >= left.value;
	if(!top.inf) result = result && _point->pos.y <= top.value;
	if(!bottom.inf) result = result && _point->pos.y >= bottom.value;

	return result;
}

void Space_Splitter_2D::Area::register_models_inside()
{
	models.clear();

	std::list<const Point*>::const_iterator point_it = points.begin();
	while(point_it != points.end())
	{
		std::list<const Object_2D*>::const_iterator obj_it = models.begin();
		bool model_is_registred = false;
		while(obj_it != models.end())
		{
			if((*point_it)->belongs_to == *obj_it)
			{
				model_is_registred = true;
				break;
			}
			++obj_it;
		}

		if(!model_is_registred)
			models.push_back((*point_it)->belongs_to);;

		++point_it;
	}
}



void Space_Splitter_2D::Area::split(LEti::Tree<Area, 4>::Iterator _it)
{
	glm::vec3 split_point;
//	split_point.x = _it->right.value - ((fabs(_it->right.value) + fabs(_it->left.value)) / 2.0f);
//	split_point.y = _it->top.value - ((fabs(_it->top.value) + fabs(_it->bottom.value)) / 2.0f);
	split_point.x = (_it->right.value + _it->left.value) / 2.0f;
	split_point.y = (_it->top.value + _it->bottom.value) / 2.0f;

	_it.insert_into_availible_index({_it->left, split_point.x, _it->top, split_point.y});
	_it.insert_into_availible_index({split_point.x, _it->right, _it->top, split_point.y});
	_it.insert_into_availible_index({split_point.x, _it->right, split_point.y, _it->bottom});
	_it.insert_into_availible_index({_it->left, split_point.x, split_point.y, _it->bottom});
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

	_it->split(_it);

	for(unsigned int i=0; i<4; ++i)
	{
		LEti::Tree<Area, 4>::Iterator next = _it;
		next.descend(i);

		float left = next->left.inf ? -10000 : next->left.value;
		float right = next->right.inf ? 10000 : next->right.value;
		float top = next->top.inf ? 10000 : next->top.value;
		float bottom = next->bottom.inf ? -10000 : next->bottom.value;
		Debug_Drawable_Frame& dfl = *df;
		dfl.clear_points();
		dfl.set_point(0, {left, top, 0.0f});
		dfl.set_point(1, {right, top, 0.0f});
		dfl.set_point(2, {right, bottom, 0.0f});
		dfl.set_point(3, {left, next->bottom.value, 0.0f});

		dfl.clear_sequence().set_sequence_element(0, 0).set_sequence_element(1, 1).set_sequence_element(2, 2).set_sequence_element(3, 3);

		dfl.update();
		dfl.draw();

		std::list<const Point*>::const_iterator point_it = _it->points.begin();
		while(point_it != _it->points.end())
		{
			if(next->point_is_inside(*point_it))
				next->points.push_back(*point_it);
			++point_it;
		}
		next->register_models_inside();

		split_space_recursive(next, _level+1);
	}
}


void Space_Splitter_2D::update()
{
//	m_timer.update();
//	if(m_timer.is_active()) return;
//	m_timer.start(0.1f);

	m_collisions.clear();

	LEti::Tree<Area, 4>::Iterator it = m_quad_tree.create_iterator();
	if(it.valid()) it.delete_branch();
	it = m_quad_tree.create_iterator();

	it.insert_into_availible_index({{}, {}, {}, {}});

	if(m_registred_models.size() == 0) return;

	m_models_points.clear();
	std::list<const Object_2D*>::iterator model_it = m_registred_models.begin();
	while(model_it != m_registred_models.end())
	{
		Physical_Model_2D::Rectangular_Border rb = ((Physical_Model_2D*)((*model_it)->get_physical_model()))->construct_rectangular_border();
		m_models_points.push_back(Point({rb.left, rb.top, 0.0f}, *model_it));
		m_models_points.push_back(Point({rb.left, rb.bottom, 0.0f}, *model_it));
		m_models_points.push_back(Point({rb.right, rb.top, 0.0f}, *model_it));
		m_models_points.push_back(Point({rb.right, rb.bottom, 0.0f}, *model_it));

		sp_ind->set_pos(rb.left, rb.top, 0.0f);
		sp_ind->draw();
		sp_ind->set_pos(rb.left, rb.bottom, 0.0f);
		sp_ind->draw();
		sp_ind->set_pos(rb.right, rb.top, 0.0f);
		sp_ind->draw();
		sp_ind->set_pos(rb.right, rb.bottom, 0.0f);
		sp_ind->draw();

		if(rb.left < it->left.value || it->left.inf) it->left = rb.left;
		if(rb.right > it->right.value || it->right.inf) it->right = rb.right;
		if(rb.top > it->top.value || it->top.inf) it->top = rb.top;
		if(rb.bottom < it->bottom.value || it->bottom.inf) it->bottom = rb.bottom;

		it->models.push_back(*model_it);

		++model_it;
	}

	std::list<Point>::const_iterator point_it = m_models_points.begin();
	while(point_it != m_models_points.end())
	{
		it->points.push_back(&(*point_it));
		++point_it;
	}

	split_space_recursive(it, 0);

	while(!it.end())
	{
		if(it->models.size() == 2)
		{
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
		}
		++it;
	}
	if(it->models.size() == 2)
	{
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
	}
}



const std::list<Space_Splitter_2D::Collision_Data>& Space_Splitter_2D::get_collisions()
{
	return m_collisions;
}
