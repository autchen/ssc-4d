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
 * @file pie_equalizer.h
 * @brief Channel equalizer
 * @author Qiuwen Chen
 * @version 0.1
 * @date 2011/6/21 21:39:00
 */

#ifndef pie_equalizer_h
#define pie_equalizer_h

// Global header
#include "plus/pie_stdinc.h"
#include "Simulation/pie_numeric.h"

// Public methods

PieBoolean PieEqualizer_equalize(// input
                                 int inputNumber,
                                 PieComplex *input,
                                 PieComplex *equIn,
                                 int moduOrder,
                                 int outputNumber,
                                 // output
                                 PieComplex *output);

#endif
