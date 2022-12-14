#include "glrectangle.h"

using namespace grinliz;

template<>
bool Rect<glm::vec3, float>::Intersects(const Rect<glm::vec3, float>& r) const {
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
bool Rect<glm::vec2, float>::Intersects(const Rect<glm::vec2, float>& r) const {
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
bool Rect<glm::vec3, float>::Contains(const glm::vec3& a) const {
	if ((a.x < pos.x) || a.x >= (pos.x + size.x))
		return false;
	if ((a.y < pos.y) || a.y >= (pos.y + size.y))
		return false;
	if ((a.z < pos.z) || a.z >= (pos.z + size.z))
		return false;
	return true;
}

template<>
bool Rect<glm::vec2, float>::Contains(const glm::vec2& a) const {
	if ((a.x < pos.x) || a.x >= (pos.x + size.x))
		return false;
	if ((a.y < pos.y) || a.y >= (pos.y + size.y))
		return false;
	return true;
}

void grinliz::TestRect()
{
	{
		// Bounds Madness!!!
		{
			Rect2F r;
			r.DoUnion({ 0, 0 });
			GLASSERT(r.size == glm::vec2(0));
			r.DoUnion({ 1, 1 });
			GLASSERT(r.size == glm::vec2(1));
		}	
		{
			Rect2I r;
			r.DoUnion({ 0, 0 });
			GLASSERT(r.size == glm::ivec2(0));
			r.DoUnion({ 1, 1 });
			GLASSERT(r.size == glm::ivec2(1));
		}
	}

	{
		Rect3F rn1(glm::vec3(-1, 0, 0), glm::vec3(1, 1, 1));
		Rect3F r0(glm::vec3(0, 0, 0), glm::vec3(1, 1, 1));
		Rect3F r1(glm::vec3(1, 0, 0), glm::vec3(1, 1, 1));

		{
			Rect3F r = rn1;
			r.DoUnion(r0);
			GLASSERT(r.pos.x == -1);
			GLASSERT(r.size.x == 2);
		}
		{
			Rect3F r = r0;
			r.DoUnion(rn1);
			GLASSERT(r.pos.x == -1);
			GLASSERT(r.size.x == 2);
		}
	}
	{
		Rect2F rn1({ -1, 0 }, { 1, 1 });
		Rect2F r0({ 0, 0 }, { 1, 1 });
		Rect2F r1({ 1, 0 }, { 1, 1 });

		{
			Rect2F r = rn1;
			r.DoUnion(r0);
			GLASSERT(r.pos.x == -1);
			GLASSERT(r.size.x == 2);
		}
		{
			Rect2F r = r0;
			r.DoUnion(rn1);
			GLASSERT(r.pos.x == -1);
			GLASSERT(r.size.x == 2);
		}
	}
	{
		Rect3F r0({ 0, 0, 0 }, { 1, 1, 1 });
		Rect3F r1({ 1, 0, 0 }, { 1, 1, 1 });
		Rect3F rh({ 0.5, 0, 0 }, { 1, 1, 1 });

		glm::vec3 p0(0, 0, 0);
		glm::vec3 p1(1, 1, 1);

		GLASSERT(!r0.Intersects(r1));
		GLASSERT(!r1.Intersects(r0));
		GLASSERT(rh.Intersects(r0));
		GLASSERT(r0.Intersects(rh));
		GLASSERT(rh.Intersects(r1));
		GLASSERT(r1.Intersects(rh));

		GLASSERT(r0.Contains(r0));
		GLASSERT(!r0.Contains(rh));

		GLASSERT(r0.Contains(p0));
		GLASSERT(!r0.Contains(p1));
	}
}
