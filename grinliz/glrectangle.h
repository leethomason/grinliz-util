#pragma once

#include <glm/glm.hpp>
#include <climits>
#include <algorithm>
#include <limits>
#include "gldebug.h"

namespace grinliz 
{
	// General rectangle type.
	// The lower ("left") edge is inclusive, the upper ("right") edge is exclusive.
	// The rectangle is default constructed to an Invalid state. You can query whether it has become valid
	//    (through values being set or with DoUnion()) by calling IsValid().
	// Note that when doing unions with points, this behavior is correct but odd; see IntersectsIncl() to help.
	// typenames for Rect2I, Rect2F, Rect3I, and Rect3F are provided. But should work for any
	//    glm-compatible VEC with value type NUM.
	template<typename VEC, typename NUM>
	struct Rect {

		using Vec_t = typename VEC;
		using Num_t = typename NUM;
		static constexpr NUM kInvalid = std::numeric_limits<NUM>::min();

		Rect() {
			pos = VEC{ kInvalid };
			size = VEC{ 0 };
		}

		Rect(const VEC& _origin, const VEC& _size) {
			pos = _origin;
			size = _size;

			for (int i = 0; i < VEC::length(); ++i) {
				GLASSERT(size[i] >= 0);
			}
		}

		bool operator==(const Rect<VEC, NUM>& rhs) const {
			return pos == rhs.pos && size == rhs.size;
		}

		bool operator!=(const Rect<VEC, NUM>& rhs) const {
			return !(*this == rhs);
		}

		bool IsValid() const { return pos.x != kInvalid; }

		void Outset(float x) {
			pos -= VEC{ x };
			size += NUM(2) * VEC{ x };
		}

		void DoUnion(const VEC& p) {
			for (int i = 0; i < VEC::length(); ++i) {
				GLASSERT(size[i] >= 0);
			}

			if (pos.x == kInvalid) {
				pos = p;
			}
			else {
				VEC upper = Upper();
				pos = glm::min(pos, p);
				size = glm::max(upper, p) - pos;
			}
		}

		void DoUnion(const Rect<VEC, NUM>& r) {
			DoUnion(r.pos);
			DoUnion(r.pos + r.size);
		}

		Rect<VEC, NUM> Intersection(const Rect<VEC, NUM>& r) const {
			for (int i = 0; i < VEC::length(); ++i) {
				GLASSERT(size[i] >= 0);
			}

			VEC low = glm::max(pos, r.pos);
			VEC high = glm::min(Upper(), r.Upper());
			VEC sz = high - low;

			for (int i = 0; i < VEC::length(); ++i) {
				if (sz[i] < 0)
					return Rect<VEC, NUM>();
			}
			return { low, sz };
		}

		void DoIntersection(const Rect<VEC, NUM>& r) {
			*this = Intersection(r);
		}

		VEC Select(int x, int y, int z) const {
			VEC p;
			const int arr[3] = { x, y, z };
			for (int i = 0; i < VEC::length(); ++i) {
				p[i] = arr[i] ? pos[i] + size[i] : pos[i];
			}
			return p;
		}

		const VEC& Lower() const { return pos; }
		VEC Upper() const { return pos + size; }
		VEC Center() const { return pos + size / NUM(2); }

		int MajorAxis() const {
			int axis = 0;
			NUM s = size.x;
			for (int i = 1; i < VEC::length(); ++i) {
				if (size[i] > s) {
					s = size[i];
					axis = i;
				}
			}
			return axis;
		}

		void Split(int axis, Rect<VEC, NUM>* a, Rect<VEC, NUM>* b) const {
			a->pos = b->pos = pos;
			a->size = b->size = size;
				
			a->size[axis] = b->size[axis] = size[axis] / NUM(2);
			b->pos[axis] = pos[axis] + a->size[axis];
		}

		bool Intersects(const Rect<VEC, NUM>& r) const {
			for (int i = 0; i < VEC::length(); ++i) {
				if (r.pos[i] + r.size[i] < pos[i])
					return false;
				if (r.pos[i] >= pos[i] + size[i])
					return false;
			}
			return true;
		}

		bool IntersectsIncl(const Rect<VEC, NUM>& r) const {
			for (int i = 0; i < VEC::length(); ++i) {
				if (r.pos[i] + r.size[i] < pos[i])
					return false;
				if (r.pos[i] > pos[i] + size[i])
					return false;
			}
			return true;
		}

		bool Contains(const Rect<VEC, NUM>& a) const {
			if (glm::all(glm::greaterThanEqual(a.Lower(), Lower())) && glm::all(glm::lessThanEqual(a.Upper(), Upper())))
				return true;
			return false;
		}

		bool Contains(const VEC& a) const {
			if (glm::any(glm::lessThan(a, pos)))
				return false;
			if (glm::any(glm::greaterThanEqual(a, pos + size)))
				return false;
			return true;
		}

		// Length, Area, or Volume depending on the dimension
		NUM Volume() const {
			NUM v = size[0];
			for (int i = 1; i < VEC::length(); ++i) {
				v *= size[i];
			}
			return v;
		}

		VEC pos;
		VEC size;
	};

	template<>
	bool Rect<glm::vec3, float>::Intersects(const Rect<glm::vec3, float>& r) const;

	template<>
	bool Rect<glm::vec2, float>::Intersects(const Rect<glm::vec2, float>& r) const;

	template<>
	bool Rect<glm::vec3, float>::Contains(const glm::vec3& a) const;

	template<>
	bool Rect<glm::vec2, float>::Contains(const glm::vec2& a) const;

	using Rect3F = Rect<glm::vec3, float>;
	using Rect2F = Rect<glm::vec2, float>;
	using Rect3I = Rect<glm::ivec3, int>;
	using Rect2I = Rect<glm::ivec2, int>;

	void TestRect();
};
