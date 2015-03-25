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
#include "pie_modem_2d.h"

// Project headers
#include "plus/pie_ini_file.h"
#include "simulation/pie_sim_misc.h"

// Lib headers
#include "math.h"

// Constants
#define PIE_MODEM_2D_ORDER 2
#define PIE_MODEM_2D_MAX_ORDER 8
#define PIE_MODEM_2D_SITA 0
#define PIE_MODEM_2D_MAXSTAR PIE_FALSE

static char *s_section = "PieModem2D";

static PieComplex s_mapQPSK[4] = {
        {1.0, 1.0},
        {1.0, -1.0},
        {-1.0, 1.0},
        {-1.0, -1.0}
};

// Internal function

double PieModem2D_distanceNormal(PieComplex a, PieComplex b, double is2)
{
        return -(pow((a.real - b.real), 2) + pow((a.imag - b.imag), 2)) * is2 / 2;
}

double PieModem2D_distanceOnlyI(PieComplex a, PieComplex b, double is2)
{
        return -(pow((a.real - b.real), 2)) * is2 / 2;
}

double PieModem2D_distanceOnlyQ(PieComplex a, PieComplex b, double is2)
{
        return -(pow((a.imag - b.imag), 2)) * is2 / 2;
}

// Method implementations

PieBoolean PieModem2D_init(PieModem2D *self, 
                           struct PieIniFile *ini, 
                           char *section)
{
        // Read config file
        if (!ini)
                return PIE_FALSE;
        char *buf = section ? section : s_section;
        int order = PieIniFile_getProfileInt(
                        ini, buf, "order", PIE_MODEM_2D_ORDER);
        if (order > PIE_MODEM_2D_MAX_ORDER)
                return PIE_FALSE;
        double sita = PieIniFile_getProfileDouble(
                        ini, buf, "sita", PIE_MODEM_2D_SITA);
        PieBoolean maxStar = PieIniFile_getProfileBoolean(
                        ini, buf, "maxStar", PIE_MODEM_2D_MAXSTAR);
        double mapBuf[PIE_MODEM_2D_MAX_POINT * 2];
        int np = PieIniFile_getProfileVectorDouble(
                        ini, buf, "map", PIE_MODEM_2D_MAX_POINT * 2, mapBuf);
        int mapSize = 1 << order;
        if (np != mapSize * 2) {
                if (order != PIE_MODEM_2D_ORDER)
                        return PIE_FALSE;
                for (int i = 0; i < (1 << order); i++) {
                        mapBuf[2 * i] = s_mapQPSK[i].real;
                        mapBuf[2 * i + 1] = s_mapQPSK[i].imag;
                }
        }

        // Init instance
        self->order = order;
        self->maxStar = maxStar;
        self->sita = sita;

        double norm = 0;
        for (int i = 0; i < mapSize; i++) {
                self->oMap[i].real = mapBuf[2 * i];
                self->oMap[i].imag = mapBuf[2 * i + 1];
                norm += self->oMap[i].real * self->oMap[i].real
                                + self->oMap[i].imag * self->oMap[i].imag;
        }
        norm = sqrt(norm / mapSize);
        double m = cos(sita);
        double n = sin(sita);
        for (int i = 0; i < mapSize; i++) {
		self->rMap[i].real = (self->oMap[i].real * m 
                                + self->oMap[i].imag * n) / norm;
		self->rMap[i].imag = (-self->oMap[i].real * n
                                + self->oMap[i].imag * m) / norm;
        }

        return PIE_TRUE;
}

// void PieModem2D_destroy(PieModem2D *self);

PieBoolean PieModem2D_modulate(PieModem2D *self, 
                               // input
                               int inputNumber,
                               PieUint32 *input,
                               int outputNumber,
                               // output
                               PieComplex *output)
{
        if (inputNumber < 0 || !input || !output || (inputNumber & 1) != 0)
                return PIE_FALSE;
        if (inputNumber != outputNumber * self->order)
                return PIE_FALSE;

	int order = self->order;
        int symNum = inputNumber / order;
        PieComplex *map = self->rMap;

        PieUint32 *curInput = input;
        for (int i = 0; i < symNum; i++) {
                PieUint32 index = 0;
                for (int j = 0; j < order; j++) {
                        index <<= 1;
                        index += (*curInput) & 1;
                        curInput++;
                }
                output[i] = map[index];
        }
        
        return PIE_TRUE;
}

PieBoolean PieModem2D_demodulate(PieModem2D *self, 
                                 // input
                                 int inputNumber,
                                 PieComplex *input,
                                 PieComplex *equR,
                                 PieComplex *equI,
                                 double EbN0,
                                 double rate,
                                 double *extin,
                                 int outputNumber,
                                 // output
                                 double *output)
{
	int symNum = inputNumber;
        int order = self->order;
	PieUint32 ptNum = 1 << order; // 2 ^ order
        PieComplex *map = self->rMap;
	double inv_sigma2 = 0; 
        double (*distanceFunc)(PieComplex a, PieComplex b, double is2) 
                        = self->distanceFunc;

	if (!input || !equR || !output || inputNumber < 0 || (inputNumber & 1) != 0)
		return PIE_FALSE;
        if (outputNumber != inputNumber * order)
                return PIE_FALSE;

	if (!equI)
		equI = equR;

        // Noise mode
	if (rate == 0) {
		inv_sigma2 = 2 * pow(10.0, (EbN0 / 10));
	} else {
		inv_sigma2 = 2 * pow(10.0, (EbN0 / 10)) * order * rate;
	}

	double distance[PIE_MODEM_2D_MAX_POINT];
        double *ext = extin;
        double *out = output;
	for (int i = 0; i < symNum; i++) {
		// calc euclid distance of each symbol
		double er = sqrt(equR[i].real * equR[i].real + equR[i].imag * equR[i].imag);
		double ei = sqrt(equI[i].real * equI[i].real + equI[i].imag * equI[i].imag);
		for(int j = 0; j < ptNum; j++) {
                        PieComplex ecp;
			ecp.real = er * map[j].real;
			ecp.imag = ei * map[j].imag;
                        distance[j] = distanceFunc(ecp, input[i], inv_sigma2);
		}
                if (extin) {
                        PieSim_bitDemapExt(order, distance, ext, self->maxStar, out);
                        ext += order;
                } else
                        PieSim_bitDemap(order, distance, self->maxStar, out);
                out += order;
	}
        
	return PIE_TRUE;
}

void PieModem2D_setMode(PieModem2D *self, int mode)
{
        switch (mode) {
                case PIE_MODEM_2D_MODE_ONLY_I:
                        self->distanceFunc = PieModem2D_distanceOnlyI;
                        break;
                case PIE_MODEM_2D_MODE_ONLY_Q:
                        self->distanceFunc = PieModem2D_distanceOnlyQ;
                        break;
                default:
                        self->distanceFunc = PieModem2D_distanceNormal;
        }
}

// TB example

/* #include <stdio.h> */
/* #include <stdlib.h> */
/* #include <string.h> */

/* #include "plus/pie_ini_file.h" */

/* #include "simulation/pie_numeric.h" */
/* #include "simulation/pie_sim_misc.h" */
/* #include "simulation/pie_modem_2d.h" */

/* int main(int argc, char *argv[]) */
/* { */
        /* PieIniFile ini; */
        /* PieIniFile_init(&ini, "resource/test.ini"); */

        /* PieModem2D modem; */
        /* PieModem2D_init(&modem, &ini, 0); */

        /* printf("order %d\n", modem.order); */
        /* printf("sita %f\n", modem.sita); */
        /* printf("maxStar %d\n", modem.maxStar); */
        /* printf("omap\n"); */
        /* for (int i = 0; i < (1 << modem.order); i++) { */
                /* printf("(%f, %f)\n", modem.oMap[i].real, modem.oMap[i].imag); */
        /* } */
        /* printf("rmap\n"); */
        /* for (int i = 0; i < (1 << modem.order); i++) { */
                /* printf("(%f, %f)\n", modem.rMap[i].real, modem.rMap[i].imag); */
        /* } */

        /* PieUint32 input[64]; */
        /* PieSim_randomVectorInt(64, input); */
        /* printf("input\n"); */
        /* for (int i = 0; i < 64; i++) { */
                /* printf("%d ", input[i]); */
        /* } */
        /* printf("\n"); */

        /* PieComplex output[32]; */
        /* PieModem2D_modulate(&modem, 64, input, 0, output); */
        /* printf("output\n"); */
        /* for (int i = 0; i < 32; i++) { */
                /* printf("(%f, %f) ", output[i].real, output[i].imag); */
        /* } */
        /* printf("\n"); */

        /* PieComplex equ[32]; */
        /* for (int i = 0; i < 32; i++) { */
                /* equ[i].real = 0.707; */
                /* equ[i].imag = 0.707; */
        /* } */
        /* double demod[64]; */
        /* for (int i = 0; i < 16; i++) { */
                /* output[i].imag = 0; */
                /* output[i + 16].real = 0; */
        /* } */
        /* PieModem2D_setMode(&modem, PIE_MODEM_2D_MODE_ONLY_I); */
        /* PieModem2D_demodulate( */
                        /* &modem, 16, output, equ, equ, 10, 0, 0,  */
                        /* 0, demod); */
        /* PieModem2D_setMode(&modem, PIE_MODEM_2D_MODE_ONLY_Q); */
        /* PieModem2D_demodulate( */
                        /* &modem, 16, output + 16, equ, equ, 10, 0, 0,  */
                        /* 0, demod + 32); */
        /* printf("demod\n"); */
        /* for (int i = 0; i < 64; i++) { */
                /* printf("%d ", (demod[i] > 0 ? 0 : 1)); */
        /* } */
        /* printf("\n"); */
        
        /* return 0; */
/* } */
