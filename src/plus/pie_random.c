/* Copyright (C) 
 * 2011 - Qiuwen Chen
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 * 
 */

// Primary header
#include "pie_random.h"

// Lib header
#include "time.h"

static PieSint64 s_seed = 12345L;

// Method implementations

/**
 * @brief Generate pseudo random number
 *
 * @return number between 0 and 1
 */
double Pie_random()
{
        static PieSint64 s2 = 12345473464L;
        PieSint64 z = 0L, k = 0L;

        k = s_seed / 53668L;
        s_seed = 40014L * (s_seed - k * 53668L) - k * 12211L;
        if (s_seed < 0)
                s_seed = s_seed + 2147483563L;
        k = s2 / 52774L;
        s2 = 40692L * (s2 - k * 52774L) - k * 3791L;
        if (s2 < 0)
                s2 = s2 + 2147483399L;
        z = s_seed - s2;
        if (z < 1)
                z = z + 2147483562L;
        return ((double)z) / ((double)2147483563.0);
}

/**
 * @brief Return random boolean
 *
 * @param pTrue - probability that output is true
 *
 * @return random boolean
 */
PieBoolean Pie_randomBoolean(double pTrue)
{
        if (pTrue > 1)
                pTrue = 1.0;
        else if (pTrue < 0)
                pTrue = 0.0;
        return Pie_random() <= pTrue ? PIE_TRUE : PIE_FALSE;
}

/**
 * @brief Return random number in a indicated range
 *
 * @param low
 * @param high
 *
 * @return random number
 */
double Pie_randomInterval(double low, double high)
{
        return low + (high - low) * Pie_random();
}

/**
 * @brief Seed the pseudo random process
 *
 * @param seed - 0 for automatic seeding
 */
void Pie_randomSeed(PieUint64 seed)
{
        if (seed)
                s_seed = seed;
        else
                s_seed = (PieUint64)time(0);
}

