#pragma once

#include <glm/glm.hpp>
#include <climits>
#include <algorithm>
#include "gldebug.h"

namespace grinliz 
{
	template<typename VEC>
	struct RectF {
		/*RectF(const VEC& v0, const VEC& v1, const VEC& v2) {
			size = VEC{ 0 };
			pos = v0;
			DoUnion(v1);
			DoUnion(v2);
		}
		*/

		RectF() {
			pos = VEC{ FLT_MIN };
			pos = VEC{ 0 };
		}

		RectF(const VEC& _origin, const VEC& _size) {
			pos = _origin;
			size = _size;

			for (int i = 0; i < VEC::length(); ++i) {
				GLASSERT(size[i] >= 0);
			}
		}

		bool IsValid() const { return pos.x != FLT_MIN; }

		void Outset(float x) {
			pos.x -= VEC{ x };
			size.x += 2.0f * VEC{ x };
		}

		void DoUnion(const VEC& p) {
			for (int i = 0; i < VEC::length(); ++i) {
				GLASSERT(size[i] >= 0);
			}

			if (pos.x == FLT_MIN) {
				pos = p;
			}
			else {
				VEC upper = Upper();
				pos = glm::min(pos, p);
				size = glm::max(upper, p) - pos;
			}
		}

		void DoUnion(const RectF<VEC>& r) {
			DoUnion(r.pos);
			DoUnion(r.pos + r.size);
		}

		RectF<VEC> Intersection(const RectF<VEC>& r) const {
			for (int i = 0; i < VEC::length; ++i) {
				GLASSERT(size[i] >= 0);
			}

			VEC low = glm::min(pos + size, r.pos + r.size);
			VEC high = glm::max(pos, r.pos);
			VEC sz = high - low;

			VEC out{ 0 };
			for (int i = 0; i < VEC::length(); ++i) {
				if (sz[i] < 0)
					return out;
			}
			out.pos = low;
			out.size = sz;
			return out;
		}

		void DoIntersection(const RectF<VEC>& r) {
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
		VEC Center() const { return pos + size * 0.5f; }

		int MajorAxis() const {
			int axis = 0;
			float s = size.x;
			for (int i = 1; i < VEC::length(); ++i) {
				if (size[i] > s) {
					s = size[i];
					axis = i;
				}
			}
			return axis;
		}

		void Split(int axis, RectF<VEC>* a, RectF<VEC>* b) const {
			a->pos = b->pos = pos;
			a->size = b->size = size;
				
			a->size[axis] = b->size[axis] = size[axis] * 0.5f;
			b->pos[axis] = pos[axis] + a->size[axis];
		}

		bool Intersects(const RectF<VEC>& r) const {
			GLASSERT(false);	// should be specialized.
			if (glm::any(glm::lessThanEqual(r.Upper(), Lower())))
				return false;
			if (glm::any(glm::greaterThanEqual(r.Lower(), Upper())))
				return false;
			return true;
		}

		bool Contains(const RectF<VEC>& a) const {
			if (glm::all(glm::greaterThanEqual(a.Lower(), Lower())) && glm::all(glm::lessThanEqual(a.Upper(), Upper())))
				return true;
			return false;
		}

		bool Contains(const VEC& a) const {
			GLASSERT(false);	// should be specialized.
			if (glm::any(glm::lessThan(a, pos)))
				return false;
			if (glm::any(glm::greaterThanEqual(a, pos + size)))
				return false;
			return true;
		}

		VEC pos;
		VEC size;
	};

	template<>
	inline bool RectF<glm::vec3>::Intersects(const RectF<glm::vec3>& r) const {
		if (r.pos.x + r.size.x <= pos.x)
			return false;
		if (r.pos.y + r.size.y <= pos.y)
			return false;
		if (r.pos.z + r.size.z <= pos.z)
			return false;
		if (r.pos.x >= pos.x + size.x)
			return false;
		if (r.pos.y >= pos.y + size.y)
			return false;
		if (r.pos.z >= pos.z + size.z)
			return false;
		return true;
	}

	template<>
	inline bool RectF<glm::vec2>::Intersects(const RectF<glm::vec2>& r) const {
		if (r.pos.x + r.size.x <= pos.x)
			return false;
		if (r.pos.y + r.size.y <= pos.y)
			return false;
		if (r.pos.x >= pos.x + size.x)
			return false;
		if (r.pos.y >= pos.y + size.y)
			return false;
		return true;
	}

	template<>
	inline bool RectF<glm::vec3>::Contains(const glm::vec3& a) const {
		if ((a.x < pos.x) || a.x >= (pos.x + size.x))
			return false;
		if ((a.y < pos.y) || a.y >= (pos.y + size.y))
			return false;
		if ((a.z < pos.z) || a.z >= (pos.z + size.z))
			return false;
		return true;
	}

	template<>
	inline bool RectF<glm::vec2>::Contains(const glm::vec2& a) const {
		if ((a.x < pos.x) || a.x >= (pos.x + size.x))
			return false;
		if ((a.y < pos.y) || a.y >= (pos.y + size.y))
			return false;
		return true;
	}

	using Rect3F = RectF<glm::vec3>;
	using Rect2F = RectF<glm::vec2>;

	void TestRect();
};
