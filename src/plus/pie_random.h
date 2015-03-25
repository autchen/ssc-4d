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
 * @file pie_random.h
 * @brief Pseudo random number generator
 * @author Qiuwen Chen
 * @version 0.1
 * @date 2011/6/20 10:51:15
 */

#ifndef pie_random_h
#define pie_random_h

// Global header
#include "plus/pie_stdinc.h"

// Public methods

double Pie_random();

PieBoolean Pie_randomBoolean(double pTrue);

double Pie_randomInterval(double low, double high);

void Pie_randomSeed(PieUint64 seed);

#endif
