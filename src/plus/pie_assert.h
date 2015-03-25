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
 * @file pie_assert.h
 * @brief Assert macros with different debug levels
 * @author Qiuwen Chen
 * @version 0.1
 * @date 2011/6/13 20:36:47
 */

#ifndef pie_assert_h
#define pie_assert_h

// Global header
#include "plus/pie_stdinc.h"

// Lib header
#include "assert.h"

#ifdef PIE_DEBUG_LEVEL_HIGH
        #define PIE_ASSERT_HIGH(P) assert(P)
        #define PIE_ASSERT_MEDIUM(P) assert(P)
        #define PIE_ASSERT_LOW(P) assert(P)
#endif

#ifdef PIE_DEBUG_LEVEL_MEDIUM
        #define PIE_ASSERT_HIGH(P) assert(P)
        #define PIE_ASSERT_MEDIUM(P) assert(P)
        #define PIE_ASSERT_LOW(P) 0
#endif

#ifdef PIE_DEBUG_LEVEL_LOW
        #define PIE_ASSERT_HIGH(P) assert(P)
        #define PIE_ASSERT_MEDIUM(P) 0
        #define PIE_ASSERT_LOW(P) 0
#endif

#endif
