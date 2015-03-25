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

// Primary header
#include "pie_slice_pool.h"

// Project headers
#include "plus/pie_alloc.h"

// Macros constants
#define PIE_SLICE_POOL_BUNDLE_BASE 8

// Internal struct
typedef struct PieSliceBundle {
        int sliceNumber;
        struct PieSliceBundle *next;
        char data[0];
} PieSliceBundle;

// Method implementations

/**
 * @brief Ctor
 *
 * @param self
 * @param sliceSize - size of single slice in char
 *
 * @return op
 */
PieBoolean PieSlicePool_init(PieSlicePool *self, PieSize sliceSize)
{
        sliceSize += sliceSize % sizeof(void *);
        self->sliceSize = sliceSize;
        self->freeSlice = PIE_NULL;
        self->bundle = PIE_NULL;
        PieSlicePool_expand(self);
        return PIE_TRUE;
}

void PieSlicePool_destroy(PieSlicePool *self)
{
        PieSliceBundle *bundle = self->bundle;
        while (bundle) {
                PieSliceBundle *tmp = bundle;
                bundle = bundle->next;
                Pie_free(tmp);
        }
}

/**
 * @brief Expand the memory pool size to support more slices
 *
 * @param self
 */
void PieSlicePool_expand(PieSlicePool *self)
{
        PieSliceBundle *bundle = self->bundle;
        int sliceNumber = bundle ? 
                        (bundle->sliceNumber * 2) : 
                        PIE_SLICE_POOL_BUNDLE_BASE;
        PieSize sliceSize = self->sliceSize;
        PieSize bundleSize = sliceNumber * sliceSize;
        
        bundle = Pie_malloc(bundleSize + sizeof *bundle);
        bundle->sliceNumber = sliceNumber;
        bundle->next = self->bundle;
        self->bundle = bundle;

        PieSliceUnit *unit = (PieSliceUnit *)bundle->data;
        PieSliceUnit *end = (PieSliceUnit *)
                        (bundle->data + bundleSize - sliceSize);
        while (unit != end) {
                unit->next = (PieSliceUnit *)(unit->data + sliceSize);
                unit = unit->next;
        }
        unit->next = self->freeSlice;
        self->freeSlice = (PieSliceUnit *)bundle->data;
}

// PieBoolean PieSlicePool_doesContain(PieSlicePool *self, void *ptr);

/**
 * @brief Count the total number of managed slices
 *
 * @param self
 *
 * @return number of slices
 */
int PieSlicePool_countTotalSlice(PieSlicePool *self)
{
        int count = 0;
        PieSliceBundle *bundle = self->bundle;
        while (bundle) {
                count += bundle->sliceNumber;
                bundle = bundle->next;
        }
        return count;
}

/**
 * @brief Count available slices
 *
 * @param self
 *
 * @return number of free slices in the pool
 */
int PieSlicePool_countFreeSlice(PieSlicePool *self)
{
        int count = 0;
        PieSliceUnit *slice = self->freeSlice;
        while (slice) {
                count++;
                slice = slice->next;
        }
        return count;
}

// TestBench example

/* #include <stdio.h> */
/* #include <stdlib.h> */
/* #include <string.h> */

/* #include "plus/pie_slice_pool.h" */

/* typedef struct ts { */
        /* int a; */
        /* int b; */
        /* int c; */
        /* int d; */
/* } ts; */

/* int main(int argc, char *argv[]) */
/* { */
        /* PieSlicePool p; */
        /* PieSlicePool_init(&p, sizeof(ts)); */
        /* ts *pts[20000]; */
        /* for (int i = 0; i < 10; i++) { */
                /* pts[i] = PieSlicePool_alloc(&p); */
                /* pts[i]->a = 1; */
                /* pts[i]->b = 2; */
                /* pts[i]->c = 3; */
                /* printf("%d / %d\n",  */
                       /* PieSlicePool_countFreeSlice(&p),  */
                       /* PieSlicePool_countTotalSlice(&p)); */
        /* } */
        /* for (int i = 0; i < 5; i++) { */
                /* PieSlicePool_free(&p, pts[i+2]); */
                /* printf("%d / %d\n",  */
                       /* PieSlicePool_countFreeSlice(&p),  */
                       /* PieSlicePool_countTotalSlice(&p)); */
        /* } */
        /* for (int i = 0; i < 100; i++) { */
                /* pts[i] = PieSlicePool_alloc(&p); */
                /* pts[i]->a = 1; */
                /* pts[i]->b = 2; */
                /* pts[i]->c = 3; */
                /* printf("%d / %d\n",  */
                       /* PieSlicePool_countFreeSlice(&p),  */
                       /* PieSlicePool_countTotalSlice(&p)); */
        /* } */
        /* for (int i = 0; i < 50; i++) { */
                /* PieSlicePool_free(&p, pts[i+2]); */
                /* printf("%d / %d\n",  */
                       /* PieSlicePool_countFreeSlice(&p),  */
                       /* PieSlicePool_countTotalSlice(&p)); */
        /* } */
        /* for (int i = 0; i < 1000; i++) { */
                /* pts[i] = PieSlicePool_alloc(&p); */
                /* pts[i]->a = 1; */
                /* pts[i]->b = 2; */
                /* pts[i]->c = 3; */
                /* printf("%d / %d\n",  */
                       /* PieSlicePool_countFreeSlice(&p),  */
                       /* PieSlicePool_countTotalSlice(&p)); */
        /* } */
        /* PieSlicePool_destroy(&p); */
        /* return 0; */
/* } */
