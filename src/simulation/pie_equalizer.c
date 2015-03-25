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
#include "pie_equalizer.h"

// Lib header
#include "math.h"

// Method implementations

PieBoolean PieEqualizer_equalize(// input
                                 int inputNumber,
                                 PieComplex *input,
                                 PieComplex *equIn,
                                 int moduOrder,
                                 int outputNumber,
                                 // output
                                 PieComplex *output)
{
	if (inputNumber < 0 || !input || !equIn || !output)
		return PIE_FALSE;
        if (outputNumber != inputNumber)
                return PIE_FALSE;

        for (int i = 0; i < inputNumber; i++) {
                if (moduOrder == 1) {
                        output[i].real = input[i].real;
                        output[i].imag = 0;
                } else {
                        output[i].real = (input[i].real * equIn[i].real 
                                        + input[i].imag * equIn[i].imag) 
                                        / sqrt(pow(equIn[i].real, 2) 
                                        + pow(equIn[i].imag, 2));
                        output[i].imag = (input[i].imag * equIn[i].real 
                                        - input[i].real * equIn[i].imag) 
                                        / sqrt(pow(equIn[i].real, 2) 
                                        + pow(equIn[i].imag, 2));
                }
        }
        
        return PIE_TRUE;
}
