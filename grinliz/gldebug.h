/*
Copyright (c) 2000-2019 Lee Thomason (www.grinninglizard.com)
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


#if defined( _DEBUG ) || defined( DEBUG ) || defined (__DEBUG__)
#   ifndef DEBUG
#       define DEBUG
#   endif
#endif

#ifndef GRINLIZ_DEBUG_INCLUDED
#define GRINLIZ_DEBUG_INCLUDED

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

#if defined(DEBUG)
#	if defined(_MSC_VER)
#		if _WIN64
#include <assert.h>
			// #define GLASSERT assert
			void WinDebugBreak();
#			define GLASSERT(x)		if (!(x)) WinDebugBreak();
		#else
			void WinDebugBreak();		
#			define GLASSERT( x )		if (!(x)) WinDebugBreak(); //if ( !(x)) { _asm { int 3 } }
#		endif
		#define GLOUTPUT( x )		printf x
		#define GLOUTPUT_REL( x )	printf x
	#elif defined (ANDROID_NDK)
		#include <android/log.h>
		void dprintf( const char* format, ... );
		#define GLASSERT( x )		if ( !(x)) { __android_log_assert( "assert", "grinliz", "ASSERT in '%s' at %d.", __FILE__, __LINE__ ); }
		#define	GLOUTPUT( x )		dprintf x
	#else
		#include <assert.h>
        #include <stdio.h>
		#define GLASSERT			assert
		#define GLOUTPUT( x )		printf x
		#define GLOUTPUT_REL( x )	relprintf x
	#endif
#else
	#define GLOUTPUT( x )
	#define GLOUTPUT_REL( x )		printf x
	#define GLASSERT( x )
#endif

#define GLASSERT_32BIT_ALIGNED(x) GLASSERT(((uintptr_t)(x) & 0x03) == 0);
#define GLASSERT_64BIT_ALIGNED(x) GLASSERT(((uintptr_t)(x) & 0x07) == 0);

#define	GLTEST(x)															\
	GLASSERT(x);															\
	if (!(x)) {																\
		GLOUTPUT_REL(("TEST FAIL in '%s' at %d.\n", __FILE__, __LINE__));	\
		GLOUTPUT(("TEST FAIL in '%s' at %d.\n", __FILE__, __LINE__));		\
	}

#endif // GRINLIZ_DEBUG_INCLUDED

