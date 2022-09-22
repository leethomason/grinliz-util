#pragma once

#include <glm/glm.hpp>
#include "glrectangle.h"

namespace grinliz {

	float ClosestPoint(glm::vec2 p, const Rect2F& r, glm::vec2* pointOnRect);

	bool Test2D();

}