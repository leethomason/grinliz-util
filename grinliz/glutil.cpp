#include "glutil.h"
#include "gldebug.h"

using namespace grinliz;

bool grinliz::TestUtil()
{
	{
		uint32_t a = 0xabcd'0123;
		uint64_t b = 0xabcd'0123'4567'1357;
		GLASSERT(Rotate(a, 0) == a);
		GLASSERT(Rotate(a, 4) == 0xbcd0123a);
		GLASSERT(Rotate(b, 0) == b);
		GLASSERT(Rotate(b, 4) == 0xbcd012345671357a);
	}
	return true;
}
