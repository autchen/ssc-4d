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
 * @file pie_channel.h
 * @brief Channel simulation and related functions
 * @author Qiuwen Chen
 * @version 0.1
 * @date 2011/6/21 20:12:59
 */

#ifndef pie_channel_h
#define pie_channel_h

// Global header
#include "plus/pie_stdinc.h"
#include "simulation/pie_numeric.h"

#define PIE_CHANNEL_SNR_MODE 0 

// Public methods

PieBoolean PieChannel_AWGN(int inputNumber,
                           PieComplex *input, // IO
                           double EbN0,
                           double rate,
                           int moduOrder);

PieBoolean PieChannel_fading(// input
                             int inputNumber,
                             PieComplex *input, // IO
                             int moduOrder,
                             int blockSize,
                             // output
                             PieComplex *equOut);

PieBoolean PieChannel_iFading(// input
                              int inputNumber,
                              PieComplex *input, // IO
                              PieComplex *fading, // IO
                              int moduOrder,
                              // output
                              PieComplex *equOut);

#endif
