#ifndef __SPACE_SPLITTER
#define __SPACE_SPLITTER

#include <list>

#include "Tree.h"
#include "Object.h"


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
			std::list<const Object_2D*> models;

			Area(const Border& _left, const Border& _right, const Border& _top, const Border& _bottom)
				: left(_left), right(_right), top(_top), bottom(_bottom) { }

			bool point_is_inside(const glm::vec3& _point) const;
			bool model_is_inside(const Object_2D* _object) const;

			glm::vec3 get_point_to_split() const;
		};

	public:
		struct Collision_Data
		{
			const Object_2D* first = nullptr, * second = nullptr;
			LEti::Physical_Model_2D::Intersection_Data collision_data;
			Collision_Data(const Object_2D* _first, const Object_2D* _second, LEti::Physical_Model_2D::Intersection_Data _id)
				: first(_first), second(_second), collision_data(_id) { }
			bool operator==(const Collision_Data& _other) const { return first == _other.first && second == _other.second; }
		};

	private:
		static std::list<const Object_2D*> m_registred_models;
		static LEti::Tree<Area, 4> m_quad_tree;

		static std::list<Collision_Data> m_collisions;

	public:
//		static void set_update_rate(float _rate); // times per second, aka 1.0f / 60.0f

		static void register_object(const Object_2D* _model);
		static void unregister_object(const Object_2D* _model);

	private:
		static void split_space_recursive(LEti::Tree<Area, 4>::Iterator _it);
	public:
		static void update();

	public:
		static const std::list<Collision_Data>& get_collisions();

	};

	class Space_Splitter_3D final
	{
	private:
		Space_Splitter_3D() = delete;

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
			Border left, right, top, bottom, far, close;
			std::list<const Object_3D*> models;

			Area(const Border& _left, const Border& _right, const Border& _top, const Border& _bottom,
				 const Border& _far, const Border& _close)
				: left(_left), right(_right), top(_top), bottom(_bottom), far(_far), close(_close) { }

			bool point_is_inside(const glm::vec3& _point) const;
			bool model_is_inside(const Object_3D* _model) const;

			glm::vec3 get_point_to_split() const;
		};

	public:
		struct Collision_Data
		{
			const Object_3D* first = nullptr, * second = nullptr;
			LEti::Physical_Model_3D::Intersection_Data collision_data;
			Collision_Data(const Object_3D* _first, const Object_3D* _second, LEti::Physical_Model_2D::Intersection_Data _id)
				: first(_first), second(_second), collision_data(_id) { }
			bool operator==(const Collision_Data& _other) const { return first == _other.first && second == _other.second; }
		};

	private:
		static std::list<const Object_3D*> m_registred_models;
		static LEti::Tree<Area, 8> m_quad_tree;

		static std::list<Collision_Data> m_collisions;

	public:
//		static void set_update_rate(float _rate); // times per second, aka 1.0f / 60.0f

		static void register_object(const Object_3D* _model);
		static void unregister_object(const Object_3D* _model);

	private:
		static void split_space_recursive(LEti::Tree<Area, 8>::Iterator _it);
	public:
		static void update();

	public:
		static std::list<Collision_Data> get_collisions();

	};

}	/*LEti*/


#endif // __SPACE_SPLITTER
