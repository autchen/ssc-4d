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
 * @file pie_cstring.h
 * @brief Extension for c string
 * @author Qiuwen Chen
 * @version 0.1
 * @date 2011/6/19 14:08:16
 */

#ifndef pie_cstring_h
#define pie_cstring_h

// Global header
#include "plus/pie_stdinc.h"

// Lib header
#include "string.h"

void PieCString_trimLeft(char *self);

void PieCString_trimRight(char *self);

void PieCString_trim(char *self);

#endif
