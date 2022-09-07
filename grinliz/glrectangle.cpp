#include "glrectangle.h"

using namespace grinliz;

void grinliz::TestRect()
{
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
