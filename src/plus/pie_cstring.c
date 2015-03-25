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
#include "pie_cstring.h"

// Method implementations

void PieCString_trimLeft(char *self)
{
        int p = 0, k = 0;
        if (*self == '\0')
                return;
        while (self[p] == ' ' || self[p] == '\t' || self[p] == '\n')
                p++;
        if (p == 0)
                return;
        while (self[k] != '\0')
                self[k++] = self[p++];
}

void PieCString_trimRight(char *self)
{
        int l = 0, p = 0;
        l = strlen(self);
        if (l == 0)
                return;
        p = l - 1;
        while (self[p] == ' ' || self[p] == '\t' || self[p] == '\n') {
                self[p--] = '\0';
                if (p < 0)
                        break;
        }
}

void PieCString_trim(char *self)
{
        PieCString_trimLeft(self);
        PieCString_trimRight(self);
}

