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
 * @file pie_slice_pool.h
 * @brief Fixed sized and single-thread memory pool
 * @author Qiuwen Chen
 * @version 0.1
 * @date 2011/6/19 9:26:11
 */

#ifndef pie_slice_pool_h
#define pie_slice_pool_h

// Global header
#include "plus/pie_stdinc.h"

// External structs

struct PieSliceBundle;

typedef union PieSliceUnit {
        union PieSliceUnit *next;
        char data[0];
} PieSliceUnit;

// Structs declaration
typedef struct PieSlicePool {
        PieSize sliceSize;
        union PieSliceUnit *freeSlice;
        struct PieSliceBundle *bundle;
} PieSlicePool;

// Public methods

PieBoolean PieSlicePool_init(PieSlicePool *self, PieSize sliceSize);

void PieSlicePool_destroy(PieSlicePool *self);

void PieSlicePool_expand(PieSlicePool *self);

// PieBoolean PieSlicePool_doesContain(PieSlicePool *self, void *ptr);

int PieSlicePool_countTotalSlice(PieSlicePool *self);

int PieSlicePool_countFreeSlice(PieSlicePool *self);

// Inline functions

static inline void * PieSlicePool_alloc(PieSlicePool *self)
{
        if (!self->freeSlice)
                PieSlicePool_expand(self);
        PieSliceUnit *ptr = self->freeSlice;
        self->freeSlice = ptr->next;
        return ptr;
}

static inline void PieSlicePool_free(PieSlicePool *self, void *ptr)
{
        PieSliceUnit *unit = (PieSliceUnit *)ptr;
        unit->next = self->freeSlice;
        self->freeSlice = unit;
}

#endif
