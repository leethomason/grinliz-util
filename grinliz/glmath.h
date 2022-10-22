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


#ifndef GRINLIZ_MATH_INCLUDED
#define GRINLIZ_MATH_INCLUDED

#include <glm/glm.hpp>

#include "gldebug.h"
#include "glutil.h"
#include <math.h>

namespace grinliz
{
constexpr float PI = 3.1415926535897932384626433832795f;
constexpr float TWO_PI = 2.0f*3.1415926535897932384626433832795f;
constexpr float TAU = TWO_PI;
constexpr double PI_D = 3.1415926535897932384626433832795;
constexpr double TWO_PI_D = 2.0*3.1415926535897932384626433832795;

constexpr float RAD_TO_DEG = (float)( 360.0 / ( 2.0 * PI ) );
constexpr float DEG_TO_RAD = (float)( ( 2.0 * PI ) / 360.0f );
constexpr double RAD_TO_DEG_D = ( 360.0 / ( 2.0 * PI_D ) );
constexpr double DEG_TO_RAD_D = ( ( 2.0 * PI_D ) / 360.0 );
constexpr float SQRT2 = 1.4142135623730950488016887242097f;
constexpr float SQRT2OVER2 = float( 1.4142135623730950488016887242097 / 2.0 );
constexpr float INV_SQRT2 = 0.70710678118654752440084436210485f;


inline float ToDegree( float radian ) { return radian * RAD_TO_DEG; }
inline double ToDegree( double radian ) { return radian * RAD_TO_DEG_D; }
inline float ToRadian( float degree ) { return degree * DEG_TO_RAD; }
inline double ToRadian( double degree ) { return degree * DEG_TO_RAD_D; }

template< class T > T Square(T t) { return t*t;  }

inline float NormalizeAngleDegrees( float alpha ) {
	GLASSERT( alpha >= -360.f && alpha <= 720.0f );	// check for reasonable input
	while ( alpha < 0.0f )		alpha += 360.0f;
	while ( alpha >= 360.0f )	alpha -= 360.0f;
	return alpha;
}

inline int NormalizeAngleDegrees( int alpha ) {
	GLASSERT( alpha >= -360 && alpha <= 720 );
	while ( alpha < 0 )		alpha += 360;
	while ( alpha >= 360 )	alpha -= 360;
	return alpha;
}

inline float ModTau(float r)
{
	return fmodf(r, TAU);
}

inline float ShortestAngularDistance(float target, float source)
{
	float a = target - source;
	a = ModTau(a + TAU + PI) - PI;
	return a;
}

inline float NormalizeAngle(float theta)
{
	GLASSERT(theta < 2 * TAU);
	GLASSERT(theta > -2 * TAU);
	return ModTau(theta + 2.0f * TAU);
}

template<typename T>
bool Equal(T x, T y, T error) {
    return fabs(x - y) <= error;
}

inline bool Equal(const glm::vec2& a, const glm::vec2& b, float err) {
	for (int i = 0; i < 2; ++i) {
		if (fabs(a[i] - b[i]) > err)
			return false;
	}
	return true;
}

inline bool Equal(const glm::vec3& a, const glm::vec3& b, float err) {
	for (int i = 0; i < 3; ++i) {
		if (fabs(a[i] - b[i]) > err)
			return false;
	}
	return true;
}

inline bool Equal(const glm::vec4& a, const glm::vec4& b, float err) {
	for (int i = 0; i < 4; ++i) {
		if (fabs(a[i] - b[i]) > err)
			return false;
	}
	return true;
}

/// Template to return the average of 2 numbers.
template <class T> 
inline T Average( T y0, T y1 )
{
	return ( y0 + y1 ) / T( 2 );
}

template <class T> 
T Sign( T x ) {
	if ( x > 0 ) return 1;
	if ( x < 0 ) return -1;
	return 0;
}

inline bool IsPrime(int n) {
	if (n == 0 || n == 1 || n == 2) {
		return false;
	}
	for (int i = 2; i <= n / 2; ++i) {
		if (n % i == 0) {
			return false;
		}
	}
	return true;
}

inline bool IsOdd(int n) {
	return (n & 1) == 1;
}

};	// namespace grinliz

inline float Fractf(float f)
{
	float intPart;
	return modf(f, &intPart);
}

#endif
