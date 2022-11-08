/*
Copyright (c) 2000-2007 Lee Thomason (www.grinninglizard.com)
Grinning Lizard Utilities.

This software is provided 'as-is', without any express or implied 
warranty. In no event will the authors be held liable for any 
damages arising from the use of this software.

Permission is granted to anyone to use this software for any 
purpose, including commercial applications, and to alter it and 
redistribute it freely, subject to the following restrictions:

1. The origin of this software must not be misrepresented; you must 
not claim that you wrote the original software. If you use this 
software in a product, an acknowledgment in the product documentation 
would be appreciated but is not required.

2. Altered source versions must be plainly marked as such, and 
must not be misrepresented as being the original software.

3. This notice may not be removed or altered from any source 
distribution.
*/



#ifndef GRINLIZ_PERFORMANCE_MEASURE
#define GRINLIZ_PERFORMANCE_MEASURE

#include "gldebug.h"
#include <chrono>

namespace grinliz {

	using timePoint_t = std::chrono::high_resolution_clock::time_point;

	inline timePoint_t Now() { return std::chrono::high_resolution_clock::now(); }
	inline double DeltaSeconds(timePoint_t start, timePoint_t end) {
		return std::chrono::duration_cast<std::chrono::microseconds>(end - start).count() / 1'000'000.0;
	}
	inline int64_t DeltaMillis(timePoint_t start, timePoint_t end) {
		return std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
	}

	class QuickProfile
	{
	public:
		QuickProfile(const char* name) {
			startTime = std::chrono::high_resolution_clock::now();
			this->name = name;
		}

		~QuickProfile() {
			auto endTime = std::chrono::high_resolution_clock::now();
			std::chrono::microseconds micro = std::chrono::duration_cast<std::chrono::microseconds>(
				endTime - startTime
				);
			GLOUTPUT_REL(("%s: %d micro %d millis\n", name, (int)micro.count(), (int)(micro.count() / 1000)));
		}

	private:
		std::chrono::steady_clock::time_point startTime;
		const char* name;
	};
}
#endif
