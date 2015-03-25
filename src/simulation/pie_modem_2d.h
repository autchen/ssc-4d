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

/**
 * @file pie_modem_2d.h
 * @brief modulator/demodulator of SSD order 2
 * @author Qiuwen Chen
 * @version 0.1
 * @date 2011/6/20 13:23:06
 */

#ifndef pie_modem_2d_h
#define pie_modem_2d_h

// Global header
#include "plus/pie_stdinc.h"
#include "simulation/pie_numeric.h"

// Constants
#define PIE_MODEM_2D_MAX_POINT 256
#define PIE_MODEM_2D_MODE_NORMAL 0
#define PIE_MODEM_2D_MODE_ONLY_I 1
#define PIE_MODEM_2D_MODE_ONLY_Q 2

// External struct
struct PieIniFile;

// Structs declaration
typedef struct PieModem2D {
        int order;
        double sita;
        PieBoolean maxStar;
        PieComplex oMap[PIE_MODEM_2D_MAX_POINT];
        PieComplex rMap[PIE_MODEM_2D_MAX_POINT];

        double (*distanceFunc)(PieComplex a, PieComplex b, double is2);
} PieModem2D;

// Public methods

PieBoolean PieModem2D_init(PieModem2D *self, 
                           struct PieIniFile *ini, 
                           char *section);

// void PieModem2D_destroy(PieModem2D *self);
#define PieModem2D_destroy(S) 0

PieBoolean PieModem2D_modulate(PieModem2D *self, 
                               // input
                               int inputNumber,
                               PieUint32 *input,
                               int outputNumber,
                               // output
                               PieComplex *output);

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
                                 double *output);

void PieModem2D_setMode(PieModem2D *self, int mode);

#endif
