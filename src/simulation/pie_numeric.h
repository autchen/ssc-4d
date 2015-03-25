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
 * @file pie_numeric.h
 * @brief Numeric types for baseband simulation
 * @author Qiuwen Chen
 * @version 0.1
 * @date 2011/6/19 21:02:15
 */

#ifndef pie_numeric_h
#define pie_numeric_h

// Global header
#include "plus/pie_stdinc.h"

typedef struct PieComplex {
        double real;
        double imag;
} PieComplex;

#endif
