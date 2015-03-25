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
 * @file pie_ssd_interleaver.h
 * @brief SSD Interleaver
 * @author Qiuwen Chen
 * @version 0.1
 * @date 2011/6/23 18:24:10
 */

#ifndef pie_ssd_interleaver_h
#define pie_ssd_interleaver_h

// Global header
#include "plus/pie_stdinc.h"
#include "simulation/pie_numeric.h"

// Public methods

PieBoolean PieSSDInterleaver_splitQ(int inputNumber, 
                                    PieComplex *input, 
                                    PieComplex *output);

PieBoolean PieSSDInterleaver_split2(int inputNumber, 
                                    PieComplex *input,
                                    PieComplex *output);

PieBoolean PieSSDInterleaver_combineEqu(int inputNumber,
                                        PieComplex *equIn,
                                        PieComplex *equOut);

#define PieSSDInterleaver_combineQ PieSSDInterleaver_splitQ

PieBoolean PieSSDInterleaver_combine2(int inputNumber, 
                                      PieComplex *input,
                                      PieComplex *output);

#endif
