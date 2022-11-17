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


#ifndef GRINLIZ_RANDOM_INCLUDED
#define GRINLIZ_RANDOM_INCLUDED

#include "glutil.h"
#include "SpookyV2.h"
#include "gldebug.h"
#include <stdint.h>
#include <string.h>

namespace grinliz {

    inline uint64_t Hash64(const void* data, int n)
    {
        return SpookyHash::Hash64(data, n, 0);
    }

    inline uint64_t Hash64(const char* str) {
        if (!str || !*str)
            return 0;
        size_t n = strlen(str);
        return SpookyHash::Hash64(str, n, 0);
    }

    inline uint32_t Hash32(const void* data, int n)
    {
        return SpookyHash::Hash32(data, n, 0);
    }

    inline uint32_t Hash32(const char* str) 
    {
        if (!str || !*str)
            return 0;
        size_t n = strlen(str);
        return SpookyHash::Hash32(str, n, 0);
    }

    /**	Fast simple pseudo random number generator.
    */
    class Random
    {
    public:
        /// Constructor, with optional seed value.
        Random(uint32_t seed = 1) { SetSeed(seed); }

        /** The current seed can be set at any time to
            guarentee a certain sequence of random numbers.
        */
        void SetSeed(uint32_t seed) {
            if (seed == 0) seed = 1;
            x = seed;
        }

        /// Returns a 32 bit random number.
        uint32_t Rand() {
            x ^= x << 13;
            x ^= x >> 17;
            x ^= x << 5;
            return x;
        }

        /** Returns a random number greater than or equal to 0, and less
            that 'upperBound'.
        */
        uint32_t Rand(uint32_t upperBound) {	
            return Rand() % upperBound;
        }

        /** "Roll dice." Has the same bell curve distribution as N dice. Dice start with the
            value '1', so RandD2( 2, 6 ) returns a value from 2-12
        */
        int Dice(uint32_t nDice, uint32_t sides) {
            int total = 0;
            for (uint32_t i = 0; i < nDice; ++i) { total += (int)Rand(sides) + 1; }
            return total;
        }

        /** Like Dice, but returns a value between 0 and 1.
        */
        float DiceUniform(uint32_t nDice, uint32_t sides)
        {
            int d = Dice(nDice, sides);
            int minRoll = nDice;
            int maxRoll = sides * nDice;
            return (float(d) - float(minRoll)) / (float(maxRoll - minRoll));
        }

        /// Return a random number from 0 to upper: [0.0,1.0].
        float Uniform() {
            static const float INV = 1.0f / 4095.0f;
            uint32_t r = Rand(4096);
            return float(r) * INV;
        }

        /// Return 0 or 1
        int Bit()
        {
            uint32_t v = Rand();
            return v & 1;
        }

        /// Return a random boolean.
        bool Boolean()
        {
            return Bit() ? true : false;
        }

        /// Return +1 or -1
        int Sign()
        {
            return -1 + 2 * Bit();
        }

        template< class T >
        void ShuffleArray(T* mem, int size) {
            for (int i = 0; i < size; ++i) {
                int j = Rand(size);
                Swap(mem[i], mem[j]);
            }
        }

        static uint32_t Mix(uint32_t a) {
            // https://burtleburtle.net/bob/hash/integer.html
            a -= (a << 6);
            a ^= (a >> 17);
            a -= (a << 9);
            a ^= (a << 4);
            a -= (a << 3);
            a ^= (a << 10);
            a ^= (a >> 15);
            return a;
        }

        static double MixUniform(uint32_t a) {
            static const double INV = 1.0 / 65535;
            return (Mix(a) & 0xffff) * INV;
        }

    private:
        uint32_t x;
    };

}	// namespace grinliz

#endif
