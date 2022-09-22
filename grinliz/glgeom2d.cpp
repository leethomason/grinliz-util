#include "glgeom2d.h"
#include "glmath.h"

using namespace grinliz;

float grinliz::ClosestPoint(glm::vec2 p, const Rect2F& r, glm::vec2* pointOnRect)
{
	if (r.Contains(p)) {
		glm::vec2 center = r.Center();
		glm::vec2 dc = p - center;
		glm::vec2 delta(0);

		if (dc.x > 0)
			delta.x = r.Upper().x - p.x;
		else
			delta.x = p.x - r.pos.x;

		if (dc.y > 0)
			delta.y = r.Upper().y - p.y;
		else
			delta.y = p.y - r.pos.y;

		if (delta.x < delta.y) {
			*pointOnRect = p + glm::vec2(Sign(dc.x) * delta.x, 0);
			return -delta.x;
		}
		else {
			*pointOnRect = p + glm::vec2(0, Sign(dc.y) * delta.y);
			return -delta.y;
		}
	}
	// Outside.
	if (p.x >= r.pos.x && p.x <= r.Upper().x) {
		pointOnRect->x = p.x;
		if (p.y <= r.pos.y) {
			// North
			pointOnRect->y = r.pos.y;
			return r.pos.y - p.y;
		}
		else {
			// South
			pointOnRect->y = r.Upper().y;
			return p.y - r.Upper().y;
		}
	}
	else if (p.y >= r.pos.y && p.y <= r.Upper().y) {
		pointOnRect->y = p.y;
		if (p.x <= r.pos.x) {
			// West
			pointOnRect->x = r.pos.x;
			return r.pos.x - p.x;
		}
		else {
			// East
			pointOnRect->x = r.Upper().x;
			return p.x - r.Upper().x;
		}
	}

	if (p.x >= r.Upper().x && p.y <= r.pos.y) {
		// NE
		*pointOnRect = r.Select(1, 0, 0);
	}
	else if (p.x >= r.Upper().x && p.y >= r.Upper().y) {
		// SE
		*pointOnRect = r.Select(1, 1, 0);
	}
	else if (p.x <= r.pos.x && p.y <= r.pos.y) {
		// NW
		*pointOnRect = r.Select(0, 0, 0);
	}
	else {
		*pointOnRect = r.Select(0, 1, 0);
	}
	return glm::distance(p, *pointOnRect);
}

void OneTest2D(glm::vec2 p, const grinliz::Rect2F& r, glm::vec2 outExpected, float dExpected)
{
	glm::vec2 out;
	float d = ClosestPoint(p, r, &out);
	GLASSERT(Equal(d, dExpected, 0.0001f));
	GLASSERT(Equal(out, outExpected, 0.0001f));
}

bool grinliz::Test2D()
{
	Rect2F r({ 0, 0 }, { 2, 1 });

	OneTest2D(glm::vec2(0.1f, 0.5f), r, glm::vec2(0.0f, 0.5f), -0.1f);
	OneTest2D(glm::vec2(1.9f, 0.5f), r, glm::vec2(2.0f, 0.5f), -0.1f);
	OneTest2D(glm::vec2(0.9f, 0.1f), r, glm::vec2(0.9f, 0.0f), -0.1f);
	OneTest2D(glm::vec2(0.9f, 0.9f), r, glm::vec2(0.9f, 1.0f), -0.1f);

	OneTest2D(glm::vec2(1.0f, -1.0f), r, glm::vec2(1.0f, 0.0f), 1.0f);	// n
	OneTest2D(glm::vec2(1.0f, 2.0f), r, glm::vec2(1.0f, 1.0f), 1.0f);	// s
	OneTest2D(glm::vec2(-1.0f, 0.5f), r, glm::vec2(0.0f, 0.5f), 1.0f);	// w
	OneTest2D(glm::vec2(3.0f, 0.5f), r, glm::vec2(2.0f, 0.5f), 1.0f);	// e

	OneTest2D(glm::vec2(-1.0f, -1.0f), r, glm::vec2(0.0f, 0.0f), SQRT2);
	OneTest2D(glm::vec2(3.0f, -1.0f), r, glm::vec2(2.0f, 0.0f), SQRT2);	
	OneTest2D(glm::vec2(3.0f, 2.0f), r, glm::vec2(2.0f, 1.0f), SQRT2);	
	OneTest2D(glm::vec2(-1.0f, 2.0f), r, glm::vec2(0.0f, 1.0f), SQRT2);	


	return true;
}
