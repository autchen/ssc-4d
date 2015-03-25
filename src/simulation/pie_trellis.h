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
 * @file pie_trellis.h
 * @brief Treliis encode/decode module
 * @author Qiuwen Chen
 * @version 0.1
 * @date 2011/6/22 10:01:30
 */

#ifndef pie_trellis_h
#define pie_trellis_h

// Global header
#include "plus/pie_stdinc.h"

// Constants
#define PIE_TRELLIS_PARITY 0x00000001
#define PIE_TRELLIS_SYSTEM 0x00000002
#define PIE_TRELLIS_EXTRIN 0x00000004
#define PIE_TRELLIS_ALL 0x00000007

#define PIE_TRELLIS_NO_TAIL 0
#define PIE_TRELLIS_TAIL 0x0FFFFFFF

// External structs
struct PieIniFile;
struct PieTrellisTransfer;

// Structs declaration
typedef struct PieTrellis {
        int stateNumber;
        int systemNumber;
        int parityNumber;
        struct PieTrellisTransfer **next;
        struct PieTrellisTransfer **last;
        PieUint32 *tail;
        int tailNumber;
        PieBoolean maxStar;

        // operation buffer
        int bufferLength;
        double *gamma;
        double *alpha;
        double *beta;
} PieTrellis;

// Public methods

PieBoolean PieTrellis_init(PieTrellis *self, 
                           struct PieIniFile *ini, 
                           char *section);

void PieTrellis_destroy(PieTrellis *self);

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
                             ...);

#define PieTrellis_decodeBCJR PieTrellis_upperDecode

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
                                 ...);

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
                                  double *mBeta);

double PieTrellis_getCodeRate(PieTrellis *self, PieBoolean system);
int PieTrellis_getTransferNumber(PieTrellis *self, 
                                 int inputNumber, 
                                 PieBoolean tail);

#endif
