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
#include "pie_trellis.h"

// Project headers
#include "plus/pie_cstring.h"
#include "plus/pie_alloc.h"
#include "plus/pie_ini_file.h"
#include "simulation/pie_sim_misc.h"

// Lib headers
#include "math.h"
#include "stdio.h"
#include "stdarg.h"

// Constants
#define PIE_TRELLIS_TRANSFER_UNDEFINE(T) ((T)->fromState = 0xFFFFFFFF)
#define PIE_TRELLIS_TRANSFER_IS_DEFINED(T) ((T)->fromState != 0xFFFFFFFF)

#define PIE_TRELLIS_BUFFER_INIT_SIZE 3000
#define PIE_MAXE 80.0
#define PIE_INFI_0 20.0
#define PIE_INFI_1 1.0e10
#define PIE_INFI 1.0e20
#define PIE_TINY 1.0e-310

#define PIE_TRELLIS_OVERFLOW_LIMIT
// #define TEST_BENCH

static char *s_section = "PieTrellis";

// Internal struct
typedef struct PieTrellisTransfer {
        PieUint32 fromState;
        PieUint32 toState;
        PieUint32 system;
        PieUint32 parity;
} PieTrellisTransfer;

// Method implementations

PieBoolean PieTrellis_init(PieTrellis *self, 
                           struct PieIniFile *ini, 
                           char *section)
{
        if (!ini)
                return PIE_FALSE;
        if (!section)
                section = s_section;

        int stateNumber = PieIniFile_getProfileInt(ini, 
                        section, "stateNumber", 0);
        if (stateNumber <= 0)
                return PIE_FALSE;
        int systemNumber = PieIniFile_getProfileInt(ini, 
                        section, "systemNumber", 0);
        if (systemNumber <= 0)
                return PIE_FALSE;
        int parityNumber = PieIniFile_getProfileInt(ini,
                        section, "parityNumber", 0);
        if (parityNumber <= 0)
                return PIE_FALSE;


        int branchNumber = 1 << systemNumber; // 2^systemNumber
        int parityType = 1 << parityNumber;

        int valueBuf[100];
        int ii = PieIniFile_getProfileVectorInt(ini, 
                        section, "tail", 100, valueBuf);
        if (ii != stateNumber && ii != 1)
                return PIE_FALSE;
        PieUint32 *tail = Pie_calloc(stateNumber, sizeof *tail);
        for (int i = 0; i < stateNumber; i++) {
                if (ii == 1)
                        tail[i] = valueBuf[0];
                else {
                        if (valueBuf[i] < 0 || valueBuf[i] >= branchNumber) {
                                Pie_free(tail);
                                return PIE_FALSE;
                        }
                        tail[i] = valueBuf[i];
                }
        }
        PieUint32 state = (PieUint32)stateNumber;
        int tailNumber = 0;
        for (tailNumber = 0; state != 1; state >>= 1, tailNumber++);
        tailNumber /= systemNumber;
        self->tailNumber = tailNumber;

        char nameBuf[100];
        PieBoolean fail = PIE_FALSE;
        PieTrellisTransfer **next = Pie_calloc(stateNumber, sizeof *next);
        for (int i = 0; i < stateNumber; i++)
                next[i] = Pie_calloc(branchNumber, sizeof *(*next));
        for (int i = 0; i < stateNumber; i++) {
                for (int j = 0; j < branchNumber; j++) {
                        sprintf(nameBuf, "state%dsystem%d", i, j);
                        ii = PieIniFile_getProfileVectorInt(ini,
                                        section, nameBuf, 100, valueBuf);
                        if (ii != 2 || valueBuf[0] < 0 
                                        || valueBuf[0] >= parityType
                                        || valueBuf[1] < 0
                                        || valueBuf[1] >= stateNumber) {
                                fail = PIE_TRUE;
                                i = stateNumber;
                                break;
                        }
                        next[i][j].fromState = i;
                        next[i][j].system = j;
                        next[i][j].parity = valueBuf[0];
                        next[i][j].toState = valueBuf[1];
                }
        }
        if (fail) {
                for (int i = 0; i < stateNumber; i++)
                        Pie_free(next[i]);
                Pie_free(next);
                return PIE_FALSE;
        }

        PieTrellisTransfer **last = Pie_calloc(stateNumber, sizeof *last);
        for (int i = 0; i < stateNumber; i++) {
                last[i] = Pie_calloc(branchNumber, sizeof *(*last));
                for (int j = 0; j < branchNumber; j++)
                        PIE_TRELLIS_TRANSFER_UNDEFINE(last[i] + j);
        }
        for (int i = 0; i < stateNumber; i++) {
                for (int j = 0; j < branchNumber; j++) {
                        int ns = next[i][j].toState;
                        PieTrellisTransfer *tt = last[ns];
                        while (PIE_TRELLIS_TRANSFER_IS_DEFINED(tt))
                                tt++;
                        *tt = next[i][j];
                }
        }

        self->stateNumber = stateNumber;
        self->systemNumber = systemNumber;
        self->parityNumber = parityNumber;
        self->next = next;
        self->last = last;
        self->tail = tail;
        self->bufferLength = PIE_TRELLIS_BUFFER_INIT_SIZE;
        self->gamma = Pie_malloc(self->bufferLength
                        * branchNumber 
                        * parityType 
                        * sizeof *(self->gamma));
        self->alpha = Pie_malloc(self->bufferLength
                        * stateNumber
                        * sizeof *(self->alpha));
        self->beta = Pie_malloc(self->bufferLength
                        * stateNumber
                        * sizeof *(self->beta));
        self->maxStar = PieIniFile_getProfileBoolean(ini, 
                        section, "maxStar", PIE_FALSE);
#ifdef TEST_BENCH
        printf("trellis\n");
        printf("state system parity state+1 system parity state-1\n");
        for (int i = 0; i < stateNumber; i++) {
                for (int j = 0; j < branchNumber; j++) {
                        printf("%d  %d  %d  %d  %d  %d  %d\n", i, 
                                        next[i][j].system,
                                        next[i][j].parity,
                                        next[i][j].toState,
                                        last[i][j].system,
                                        last[i][j].parity,
                                        last[i][j].fromState);
                }
        }
        printf("tail\n");
        for (int i = 0; i < stateNumber; i++)
                printf("%d  ", tail[i]);
        printf("\n");
#endif

        return PIE_TRUE;
}

void PieTrellis_destroy(PieTrellis *self)
{
        for (int i = 0; i < self->stateNumber; i++) {
                Pie_free(self->next[i]);
                Pie_free(self->last[i]);
        }
        Pie_free(self->next);
        Pie_free(self->last);
        Pie_free(self->tail);

        Pie_free(self->gamma);
        Pie_free(self->alpha);
        Pie_free(self->beta);
}

PieBoolean PieTrellis_encode(PieTrellis *self,
                             // input
                             int inputNumber,
                             PieUint32 *input,
                             PieUint32 startState,
                             int transferNumber,
                             int tailNumber,
                             // output
                             PieUint32 *endState,
                             // va output
                             // system1, system2, ... 
                             // parity1, parity2, ...
                             ...)
{
        if (inputNumber < 0 || tailNumber < 0 || transferNumber < 0)
                return PIE_FALSE;

        int stateNumber = self->systemNumber;
        int systemNumber = self->systemNumber;
        int parityNumber = self->parityNumber;
        if (tailNumber == PIE_TRELLIS_TAIL)
                tailNumber = self->tailNumber;
        int netTransferNumber = transferNumber - tailNumber;

        if (startState >= stateNumber)
                return PIE_FALSE;
        if (inputNumber != netTransferNumber * systemNumber)
                return PIE_FALSE;

        // va
        PieUint32 *system[10], *parity[10];
        va_list ap;
        va_start(ap, endState);
        for (int i = 0; i < systemNumber; i++)
                system[i] = va_arg(ap, PieUint32 *);
        for (int i = 0; i < parityNumber; i++)
                parity[i] = va_arg(ap, PieUint32 *);
        va_end(ap);

        PieTrellisTransfer **next = self->next;
        PieUint32 *curInput = input;
        PieUint32 state = startState;

        for (int i = 0; i < netTransferNumber; i++) {
                PieUint32 in = 0;
                for (int j = 0; j < systemNumber; j++) {
                        system[j][i] = *(curInput) & 1;
                        curInput++;
                        in <<= 1;
                        in += system[j][i];
                }
                PieTrellisTransfer *transfer = &next[state][in];
                PieUint32 out = transfer->parity;
                for (int j = parityNumber - 1; j >= 0; j--) {
                        parity[j][i] = out & 1;
                        out >>= 1;
                }
                state = transfer->toState;
        }

        if (tailNumber == PIE_TRELLIS_NO_TAIL) {
                if (endState)
                        *endState = state;
                return PIE_TRUE;
        }

        PieUint32 *tail = self->tail;
        for (int i = netTransferNumber; i < transferNumber; i++) {
                PieUint32 in = tail[state];
                PieTrellisTransfer *transfer = &next[state][in];
                PieUint32 out = transfer->parity;
                for (int j = systemNumber - 1; j >= 0; j--) {
                        system[j][i] = in & 1;
                        in >>= 1;
                }
                for (int j = parityNumber - 1; j >= 0; j--) {
                        parity[j][i] = out & 1;
                        out >>= 1;
                }
                state = transfer->toState;
        }

        if (endState)
                *endState = state;
        return PIE_TRUE;
}

PieBoolean PieTrellis_upperDecode(PieTrellis *self,
                                  // input
                                  int transferNumber,
                                  int tailNumber,
                                  // IO
                                  double *mAlpha,
                                  double *mBeta,
                                  // va input
                                  // system1, system2, ... 
                                  // parity1, parity2, ... 
                                  // ext1, ext2, ...
                                  // va output
                                  // systemLLR1, systemLLR2, ...
                                  // parityLLR1, parityLLR2, ...
                                 ...)
{
        int systemNumber = self->systemNumber;
        int parityNumber = self->parityNumber;
        int stateNumber = self->stateNumber;
        int systemType = 1 << systemNumber;
        int parityType = 1 << parityNumber;

        if (transferNumber < 0 || tailNumber < 0)
                return PIE_FALSE;

        // va
        double *system[10], *parity[10], *ext[10];
        va_list ap;
        va_start(ap, mBeta);
        for (int i = 0; i < systemNumber; i++)
                system[i] = va_arg(ap, double *);
        for (int i = 0; i < parityNumber; i++)
                parity[i] = va_arg(ap, double *);
        for (int i = 0; i < systemNumber; i++)
                ext[i] = va_arg(ap, double *);

        double *systemLLR[10], *parityLLR[10];
        for (int i = 0; i < systemNumber; i++)
                systemLLR[i] = va_arg(ap, double *);
        for (int i = 0; i < parityNumber; i++)
                parityLLR[i] = va_arg(ap, double *);
        va_end(ap);

#ifdef PIE_TRELLIS_OVERFLOW_LIMIT
        for (int j = 0; j < transferNumber; j++) {
                for (int i = 0; i < systemNumber; i++) {
                        if (system[i][j] > PIE_INFI_0)
                                system[i][j] = PIE_INFI_0;
                        else if (system[i][j] < -PIE_INFI_0)
                                system[i][j] = -PIE_INFI_0;
                        if (ext[i][j] > PIE_MAXE)
                                ext[i][j] = PIE_MAXE;
                        else if (ext[i][j] < -PIE_MAXE)
                                ext[i][j] = -PIE_MAXE;
                }
                for (int i = 0; i < parityNumber; i++) {
                        if (parity[i][j] > PIE_INFI_0)
                                parity[i][j] = PIE_INFI_0;
                        else if (parity[i][j] < -PIE_INFI_0)
                                parity[i][j] = -PIE_INFI_0;
                }
        }
#endif
        // init buffer
        if (self->bufferLength < transferNumber) {
                self->gamma = Pie_realloc(self->gamma, 
                                transferNumber 
                                * systemType 
                                * parityType 
                                * sizeof *(self->gamma));
                self->alpha = Pie_realloc(self->alpha,
                                transferNumber
                                * stateNumber
                                * sizeof *(self->alpha));
                self->beta = Pie_realloc(self->beta,
                                transferNumber
                                * stateNumber
                                * sizeof *(self->beta));
                self->bufferLength = transferNumber;
        }

        double *gamma = self->gamma;
        int codeType = systemType * parityType;
        for (int i = 0; i < transferNumber; i++) {
                double tmp1[10], tmp2[10], tmp3[10];
                for (int k = 0; k < parityNumber; k++)
                        tmp1[k] = log(1 + exp(parity[k][i]));
                for (int k = 0; k < systemNumber; k++) {
                        tmp2[k] = log(1 + exp(system[k][i]));
                        tmp3[k] = 0.5 * ext[k][i];
                }
                for (int j = 0; j < codeType; j++) {
                        double g = 0;
                        PieUint32 code = j;
                        for (int k = parityNumber - 1; k >= 0; k--) {
                                g += !(code & 1) * parity[k][i] - tmp1[k];
                                code >>= 1;
                        }
                        for (int k = systemNumber - 1; k >= 0; k--) {
                                g += !(code & 1) * system[k][i] - tmp2[k]
                                        + (1 - 2 * (code & 1)) * tmp3[k];
                                code >>= 1;
                        }
#ifdef PIE_TRELLIS_OVERFLOW_LIMIT
                        if (g < 0 && g > -PIE_TINY)
                                g = -PIE_TINY;
                        else if (g > 0 && g < PIE_TINY)
                                g = PIE_TINY;
                        else if (g < -PIE_INFI_1)
                                g = -PIE_INFI_1;
                        else if (g > PIE_INFI_1)
                                g = PIE_INFI_1;
#endif
                        gamma[j + i * codeType] = g;
                }
        }

        return PieTrellis_lowerDecode(self, 
                        transferNumber, tailNumber, gamma,
                        systemLLR, parityLLR, mAlpha, mBeta);
}

PieBoolean PieTrellis_lowerDecode(PieTrellis *self,
                                  // input,
                                  int transferNumber,
                                  int tailNumber,
                                  double *gamma,
                                  // output
                                  double **systemLLR,
                                  double **parityLLR,
                                  // IO
                                  double *mAlpha,
                                  double *mBeta)
{
        int stateNumber = self->stateNumber;
        int systemNumber = self->systemNumber;
        int parityNumber = self->parityNumber;
        int systemType = 1 << systemNumber;
        int parityType = 1 << parityNumber;
        int codeType = systemType * parityType;
        double *alpha = self->alpha;
        double *beta = self->beta;

        PieTrellisTransfer **next = self->next;
        PieTrellisTransfer **last = self->last;

        double (*maxFunc)(double a, double b) = self->maxStar ? 
                        PieSim_maxStar : PieSim_max;

        // init alpha
        if (mAlpha) {
                for (int i = 0; i < stateNumber; i++)
                        alpha[i] = mAlpha[i];
        } else {
                alpha[0] = 0;
                for (int i = 1; i < stateNumber; i++)
                        alpha[i] = -PIE_INFI_1;
        }

        double *curAlpha = alpha;
        double *curGamma = gamma;
        for (int i = 1; i < transferNumber; i++) {
                double max = -PIE_INFI_1;
                for (int j = 0; j < stateNumber; j++) {
                        PieTrellisTransfer *tt = &last[j][0];
                        PieUint32 ti = (tt->system << parityNumber) 
                                        + tt->parity;
                        double tmp1 = curAlpha[tt->fromState] + curGamma[ti];
                        double tmp2 = 0;
                        for (int k = 1; k < systemType; k++) {
                                tt = &last[j][k];
                                ti = (tt->system << parityNumber) + tt->parity;
                                tmp2 = curAlpha[tt->fromState] + curGamma[ti];
                                tmp1 = maxFunc(tmp1, tmp2);
                        }
                        curAlpha[stateNumber + j] = tmp1;
                        if (tmp1 > max)
                                max = tmp1;
                }
                curAlpha += stateNumber;
                curGamma += codeType;
                for (int j = 0; j < stateNumber; j++) {
                        curAlpha[j] -= max;
#ifdef PIE_TRELLIS_OVERFLOW_LIMIT
                        if (curAlpha[j] > PIE_INFI)
                                curAlpha[j] = PIE_INFI;
                        else if (curAlpha[j] < -PIE_INFI)
                                curAlpha[j] = -PIE_INFI;
#endif
                }
        }

        if (mAlpha) {
                for (int i = 0; i < stateNumber; i++)
                        mAlpha[i] = curAlpha[i];
        }

        // init beta
        int index = (transferNumber - 1) * stateNumber;
        if (mBeta) {
                for (int i = 0; i < stateNumber; i++)
                        beta[index + i] = mBeta[i];
        } else if (tailNumber) {
                beta[index] = 0;
                for (int i = 1; i < stateNumber; i++)
                        beta[index + i] = -PIE_INFI_1;
        } else {
                double tmpBeta = -log(stateNumber);
                for (int i = 0; i < stateNumber; i++)
                        beta[index + i] = tmpBeta;
        }

        double *curBeta = beta + (transferNumber - 1) * stateNumber;
        curGamma = gamma + (transferNumber - 1) * codeType;
        while (curBeta != beta) {
                double max = -PIE_INFI_1;
                for (int j = 0; j < stateNumber; j++) {
                        PieTrellisTransfer *tt = &next[j][0];
                        PieUint32 ti = (tt->system << parityNumber) 
                                        + tt->parity;
                        double tmp1 = curBeta[tt->toState] + curGamma[ti];
                        double tmp2 = 0;
                        for (int k = 1; k < systemType; k++) {
                                tt = &next[j][k];
                                ti = (tt->system << parityNumber) + tt->parity;
                                tmp2 = curBeta[tt->toState] + curGamma[ti];
                                tmp1 = maxFunc(tmp1, tmp2);
                        }
                        *(curBeta - stateNumber + j) = tmp1;
                        if (max < tmp1)
                                max = tmp1;
                }
                curBeta -= stateNumber;
                curGamma -= codeType;
                for (int j = 0; j < stateNumber; j++) {
                        curBeta[j] -= max;
#ifdef PIE_TRELLIS_OVERFLOW_LIMIT
                        if (curBeta[j] > PIE_INFI)
                                curBeta[j] = PIE_INFI;
                        else if (curBeta[j] < -PIE_INFI)
                                curBeta[j] = -PIE_INFI;
#endif
                }
        }

        if (mBeta) {
                for (int i = 0; i < stateNumber; i++)
                        mBeta[i] = curBeta[i];
        }

        // LLR
        curAlpha = alpha;
        curBeta = beta;
        curGamma = gamma;
        double sysMeasure[100];
        double ptyMeasure[100];
        double sys[10];
        double pty[10];
        for (int i = 0; i < transferNumber; i++) {
                for (int k = 0; k < parityType; k++)
                        ptyMeasure[k] = -PIE_INFI;
                for (int k = 0; k < systemType; k++) {
                        PieTrellisTransfer *tt = &next[0][k];
                        PieUint32 ti = (tt->system << parityNumber) 
                                        + tt->parity;
                        sysMeasure[k] = curAlpha[0] 
                                        + curBeta[tt->toState] 
                                        + curGamma[ti];
                        ptyMeasure[tt->parity] = maxFunc(sysMeasure[k], 
                                        ptyMeasure[tt->parity]);
                }
                for (int j = 1; j < stateNumber; j++) {
                        for (int k = 0; k < systemType; k++) {
                                PieTrellisTransfer *tt = &next[j][k];
                                PieUint32 ti = (tt->system << parityNumber) 
                                                + tt->parity;
                                double tmp2 = curAlpha[j] 
                                                + curBeta[tt->toState] 
                                                + curGamma[ti];
                                sysMeasure[k] = maxFunc(
                                                sysMeasure[k], tmp2);
                                PieUint32 nc = tt->parity;
                                ptyMeasure[nc] = maxFunc(
                                                ptyMeasure[nc], tmp2);
#ifdef PIE_TRELLIS_OVERFLOW_LIMIT
                                if (sysMeasure[k] > PIE_INFI)
                                        sysMeasure[k] = PIE_INFI;
                                else if (sysMeasure[k] < -PIE_INFI)
                                        sysMeasure[k] = -PIE_INFI;
                                if (ptyMeasure[nc] > PIE_INFI)
                                        ptyMeasure[nc] = PIE_INFI;
                                else if (ptyMeasure[nc] < -PIE_INFI)
                                        ptyMeasure[nc] = -PIE_INFI;
#endif
                        }
                }
                PieSim_bitDemap(systemNumber, sysMeasure, self->maxStar, sys);
                for (int k = 0; k < systemNumber; k++)
                        systemLLR[k][i] = sys[k];
                PieSim_bitDemap(parityNumber, ptyMeasure, self->maxStar, pty);
                for (int k = 0; k < parityNumber; k++)
                        parityLLR[k][i] = pty[k];

                curAlpha += stateNumber;
                curBeta += stateNumber;
                curGamma += codeType;
        }

        return PIE_TRUE;
}

double PieTrellis_getCodeRate(PieTrellis *self, PieBoolean system)
{
        if (!system)
                return (double)(self->systemNumber / self->parityNumber);
        return (double)(self->systemNumber 
                        / (self->systemNumber + self->parityNumber));
}

int PieTrellis_getTransferNumber(PieTrellis *self, 
                                 int inputNumber, 
                                 PieBoolean tail)
{
        if (inputNumber < 0)
                return -1;
        int a = inputNumber / self->systemNumber;
        if (tail)
                a += self->tailNumber;
        return a;
}

// TestBench example

/* #include <stdio.h> */
/* #include <stdlib.h> */
/* #include <string.h> */

/* #include "plus/pie_ini_file.h" */
/* #include "simulation/pie_sim_misc.h" */
/* #include "simulation/pie_trellis.h" */

/* int main(int argc, char *argv[]) */
/* { */
        /* PieIniFile ini; */
        /* PieIniFile_init(&ini, "resource/test.ini"); */
        /* PieTrellis t; */
        /* PieTrellis_init(&t, &ini, 0); */
        /* PieIniFile_destroy(&ini); */
        /* PieUint32 a[32]; */
        /* PieSim_randomVectorInt(32, a); */
        /* printf("in\n"); */
        /* for (int i = 0; i < 32; i++) { */
                /* printf("%d ", a[i]); */
        /* } */
        /* printf("\n"); */
        /* PieUint32 b[35], c[35]; */
        /* PieUint32 end; */
        /* PieTrellis_encode(&t, 32, a, 0, 35, 3, &end, b, c); */
        /* printf("out\n"); */
        /* for (int i = 0; i < 35; i++) { */
                /* printf("%d ", b[i]); */
        /* } */
        /* printf("s%d\n", end);    */
        /* printf("out\n"); */
        /* for (int i = 0; i < 35; i++) { */
                /* printf("%d ", c[i]); */
        /* } */
        /* printf("\n"); */

        /* double d[35], e[35], f[35]; */
        /* for (int i = 0; i < 35; i++) { */
                /* d[i] = -5.0 * (2 * (int)b[i] - 1); */
                /* e[i] = -5.0 * (2 * (int)c[i] - 1); */
                /* f[i] = 0; */
        /* } */

        /* double g[35], h[35]; */
        /* PieTrellis_upperDecode(&t, 35, 3, 0, 0, d, e, f, g, h); */
        /* printf("decode\n"); */
        /* for (int i = 0; i < 35; i++) { */
                /* printf("%f ", g[i]); */
        /* } */
        /* printf("\n"); */
        /* printf("decode\n"); */
        /* for (int i = 0; i < 35; i++) { */
                /* printf("%f ", h[i]); */
        /* } */
        /* printf("\n"); */
        /* printf("decode\n"); */
        /* for (int i = 0; i < 35; i++) { */
                /* int a = g[i] > 0 ? 0 : 1; */
                /* printf("%d ", a); */
        /* } */
        /* printf("\n"); */
        /* printf("decode\n"); */
        /* for (int i = 0; i < 35; i++) { */
                /* int a = h[i] > 0 ? 0 : 1; */
                /* printf("%d ", a); */
        /* } */
        /* printf("\n"); */

        /* PieTrellis_destroy(&t); */
        /* return 0; */
/* } */
