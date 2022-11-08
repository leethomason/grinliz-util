#pragma once

#include <glm/glm.hpp>
#include "glrectangle.h"

namespace grinliz {

	float ClosestPoint(glm::vec2 p, const Rect2F& r, glm::vec2* pointOnRect);

	inline glm::vec2 rotate(float rad, glm::vec2 p) {
		glm::vec2 a;
		const float c = cosf(rad);
		const float s = sinf(rad);
		a.x = c * p.x - s * p.y;
		a.y = s * p.x + c * p.y;
		return a;
	}

	inline glm::vec2 rotateQ4(float rad, glm::vec2 p) { return rotate(-rad, p); }

	bool Test2D();

}