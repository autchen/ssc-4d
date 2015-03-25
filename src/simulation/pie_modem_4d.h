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
 * @file pie_modem_4d.h
 * @brief SSD modulation with diversity 4
 * @author Qiuwen Chen
 * @version 0.1
 * @date 2011/6/21 9:36:03
 */

#ifndef pie_modem_4d_h
#define pie_modem_4d_h

// Global header
#include "plus/pie_stdinc.h"
#include "simulation/pie_numeric.h"

// Constants
#define PIE_MODEM_4D_MAX_POINT 256
#define PIE_MODEM_4D_A 0x00000001
#define PIE_MODEM_4D_B 0x00000002
#define PIE_MODEM_4D_C 0x00000004
#define PIE_MODEM_4D_D 0x00000008
#define PIE_MODEM_4D_ALL 0x0000000F
#define PIE_MODEM_4D_SNR 0

// External structs
struct PieIniFile;
struct PieComplex2;

// Structs declaration
typedef struct PieModem4D {
        int order;
        double sita1;
        double sita2;
        PieBoolean maxStar;
        PieComplex oMap[PIE_MODEM_4D_MAX_POINT];
        struct PieComplex2 *rMap;
        PieUint32 componentFlag;
        
        // buffers
        double *distance;
} PieModem4D;

// Public methods

PieBoolean PieModem4D_init(PieModem4D *self, 
                           struct PieIniFile *ini,
                           char *section);

void PieModem4D_destroy(PieModem4D *self);

PieBoolean PieModem4D_modulate(PieModem4D *self,
                               // input
                               int inputNumber,
                               PieUint32 *input,
                               int outputNumber,
                               // output
                               PieComplex *output);

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
                                 double *output);

void PieModem4D_enableComponents(PieModem4D *self, PieUint32 flag);

#endif
