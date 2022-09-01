#pragma once

#include <glm/glm.hpp>
#include <climits>
#include <algorithm>
#include "gldebug.h"

namespace grinliz 
{
	struct Rect3F {
		Rect3F(const glm::vec3& v0, const glm::vec3& v1, const glm::vec3& v2) {
			size.x = size.y = size.z = 0;
			pos = v0;
			DoUnion(v1);
			DoUnion(v2);
		}

		Rect3F() {
			pos.x = pos.y = pos.z = FLT_MIN;
			size.x = size.y = size.z = 0;
		}

		Rect3F(const glm::vec3& _origin, const glm::vec3& _size) {
			pos = _origin;
			size = _size;

			GLASSERT(size.x >= 0);
			GLASSERT(size.y >= 0);
			GLASSERT(size.z >= 0);
		}

		Rect3F(float posX, float posY, float posZ, float sX, float sY, float sZ) {
			pos.x = posX;
			pos.y = posY;
			pos.z = posZ;

			size.x = sX;
			size.y = sY;
			size.z = sZ;

			GLASSERT(size.x >= 0);
			GLASSERT(size.y >= 0);
			GLASSERT(size.z >= 0);
		}

		bool IsValid() const { return pos.x != FLT_MIN; }

		void Outset(float x) {
			pos.x -= x;
			pos.y -= x;
			pos.z -= x;
			size.x += 2.0f * x;
			size.y += 2.0f * x;
			size.z += 2.0f * x;
		}

		void DoUnion(const glm::vec3& p) {
			GLASSERT(size.x >= 0);
			GLASSERT(size.y >= 0);
			GLASSERT(size.z >= 0);

			if (pos.x == FLT_MIN) {
				pos = p;
			}
			else {
				glm::vec3 upper = Upper();
				pos = glm::min(pos, p);
				size = glm::max(upper, p) - pos;
			}
		}

		void DoUnion(const Rect3F& r) {
			DoUnion(r.pos);
			DoUnion(r.pos + r.size);
		}

		Rect3F Intersection(const Rect3F& r) const {
			GLASSERT(size.x >= 0);
			GLASSERT(size.y >= 0);
			GLASSERT(size.z >= 0);

			glm::vec3 low = glm::min(pos + size, r.pos + r.size);
			glm::vec3 high = glm::max(pos, r.pos);
			glm::vec3 sz = high - low;

			Rect3F out;
			if (sz.x >= 0 && sz.y >= 0 && sz.z >= 0) {
				out.pos = low;
				out.size = sz;
			}
			return out;
		}

		void DoIntersection(const Rect3F& r) {
			*this = Intersection(r);
		}

		bool Intersects(const Rect3F& r) {
			for (int i = 0; i < 3; ++i) {
				if (r.pos[i] + r.size[i] < pos[i])
					return false;
				if (r.pos[i] >= pos[i] + size[i])
					return false;
			}
			return true;
		}

		glm::vec3 Select(int x, int y, int z) const {
			glm::vec3 p;
			p.x = x ? pos.x + size.x : pos.x;
			p.y = y ? pos.y + size.y : pos.y;
			p.z = z ? pos.z + size.z : pos.z;
			return p;
		}

		const glm::vec3& Lower() const { return pos; }
		glm::vec3 Upper() const { return pos + size; }
		glm::vec3 Center() const { return pos + size * 0.5f; }

		int MajorAxis() const {
			if (size.x >= size.y && size.x >= size.z)
				return 0;
			if (size.y >= size.z)
				return 1;
			return 2;
		}

		void Split(int axis, Rect3F* a, Rect3F* b) const {
			a->pos = b->pos = pos;
			a->size = b->size = size;
				
			a->size[axis] = b->size[axis] = size[axis] * 0.5f;
			b->pos[axis] = pos[axis] + a->size[axis];
		}

		bool Contains(const Rect3F& a) const {
			for (int i = 0; i < 3; ++i) {
				if (a.pos[i] < pos[i])
					return false;
				if (a.Upper()[i] > Upper()[i])
					return false;
			}
			return true;
		}

		bool Contains(const glm::vec3& a) const {
			for (int i = 0; i < 3; ++i)
				if ((a[i] < pos[i]) || (a[i] >= pos[i] + size[i]))
					return false;
			return true;
		}

		glm::vec3 pos;
		glm::vec3 size;

		static void Test();
	};

};
