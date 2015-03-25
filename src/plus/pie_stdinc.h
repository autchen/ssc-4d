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
 * @file pie_stdinc.h
 * @brief Global header of Pie, defining primitive types and config macros
 * @author Qiuwen Chen
 * @version 0.1
 * @date 2011/6/13 20:21:32
 */

#ifndef pie_stdinc_h
#define pie_stdinc_h

// Primitive types

typedef int PieBoolean;
#define PIE_FALSE 0
#define PIE_TRUE 1

// size of char = 1
typedef unsigned int PieSize;

typedef char PieSint8;
typedef unsigned char PieUint8;

typedef short PieSint16;
typedef unsigned short PieUint16;

typedef int PieSint32;
typedef unsigned int PieUint32;

typedef long long PieSint64;
typedef unsigned long long PieUint64;

// Null pointer
#define PIE_NULL (void *)0

// Configure

#define PIE_DEBUG_LEVEL_HIGH 1

#endif
