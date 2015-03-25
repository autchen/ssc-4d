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
#include "pie_channel.h"

// Project headers
#include "pie_sim_misc.h"

// Lib header
#include "math.h"

// Method implementations

PieBoolean PieChannel_AWGN(int inputNumber,
                           PieComplex *input, // IO
                           double EbN0,
                           double rate,
                           int moduOrder)
{
        if (!input || inputNumber < 0)
                return PIE_FALSE;

        double sigma = 0;
        if (rate == 0)
                sigma = sqrt(pow(10.0, (-EbN0 / 10)) / 2);
        else
                sigma = 1 / sqrt(2 * pow(10.0, (EbN0 / 10)) * moduOrder * rate);
        double sigma2 = sigma * sigma;

        if (moduOrder == 1) {
                for (int i = 0; i < inputNumber; i++)
                        input[i].real += PieSim_AWGN(sigma2);
        } else {
                for (int i = 0; i < inputNumber; i++) {
                        input[i].real += PieSim_AWGN(sigma2);
                        input[i].imag += PieSim_AWGN(sigma2);
                }
        }

        return PIE_TRUE;
}

PieBoolean PieChannel_fading(// input
                             int inputNumber,
                             PieComplex *input, // IO
                             int moduOrder,
                             int blockSize,
                             // output
                             PieComplex *equOut)
{
        if (inputNumber < 0 || !input || equOut || blockSize <= 0)
                return PIE_FALSE;

        PieComplex fading = {0.0, 0.0};
        fading.real = PieSim_AWGN(0.5);
        fading.imag = PieSim_AWGN(0.5);
        int blockCount = 0;

        for (int i = 0; i < inputNumber; i++) {
                if (blockCount == blockSize) {
                        fading.real = PieSim_AWGN(0.5);
                        fading.imag = PieSim_AWGN(0.5);
                        blockCount = 0;
                }
                blockCount++;
                if (moduOrder == 1) {
                        double a = sqrt(pow(fading.real, 2) + pow(fading.imag, 2));
                        input[i].real *= a;
                        equOut[i].real = a;
                        equOut[i].imag = 0;
                } else {
                        double a = fading.real * input[i].real 
                                - fading.imag * input[i].imag;
                        double b = fading.real * input[i].imag
                                + fading.imag * input[i].real;
                        input[i].real = a;
                        input[i].imag = b;
                        equOut[i] = fading;
                }
        }
	
	return PIE_TRUE;
}

PieBoolean PieChannel_iFading(// input
                              int inputNumber,
                              PieComplex *input, // IO
                              PieComplex *fading, // IO
                              int moduOrder,
                              // output
                              PieComplex *equOut)
{
        if (inputNumber < 0 || !input || !equOut || !fading)
                return PIE_FALSE;

        if (fading->real == 0 && fading->imag == 0) {
                fading->real = PieSim_AWGN(0.5);
                fading->imag = PieSim_AWGN(0.5);
        }

        for (int i = 0; i < inputNumber; i++) {
                if (moduOrder == 1) {
                        double a = sqrt(pow(fading->real, 2) + pow(fading->imag, 2));
                        input[i].real *= a;
                        equOut[i].real = a;
                        equOut[i].imag = 0;
                } else {
                        double a = fading->real * input[i].real 
                                - fading->imag * input[i].imag;
                        double b = fading->real * input[i].imag
                                + fading->imag * input[i].real;
                        input[i].real = a;
                        input[i].imag = b;
                        equOut[i] = *fading;
                }
        }
	
	return PIE_TRUE;
}
