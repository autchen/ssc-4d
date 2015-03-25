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
#include "pie_sim_misc.h"

// Project headers
#include "plus/pie_random.h"

// Lib headers
#include "math.h"

// Method implementations

PieBoolean PieSim_randomVectorInt(int size, PieUint32 *output)
{
        if (size < 0 || !output)
                return PIE_FALSE;
        PieUint32 *ptr = output;
        PieUint32 *end = ptr + size;

        while (ptr != end) {
                *ptr = Pie_randomBoolean(0.5);
                ptr++;
        }
        return PIE_TRUE;
}

int PieSim_compareVector(int size, PieUint32 *a, PieUint32 *b)
{
        int count = 0;
        for (int i = 0; i < size; i++) {
                if (a[i] != b[i])
                        count++;
        }
        return count;
}

int PieSim_decideCompare(int size, PieUint32 *a, double *b)
{
        int count = 0;
        for (int i = 0; i < size; i++) {
                PieUint32 c = b[i] > 0 ? 0 : 1;
                if (a[i] != c)
                        count++;
        }
        return count;
}

double PieSim_maxStar(double var1, double var2)
{
        double out = var1 > var2 ? var1 : var2;
        out += log(1 + exp(-fabs(var1 - var2)));
        return out;
}

double PieSim_max(double var1, double var2)
{
        return var1 > var2 ? var1 : var2;
}

double PieSim_AWGN(double sigma2)
{
        double r = 0.0;
        double v1 = 0.0, v2 = 0.0;
        do {
                v1 = 2.0 * Pie_random() - 1.0;
                v2 = 2.0 * Pie_random() - 1.0;
                r = v1 * v1 + v2 * v2;
        } while (r >= 1.0 || r == 0.0);
        double fac = sqrt(-2.0 * sigma2 * log(r) / r);
        return (v2 * fac);
}

#define PIE_SIM_MEASURE_MIN -10000.0

PieBoolean PieSim_bitDemap(// input 
                           int order,
                           double *prob, 
                           PieBoolean maxStar,
                           // output
                           double *output)
{
        if (order < 0 || !prob || !output)
                return PIE_FALSE;
        double (*maxFunc)(double var1, double var2) =
                        maxStar ? PieSim_maxStar : PieSim_max;
        PieUint32 groupNumber = 1 << order; // 2^order
        PieUint32 loopBlock = groupNumber >> 1; // groupNumber / 2
        for (int i = 0; i < order; i++) {
                double measure0 = PIE_SIM_MEASURE_MIN;
                double measure1 = PIE_SIM_MEASURE_MIN;

                PieUint32 k = 0, loopEnd = 0;
                while (k != groupNumber) {
                        for (loopEnd += loopBlock; k < loopEnd; k++)
                                measure0 = maxFunc(measure0, prob[k]);
                        for (loopEnd += loopBlock; k < loopEnd; k++)
                                measure1 = maxFunc(measure1, prob[k]);
                }

                loopBlock >>= 1; // loopBlock /= 2
                output[i] = measure0 - measure1;
        }

        return PIE_TRUE;
}

PieBoolean PieSim_bitDemapExt(// input
                              int order,
                              double *prob,
                              double *ext,
                              PieBoolean maxStar,
                              // output
                              double *output)
{
        if (order < 0 || !prob || !output)
                return PIE_FALSE;
        double (*maxFunc)(double var1, double var2) =
                        maxStar ? PieSim_maxStar : PieSim_max;
        PieUint32 groupNumber = 1 << order; // 2^order
        
        for (int i = 0; i < order; i++) {
                double measure0 = PIE_SIM_MEASURE_MIN;
                double measure1 = PIE_SIM_MEASURE_MIN;

                PieUint32 k = 0, loopEnd = 0;
                PieUint32 loopBlock = groupNumber >> 1; // groupNumber / 2

                while (k != groupNumber) {
                        for (loopEnd += loopBlock; k < loopEnd; k++) {
                                double tmp = prob[k];
                                PieUint32 m = groupNumber >> 1;
                                for (int l = 0; l < order; l++) {
                                        if (l != i)
                                                tmp += (k & m) == 0 ? 
                                                        (ext[l] / 2) : 
                                                        (-ext[l] / 2);
                                        m >>= 1;
                                }
                                measure0 = maxFunc(measure0, tmp);
                        }
                        for (loopEnd += loopBlock; k < loopEnd; k++) {
                                double tmp = prob[k];
                                PieUint32 m = groupNumber >> 1;
                                for (int l = 0; l < order; l++) {
                                        if (l != i)
                                                tmp += (k & m) == 0 ? 
                                                        (ext[l] / 2) : 
                                                        (-ext[l] / 2);
                                        m >>= 1;
                                }
                                measure0 = maxFunc(measure0, tmp);
                        }
                }

                loopBlock >>= 1; // loopBlock /= 2
                output[i] = measure0 - measure1;
        }

        return PIE_TRUE;
}

PieBoolean PieSim_copyVectorComplex(int number, PieComplex *a, PieComplex *b)
{
        if (number < 0)
                return PIE_FALSE;
        for (int i = 0; i < number; i++) {
                b[i] = a[i];
        }
        return PIE_TRUE;
}
