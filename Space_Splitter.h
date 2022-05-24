#ifndef __SPACE_SPLITTER
#define __SPACE_SPLITTER

#include <list>

#include "Tree.h"
#include "Physical_Model_2D.h"


namespace LEti {

	class Space_Splitter_2D final
	{
	private:
		Space_Splitter_2D() = delete;

	private:
		struct Border
		{
			bool inf = true;
			float value = 0.0f;

			Border() { }
			Border(float _value) : inf(false), value(_value) { }
			void operator=(float _value) { inf = false; value = _value; }
			Border(const Border& _other) : inf(_other.inf), value(_other.value) { }
			void operator=(const Border& _other) { inf = _other.inf; value = _other.value; }

			bool operator<(float _compared_to) const { return value < _compared_to; }
			bool operator>(float _compared_to) const { return value > _compared_to; }
		};

		struct Area
		{
			Border left, right, top, bottom;
			std::list<const Physical_Model_2D*> models;

			Area(const Border& _left, const Border& _right, const Border& _top, const Border& _bottom)
				: left(_left), right(_right), top(_top), bottom(_bottom) { }

			bool point_is_inside(const glm::vec3& _point) const;
			bool model_is_inside(const Physical_Model_2D* _model) const;

			glm::vec3 get_point_to_split() const;
		};

	private:
		static std::list<const Physical_Model_2D*> m_registred_models;
		static LEti::Tree<Area, 4> m_quad_tree;

	public:
		static void register_physical_model(const Physical_Model_2D* _model);

	private:
		static void split_space_recursive(LEti::Tree<Area, 4>::Iterator _it);
	public:
		static std::list<std::pair<const Physical_Model_2D*, const Physical_Model_2D*>> get_possible_collisions();

	};

}	/*LEti*/


#endif // __SPACE_SPLITTER
