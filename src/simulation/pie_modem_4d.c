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
#include "pie_modem_4d.h"

// Project headers
#include "plus/pie_alloc.h"
#include "plus/pie_ini_file.h"
#include "simulation/pie_sim_misc.h"

// Lib headers
#include "math.h"

// Constants
#define PIE_MODEM_4D_ORDER 2
#define PIE_MODEM_4D_MAX_ORDER 8
#define PIE_MODEM_4D_SITA 0
#define PIE_MODEM_4D_MAXSTAR PIE_FALSE

// Internal struct
typedef struct PieComplex2 {
        PieComplex s1;
        PieComplex s2;
} PieComplex2;

static char *s_section = "PieModem4D";

static PieComplex s_mapQPSK[4] = {
        {1.0, 1.0},
        {1.0, -1.0},
        {-1.0, 1.0},
        {-1.0, -1.0}
};

// Internal function

double PieModem4D_calcDistance(PieComplex2 *a, 
                               PieComplex2 *b, 
                               double is2, 
                               PieUint32 flag)
{
        double distance = 0;
        if ((flag & PIE_MODEM_4D_A) != 0)
                distance += pow((a->s1.real - b->s1.real), 2);
        if ((flag & PIE_MODEM_4D_B) != 0)
                distance += pow((a->s1.imag - b->s1.imag), 2);
        if ((flag & PIE_MODEM_4D_C) != 0)
                distance += pow((a->s2.real - b->s2.real), 2);
        if ((flag & PIE_MODEM_4D_D) != 0)
                distance += pow((a->s2.imag - b->s2.imag), 2);
        distance = -distance * is2 / 2;
        return distance;
}

// Method implementations

PieBoolean PieModem4D_init(PieModem4D *self, 
                           struct PieIniFile *ini,
                           char *section)
{
        // Read config file
        if (!ini)
                return PIE_FALSE;
        char *buf = section ? section : s_section;
        int order = PieIniFile_getProfileInt(
                        ini, buf, "order", PIE_MODEM_4D_ORDER);
        if (order > PIE_MODEM_4D_MAX_ORDER)
                return PIE_FALSE;
        double sita1 = PieIniFile_getProfileDouble(
                        ini, buf, "sita1", PIE_MODEM_4D_SITA);
        double sita2 = PieIniFile_getProfileDouble(
                        ini, buf, "sita2", PIE_MODEM_4D_SITA);
        PieBoolean maxStar = PieIniFile_getProfileBoolean(
                        ini, buf, "maxStar", PIE_MODEM_4D_MAXSTAR);
        double mapBuf[PIE_MODEM_4D_MAX_POINT * 2];
        int np = PieIniFile_getProfileVectorDouble(
                        ini, buf, "map", PIE_MODEM_4D_MAX_POINT * 2, mapBuf);
        int mapSize = 1 << order;
        if (np != mapSize * 2) {
                if (order != PIE_MODEM_4D_ORDER)
                        return PIE_FALSE;
                for (int i = 0; i < (1 << order); i++) {
                        mapBuf[2 * i] = s_mapQPSK[i].real;
                        mapBuf[2 * i + 1] = s_mapQPSK[i].imag;
                }
        }

        // Init instance
        self->order = order;
        self->maxStar = maxStar;
        self->sita1 = sita1;
        self->sita2 = sita2;
        self->componentFlag = PIE_MODEM_4D_ALL;

        double norm = 0;
        for (int i = 0; i < mapSize; i++) {
                self->oMap[i].real = mapBuf[2 * i];
                self->oMap[i].imag = mapBuf[2 * i + 1];
                norm += self->oMap[i].real * self->oMap[i].real
                                + self->oMap[i].imag * self->oMap[i].imag;
        }
        norm = sqrt(norm / mapSize);

        double m1 = cos(sita1);
        double n1 = sin(sita1);
        double m2 = cos(sita2);
        double n2 = sin(sita2);
        int ptNum = 1 << order; // 2^(2*order)
        PieComplex2 *rMap = Pie_calloc(ptNum * ptNum, sizeof *rMap);
        PieComplex *oMap = self->oMap;
        for (int i = 0; i < ptNum; i++) {
                for (int j = 0; j < ptNum; j++) {
                        PieUint32 index = i * ptNum + j; 
                        rMap[index].s1.real = (oMap[i].real * m1 * m2 
                                        + oMap[i].imag * n1 * m2 
                                        + oMap[j].real * m1 * n2 
                                        + oMap[j].imag * n1 * n2) / norm;
                        rMap[index].s1.imag = (-oMap[i].real * n1 * m2 
                                        + oMap[i].imag * m1 * m2 
                                        - oMap[j].real * n1 * n2 
                                        + oMap[j].imag * m1 * n2) / norm;
                        rMap[index].s2.real = (-oMap[i].real * m1 * n2 
                                        - oMap[i].imag * n1 * n2 
                                        + oMap[j].real * m1 * m2 
                                        + oMap[j].imag * n1 * m2) / norm;
                        rMap[index].s2.imag = (oMap[i].real * n1 * n2 
                                        - oMap[i].imag * m1 * n2 
                                        - oMap[j].real * n1 * m2 
                                        + oMap[j].imag * m1 * m2) / norm;
                }
        }
        self->rMap = rMap;
        self->distance = Pie_calloc(ptNum * ptNum, sizeof *(self->distance));

        return PIE_TRUE;
}

void PieModem4D_destroy(PieModem4D *self)
{
        Pie_free(self->rMap);
        Pie_free(self->distance);
}

PieBoolean PieModem4D_modulate(PieModem4D *self,
                               // input
                               int inputNumber,
                               PieUint32 *input,
                               int outputNumber,
                               // output
                               PieComplex *output)
{
        if (inputNumber < 0 || (inputNumber & 1) != 0 || !input || !output)
                return PIE_FALSE;
        if (inputNumber != outputNumber * self->order)
                return PIE_FALSE;

	int order2 = self->order * 2;
        int symNum2 = inputNumber / order2;
        PieComplex2 *map = self->rMap;

        PieUint32 *curInput = input;
        PieComplex *curOutput = output;
        for (int i = 0; i < symNum2; i++) {
                PieUint32 index = 0;
                for (int j = 0; j < order2; j++) {
                        index <<= 1;
                        index +=(*curInput) & 1;
                        curInput++;
                }
                *(curOutput) = map[index].s1;
                *(curOutput + 1) = map[index].s2;
                curOutput += 2;
        }

        return PIE_TRUE;
}

PieBoolean PieModem4D_demodulate(PieModem4D *self,
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
	int symNum2 = inputNumber / 2;
        int order2 = self->order * 2;
	PieUint32 ptNum = 1 << order2; // 2 ^ order2
        PieComplex2 *map = self->rMap;
	double inv_sigma2 = 0; 

	if (!input || !equR || !output || inputNumber < 0 || (inputNumber & 1) != 0)
		return PIE_FALSE;
        if (outputNumber != symNum2 * order2)
                return PIE_FALSE;

	if (!equI) {
		equI = equR;
	}

        // Noise mode
	if (rate == 0) {
		inv_sigma2 = 2 * pow(10.0, (EbN0 / 10));
	} else {
		inv_sigma2 = 2 * pow(10.0, (EbN0 / 10)) * self->order * rate;
	}

        double *distance = self->distance;
        double *ext = extin;
        double *out = output;
	for (int i = 0; i < symNum2; i++) {
		// calc euclid distance of each symbol

                int xx = 2 * i;
		double er1 = sqrt(equR[xx].real * equR[xx].real 
                                + equR[xx].imag * equR[xx].imag);
		double ei1 = sqrt(equI[xx].real * equI[xx].real 
                                + equI[xx].imag * equI[xx].imag);
                double er2 = sqrt(equR[xx + 1].real * equR[xx + 1].real 
                                + equR[xx + 1].imag * equR[xx + 1].imag);
		double ei2 = sqrt(equI[xx + 1].real * equI[xx + 1].real 
                                + equI[xx + 1].imag * equI[xx + 1].imag);

		for (int j = 0; j < ptNum; j++) {
                        PieComplex2 ecp;
			ecp.s1.real = er1 * map[j].s1.real;
			ecp.s1.imag = ei1 * map[j].s1.imag;
                        ecp.s2.real = er2 * map[j].s2.real;
                        ecp.s2.imag = ei2 * map[j].s2.imag;

                        PieComplex2 rsv;
                        rsv.s1 = input[xx];
                        rsv.s2 = input[xx + 1];

                        distance[j] = PieModem4D_calcDistance(
                                        &ecp, &rsv, inv_sigma2, 
                                        self->componentFlag);
		}
                if (extin) {
                        PieSim_bitDemapExt(
                                        order2, distance, ext, self->maxStar,
                                        out);
                        ext += order2;
                } else
                        PieSim_bitDemap(order2, distance, self->maxStar, out);
                out += order2;
	}
        
	return PIE_TRUE;
}

void PieModem4D_enableComponents(PieModem4D *self, PieUint32 flag)
{
        self->componentFlag = flag;
}

// TestBench

/* #include <stdio.h> */
/* #include <stdlib.h> */
/* #include <string.h> */

/* #include "plus/pie_ini_file.h" */

/* #include "simulation/pie_numeric.h" */
/* #include "simulation/pie_sim_misc.h" */
/* #include "simulation/pie_modem_4d.h" */

/* int main(int argc, char *argv[]) */
/* { */
        /* PieIniFile ini; */
        /* PieIniFile_init(&ini, "resource/test.ini"); */

        /* PieModem4D modem; */
        /* PieModem4D_init(&modem, &ini, 0); */

        /* printf("order %d\n", modem.order); */
        /* printf("sita1 %f\n", modem.sita1); */
        /* printf("sita2 %f\n", modem.sita2); */
        /* printf("maxStar %d\n", modem.maxStar); */
        /* printf("omap\n"); */
        /* for (int i = 0; i < (1 << modem.order); i++) { */
                /* printf("(%f, %f)\n", modem.oMap[i].real, modem.oMap[i].imag); */
        /* } */
        /* printf("rmap\n"); */
        /* for (int i = 0; i < (1 << (2 * modem.order)); i++) { */
                /* printf("(%f, %f | %f, %f)\n",  */
                                /* modem.rMap[i].s1.real,  */
                                /* modem.rMap[i].s1.imag, */
                                /* modem.rMap[i].s2.real, */
                                /* modem.rMap[i].s2.imag); */
        /* } */

        /* PieUint32 input[64]; */
        /* PieSim_randomVectorInt(64, input); */
        /* printf("input\n"); */
        /* for (int i = 0; i < 64; i++) { */
                /* printf("%d ", input[i]); */
        /* } */
        /* printf("\n"); */

        /* PieComplex output[32]; */
        /* PieModem4D_modulate(&modem, 64, input, 0, output); */
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
        /* PieModem4D_enableComponents(&modem, PIE_MODEM_4D_C | PIE_MODEM_4D_A); */
        /* PieModem4D_demodulate( */
                        /* &modem, 32, output, equ, equ, 10, 0, 0,  */
                        /* 0, demod); */
        /* printf("demod\n"); */
        /* for (int i = 0; i < 64; i++) { */
                /* printf("%d ", (demod[i] > 0 ? 0 : 1)); */
        /* } */
        /* printf("\n"); */

        /* PieModem4D_destroy(&modem); */
        
        /* return 0; */
/* } */

/* #include <stdio.h> */
/* #include <stdlib.h> */
/* #include <string.h> */

/* #include "plus/pie_ini_file.h" */
/* #include "simulation/pie_sim_misc.h" */
/* #include "simulation/pie_trellis.h" */
/* #include "simulation/pie_modem_4d.h" */
/* #include "simulation/pie_channel.h" */

/* int main(int argc, char *argv[]) */
/* { */
        /* PieIniFile ini; */
        /* PieIniFile_init(&ini, "resource/test.ini"); */
        /* PieTrellis t; */
        /* PieTrellis_init(&t, &ini, 0); */
        /* PieModem4D m; */
        /* PieModem4D_init(&m, &ini, 0); */
        /* PieIniFile_destroy(&ini); */
        /* PieUint32 a[29]; */
        /* PieSim_randomVectorInt(29, a); */
        /* printf("in\n"); */
        /* for (int i = 0; i < 29; i++) { */
                /* printf("%d ", a[i]); */
        /* } */
        /* printf("\n"); */
        /* PieUint32 b[64]; */
        /* PieUint32 *c = b + 32; */
        /* PieUint32 end; */
        /* PieTrellis_encode(&t, 29, a, 0, 32, 3, &end, b, c); */
        /* printf("out\n"); */
        /* for (int i = 0; i < 32; i++) { */
                /* printf("%d ", b[i]); */
        /* } */
        /* printf("s%d\n", end);    */
        /* printf("out\n"); */
        /* for (int i = 0; i < 32; i++) { */
                /* printf("%d ", c[i]); */
        /* } */
        /* printf("\n"); */

        /* PieComplex cc[32]; */
        /* PieModem4D_modulate(&m, 64, b, 32, cc); */

        /* printf("modu\n"); */
        /* for (int i = 0; i < 32; i++) { */
                /* printf("(%f %f) ", cc[i].real, cc[i].imag); */
        /* } */
        /* printf("\n"); */

        /* double EbN0 = 1; */
        /* PieChannel_AWGN(32, cc, EbN0, 0.5, 2); */

        /* printf("chan\n"); */
        /* for (int i = 0; i < 32; i++) { */
                /* printf("(%f %f) ", cc[i].real, cc[i].imag); */
        /* } */
        /* printf("\n"); */

        /* PieComplex equ[32]; */
        /* for (int i = 0; i < 32; i++) { */
                /* equ[i].real = 0.707; */
                /* equ[i].imag = 0.707; */
        /* } */

        /* double d[64], f[32]; */
        /* double *e = d + 32; */

        /* PieModem4D_enableComponents(&m, PIE_MODEM_4D_A | PIE_MODEM_4D_C); */
        /* void PieModem4D_enableComponents(PieModem4D *self, PieUint32 flag); */
        /* PieModem4D_demodulate(&m, 32, cc, equ, 0, EbN0, 0.5, 0, 64, d); */

        /* for (int i = 0; i < 32; i++) { */
                /* f[i] = 0; */
        /* } */

        /* printf("demod\n"); */
        /* for (int i = 0; i < 64; i++) { */
                /* printf("%f ", d[i]); */
        /* } */
        /* printf("\n");    */

        /* double g[35], h[35]; */
        /* PieTrellis_upperDecode(&t, 32, 3, 0, 0, d, e, f, g, h); */
        /* printf("decode\n"); */
        /* for (int i = 0; i < 32; i++) { */
                /* printf("%f ", g[i]); */
        /* } */
        /* printf("\n"); */
        /* printf("decode\n"); */
        /* for (int i = 0; i < 32; i++) { */
                /* printf("%f ", h[i]); */
        /* } */
        /* printf("\n"); */
        /* printf("decode\n"); */
        /* int count = 0; */
        /* for (int i = 0; i < 29; i++) { */
                /* int x = g[i] > 0 ? 0 : 1; */
                /* printf("%d ", x); */
                /* if (x != a[i]) */
                        /* count++; */
        /* } */
        /* printf("\n"); */
        /* printf("decode\n"); */
        /* for (int i = 0; i < 32; i++) { */
                /* int x = h[i] > 0 ? 0 : 1; */
                /* printf("%d ", x); */
        /* } */
        /* printf("\n"); */

        /* printf("%d\n", count); */

        /* PieTrellis_destroy(&t); */
        /* PieModem4D_destroy(&m); */
        /* return 0; */
/* } */
