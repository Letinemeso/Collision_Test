#include "Space_Splitter_2D.h"

using namespace LEti;


#include "Debug_Drawable_Frame.h"

//Debug_Drawable_Frame* df = nullptr;
//Object_2D* sp_ind = nullptr;

//void LEti::init_frame(const char* _obj_name)
//{
//	df = new Debug_Drawable_Frame;
//	df->init("debug_frame");

//	sp_ind = new Object_2D;
//	sp_ind->init("flat_indicator_red");
//}

std::list<const Object_2D*> Space_Splitter_2D::m_registred_models;

unsigned int Space_Splitter_2D::m_max_tree_depth = 4;
LEti::Tree<Space_Splitter_2D::Area, 4> Space_Splitter_2D::m_quad_tree;

std::list<Space_Splitter_2D::Collision_Data> Space_Splitter_2D::m_collisions;

Timer Space_Splitter_2D::m_timer;



bool Space_Splitter_2D::Area::rectangle_is_inside(const Physical_Model_2D::Rectangular_Border& _rectangle) const
{
	bool result = true;

	if(!right.inf) result = result && _rectangle.left <= right.value;
	if(!left.inf) result = result && _rectangle.right >= left.value;
	if(!top.inf) result = result && _rectangle.bottom <= top.value;
	if(!bottom.inf) result = result && _rectangle.top >= bottom.value;

	return result;
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
	const auto& rectangles = _it->rectangles;
	if (rectangles.size() < 3 || _level > m_max_tree_depth) return;

	_it->split(_it);

	for(unsigned int i=0; i<4; ++i)
	{
		LEti::Tree<Area, 4>::Iterator next = _it;
		next.descend(i);

//		float left = next->left.inf ? -10000 : next->left.value;
//		float right = next->right.inf ? 10000 : next->right.value;
//		float top = next->top.inf ? 10000 : next->top.value;
//		float bottom = next->bottom.inf ? -10000 : next->bottom.value;
//		Debug_Drawable_Frame& dfl = *df;
//		dfl.clear_points();
//		dfl.set_point(0, {left, top, 0.0f});
//		dfl.set_point(1, {right, top, 0.0f});
//		dfl.set_point(2, {right, bottom, 0.0f});
//		dfl.set_point(3, {left, next->bottom.value, 0.0f});

//		dfl.clear_sequence().set_sequence_element(0, 0).set_sequence_element(1, 1).set_sequence_element(2, 2).set_sequence_element(3, 3);

//		dfl.update();
//		dfl.draw();

		std::list<Rectangle_Wrapper>::const_iterator point_it = rectangles.begin();
		while(point_it != rectangles.end())
		{
			if(next->rectangle_is_inside(point_it->rectangle))
				next->rectangles.push_back(*point_it);
			++point_it;
		}

		split_space_recursive(next, _level+1);
	}
}


void Space_Splitter_2D::update()
{
	m_timer.update();
	if(m_timer.is_active()) return;
	m_timer.start(0.05f);

	m_collisions.clear();

	LEti::Tree<Area, 4>::Iterator it = m_quad_tree.create_iterator();
	if(it.valid()) it.delete_branch();
	it = m_quad_tree.create_iterator();

	it.insert_into_availible_index({{}, {}, {}, {}});

	if(m_registred_models.size() == 0) return;

	std::list<const Object_2D*>::iterator model_it = m_registred_models.begin();
	while(model_it != m_registred_models.end())
	{
		Physical_Model_2D::Rectangular_Border rb = ((Physical_Model_2D*)((*model_it)->get_physical_model()))->construct_rectangular_border();

//		sp_ind->set_pos(rb.left, rb.top, 0.0f);
//		sp_ind->draw();
//		sp_ind->set_pos(rb.left, rb.bottom, 0.0f);
//		sp_ind->draw();
//		sp_ind->set_pos(rb.right, rb.top, 0.0f);
//		sp_ind->draw();
//		sp_ind->set_pos(rb.right, rb.bottom, 0.0f);
//		sp_ind->draw();

		if(rb.left < it->left.value || it->left.inf) it->left = rb.left;
		if(rb.right > it->right.value || it->right.inf) it->right = rb.right;
		if(rb.top > it->top.value || it->top.inf) it->top = rb.top;
		if(rb.bottom < it->bottom.value || it->bottom.inf) it->bottom = rb.bottom;

		/*const Physical_Model_2D* pm = (Physical_Model_2D*)((*model_it)->get_physical_model());

		for(unsigned int pol=0; pol<pm->get_polygons_count(); ++pol)
		{
			for(unsigned int vert=0; vert < 3; ++vert)
			{
				const glm::vec3& point = (*pm)[pol][vert];
				sp_ind->set_pos(point.x, point.y, 0.0f);
				sp_ind->draw();

				if(point.x < it->left.value || it->left.inf) it->left = point.x;
				if(point.x > it->right.value || it->right.inf) it->right = point.x;
				if(point.y > it->top.value || it->top.inf) it->top = point.y;
				if(point.y < it->bottom.value || it->bottom.inf) it->bottom = point.y;

				m_models_points.push_back(Point(point, *model_it));
			}
		}*/

		it->rectangles.push_back({*model_it, rb});

		++model_it;
	}

	split_space_recursive(it, 0);

	while(!it.end())
	{
		if(it->rectangles.size() == 2)
		{
			std::list<Rectangle_Wrapper>::iterator colliding_model = it->rectangles.begin();
			std::list<Rectangle_Wrapper>::iterator next = colliding_model;
			++next;
			Collision_Data cd(colliding_model->belongs_to, next->belongs_to, colliding_model->belongs_to->is_colliding_with_other(*next->belongs_to));
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
		else if(it.is_leaf() && it->rectangles.size() > 2)
		{
			std::list<Rectangle_Wrapper>::const_iterator colliding_model = it->rectangles.begin();
			while(colliding_model != it->rectangles.end())
			{
				std::list<Rectangle_Wrapper>::const_iterator next = colliding_model;
				++next;
				while(next != it->rectangles.end())
				{
					Collision_Data cd(colliding_model->belongs_to, next->belongs_to, colliding_model->belongs_to->is_colliding_with_other(*next->belongs_to));
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
					++next;
				}
				++colliding_model;
			}

//			std::list<Rectangle_Wrapper>::const_iterator next = colliding_model;
//			++next;
//			Collision_Data cd(colliding_model->belongs_to, next->belongs_to, colliding_model->belongs_to->is_colliding_with_other(*next->belongs_to));
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
	if(it->rectangles.size() == 2)
	{
		std::list<Rectangle_Wrapper>::iterator colliding_model = it->rectangles.begin();
		std::list<Rectangle_Wrapper>::iterator next = colliding_model;
		++next;
		Collision_Data cd(colliding_model->belongs_to, next->belongs_to, colliding_model->belongs_to->is_colliding_with_other(*next->belongs_to));
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
	else if(it->rectangles.size() > 2)
	{
		std::list<Rectangle_Wrapper>::const_iterator colliding_model = it->rectangles.begin();
		while(colliding_model != it->rectangles.end())
		{
			std::list<Rectangle_Wrapper>::const_iterator next = colliding_model;
			++next;
			while(next != it->rectangles.end())
			{
				Collision_Data cd(colliding_model->belongs_to, next->belongs_to, colliding_model->belongs_to->is_colliding_with_other(*next->belongs_to));
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
				++next;
			}
			++colliding_model;
		}
	}
}



const std::list<Space_Splitter_2D::Collision_Data>& Space_Splitter_2D::get_collisions()
{
	return m_collisions;
}
