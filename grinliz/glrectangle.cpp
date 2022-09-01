#include "glrectangle.h"

using namespace grinliz;

void Rect3F::Test()
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
