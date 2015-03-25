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
#include "pie_ssd_interleaver.h"

// Method implementations

PieBoolean PieSSDInterleaver_splitQ(int inputNumber, 
                                    PieComplex *input, 
                                    PieComplex *output)
{
        if (inputNumber < 0 || (inputNumber & 1) != 0)
                return PIE_FALSE;
        int ii = inputNumber / 2;
        for (int i = 0; i < ii; i++) {
                output[i].real = input[i].real;
                output[i].imag = input[i + ii].imag;
        }
        for (int i = ii; i < inputNumber; i++) {
                output[i].real = input[i].real;
                output[i].imag = input[i - ii].imag;

        }
        return PIE_TRUE;
}

PieBoolean PieSSDInterleaver_split2(int inputNumber, 
                                    PieComplex *input,
                                    PieComplex *output)
{
        if (inputNumber < 0 || (inputNumber & 1) != 0)
                return PIE_FALSE;
        int ii = inputNumber / 2;
        for (int i = 0; i < ii; i++) {
                output[i] = input[2 * i];
                output[i + ii] = input[2 * i + 1];
        }
        return PIE_TRUE;
}

PieBoolean PieSSDInterleaver_combine2(int inputNumber, 
                                      PieComplex *input,
                                      PieComplex *output)
{
        if (inputNumber < 0 || (inputNumber & 1) != 0)
                return PIE_FALSE;
        int ii = inputNumber / 2;
        for (int i = 0; i < ii; i++) {
                output[2 * i] = input[i];
                output[2 * i + 1] = input[i + ii];
        }
        return PIE_TRUE;
}

PieBoolean PieSSDInterleaver_combineEqu(int inputNumber,
                                        PieComplex *equIn,
                                        PieComplex *equOut)
{
        if (inputNumber < 0 || (inputNumber & 1) != 0)
                return PIE_FALSE;
        int ii = inputNumber / 2;
        for (int i = 0; i < ii; i++)
                equOut[i] = equIn[i + ii];
        for (int i = ii; i < inputNumber; i++)
                equOut[i] = equIn[i - ii];
        return PIE_TRUE;
}

