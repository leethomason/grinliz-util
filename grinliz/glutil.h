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

#ifndef GRINLIZ_UTIL_INCLUDED
#define GRINLIZ_UTIL_INCLUDED

#include <stdint.h>

namespace grinliz {

	/// Minimum
	template <class T> inline T		Min(T a, T b) { return (a < b) ? a : b; }
	/// Maximum
	template <class T> inline T		Max(T a, T b) { return (a > b) ? a : b; }
	/// Minimum
	template <class T> inline T		Min(T a, T b, T c) { return Min(a, Min(b, c)); }
	/// Maximum
	template <class T> inline T		Max(T a, T b, T c) { return Max(a, Max(b, c)); }
	/// Minimum
	template <class T> inline T		Min(T a, T b, T c, T d) { return Min(d, Min(a, b, c)); }
	/// Maximum
	template <class T> inline T		Max(T a, T b, T c, T d) { return Max(d, Max(a, b, c)); }

	/// Swap
	template <class T> inline void	Swap(T& a, T& b) { T temp; temp = a; a = b; b = temp; }
	/// Returns true if value in the range [lower, upper]
	template <class T> inline bool	InRange(const T& a, const T& lower, const T& upper) { return a >= lower && a <= upper; }
	/// Returned the value clamped to the range [lower, upper]
	template <class T> inline T		Clamp(const T& a, const T& lower, const T& upper)
	{
		if (a < lower)
			return lower;
		else if (a > upper)
			return upper;
		return a;
	}

	/// Average (mean) value
	template <class T> inline T		Mean(T t0, T t1) { return (t0 + t1) / T(2); }
	/// Average (mean) value
	template <class T> inline T		Mean(T t0, T t1, T t2) { return (t0 + t1 + t2) / T(3); }

	template<typename T> T Rotate(T value, int r) {
		static const int nBits = 8 * sizeof(T);
		return ((value << r) | (value >> (nBits - r)));
	}

	template<typename T> T HashCombine(T a, T b) {
		return a ^ Rotate(b, sizeof(T) / 3);
	}

	template<typename T>
	bool EqualFromLessThan(const T& a, const T& b) {
		if (a < b) return false;
		if (b < a) return false;
		return true;
	}

	template<typename T, typename LessFunc>
	bool EqualFromLessThan(const T& a, const T& b, LessFunc func) {
		if (func(a, b)) return false;
		if (func(b, a)) return false;
		return true;
	}

	/// Find the highest bit set.
	inline uint32_t LogBase2(uint32_t v)
	{
		// I don't love this implementation, and I
		// don't love the table alternatives either.
		uint32_t r = 0;
		while (v >>= 1) {
			++r;
		}
		return r;
	}

	/// Round down to the next power of 2
	inline uint32_t FloorPowerOf2(uint32_t v)
	{
		v = v | (v >> 1);
		v = v | (v >> 2);
		v = v | (v >> 4);
		v = v | (v >> 8);
		v = v | (v >> 16);
		return v - (v >> 1);
	}

	/// Round up to the next power of 2
	inline uint32_t CeilPowerOf2(uint32_t v)
	{
		v = v - 1;
		v = v | (v >> 1);
		v = v | (v >> 2);
		v = v | (v >> 4);
		v = v | (v >> 8);
		v = v | (v >> 16);
		return v + 1;
	}

	inline bool IsPowerOf2(uint32_t x)
	{
		return x == CeilPowerOf2(x);
	}

	// More useful versions of the "HermiteInterpolate" and "InterpolateUnitX"
	template <class T, class REAL>
	inline T Lerp(const T& x0, const T& x1, REAL time)
	{
		return  x0 + (x1 - x0) * time;
	}


	// Cubic fade (hermite)
	// Use: Lerp( a, b, Fade3( t ));
	inline float Fade3(float t)
	{
		return t * t * (3.0f - 2.0f * t);
	}


	/** Template function for linear interpolation between 4 points, at x=0, x=1, y=0, and y=1.
	*/
	template <class T> inline T BilinearInterpolate(T q00, T q10, T q01, T q11, T x, T y)
	{
		static const T one = static_cast<T>(1);
		return q00 * (one - x) * (one - y) + q10 * (x) * (one - y) + q01 * (one - x) * (y)+q11 * (x) * (y);
	}

	template<typename TYPE, int N>
	class AverageSample
	{
	public:
		AverageSample(TYPE initialValue) {
			for (int i = 0; i < N; ++i) {
				m_sample[i] = initialValue;
				m_total += initialValue;
			}
			m_average = initialValue;
		}

		AverageSample() {
			for (int i = 0; i < N; ++i) {
				m_sample[i] = 0;
			}
			m_total = 0;
			m_average = 0;
		}

		void recount() {
			m_total = 0;
			for (int i = 0; i < N; ++i)
				m_total += m_sample[i];
		}

		void push(TYPE value) {
			m_total -= m_sample[m_pos];
			m_sample[m_pos] = value;
			m_total += value;

			m_pos++;
			if (m_pos == N) {
				m_pos = 0;
				recount();
			}
			m_average = m_total / N;
		}

		void fill(TYPE value) {
			for (int i = 0; i < N; ++i)
				push(value);
		}

		TYPE average() const { return m_average; }
		int numSamples() const { return N; }
		bool origin() const { return m_pos == 0; }

	private:
		TYPE m_average;
		TYPE m_total = 0;
		int m_pos = 0;
		TYPE m_sample[N];
	};

	bool TestUtil();

}; // namespace grinliz

#endif
