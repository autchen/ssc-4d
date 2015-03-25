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
 * @file pie_alloc.h
 * @brief Standard memory allocation interfaces
 * @author Qiuwen Chen
 * @version 0.1
 * @date 2011/6/13 20:46:31
 */

#ifndef pie_alloc_h
#define pie_alloc_h

// Global header
#include "plus/pie_stdinc.h"

// Lib headers
#include "stdlib.h"
#include "assert.h"

// Alloc interfaces

static inline void * Pie_malloc(PieSize size)
{
        void *ptr = malloc(size);
        assert(ptr);
        return ptr;
}

static inline void * Pie_calloc(PieSize number, PieSize size)
{
        void *ptr = calloc(number, size);
        assert(ptr);
        return ptr;
}

static inline void * Pie_realloc(void *oldPtr, PieSize size)
{
        void *ptr = realloc(oldPtr, size);
        assert(ptr);
        return ptr;
}

#define Pie_free(P) free(P)

#endif
