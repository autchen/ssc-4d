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
 * @file pie_sim_misc.h
 * @brief Simulation miscellaneous functions
 * @author Qiuwen Chen
 * @version 0.1
 * @date 2011/6/20 12:27:26
 */

#ifndef pie_sim_misc_h
#define pie_sim_misc_h

// Global header
#include "plus/pie_stdinc.h"
#include "simulation/pie_numeric.h"

// Public methods

PieBoolean PieSim_randomVectorInt(int size, PieUint32 *output);

int PieSim_compareVector(int size, PieUint32 *a, PieUint32 *b);

int PieSim_decideCompare(int size, PieUint32 *a, double *b);

double PieSim_maxStar(double var1, double var2);

double PieSim_max(double var1, double var2);

double PieSim_AWGN(double sigma2);

PieBoolean PieSim_bitDemap(// input 
                           int order,
                           double *prob, 
                           PieBoolean maxStar,
                           // output
                           double *output);

PieBoolean PieSim_bitDemapExt(// input
                              int order,
                              double *prob,
                              double *ext,
                              PieBoolean maxStar,
                              // output
                              double *output);

PieBoolean PieSim_copyVectorComplex(int number, 
                                    PieComplex *from, 
                                    PieComplex *to);

#endif
