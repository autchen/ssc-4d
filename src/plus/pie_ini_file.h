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
 * @file pie_ini_file.h
 * @brief Entity providing access to .ini formatted files
 * NOTE: This entity is only for init the program and thus not optimized for
 * fast set/get of profile fields !
 * @author Qiuwen Chen
 * @version 0.1
 * @date 2011/6/18 16:56:29
 */

#ifndef pie_ini_file_h
#define pie_ini_file_h

// Global header
#include "plus/pie_stdinc.h"

// Macro constant
#define PIE_INI_UNDEFINED_SECTION "undefined"

// External structs
struct PieSlicePool;
struct PieIniSection;

// Structs declaration
typedef struct PieIniFile {
        char filePath[100];
        PieBoolean isChanged;
        struct PieIniSection *section;
        struct PieSlicePool *buffer;
} PieIniFile;

// Public methods

PieBoolean PieIniFile_init(PieIniFile *self, char *filePath);

void PieIniFile_destroy(PieIniFile *self);

PieBoolean PieIniFile_save(PieIniFile *self);

PieBoolean PieIniFile_saveAs(PieIniFile *self, char *filePath);

char * PieIniFile_getProfileString(PieIniFile *self,
                                   char *sectionName,
                                   char *key,
                                   int bufferSize,
                                   char *bufferAddr);

PieBoolean PieIniFile_getProfileBoolean(PieIniFile *self, 
                                        char *sectionName,
                                        char *key,
                                        PieBoolean defaultValue);

int PieIniFile_getProfileInt(PieIniFile *self, 
                             char *sectionName, 
                             char *key, 
                             int defaultValue);

int PieIniFile_getProfileVectorInt(PieIniFile *self,
                                   char *sectionName,
                                   char *key,
                                   int bufferSize,
                                   int *bufferAddr);

double PieIniFile_getProfileDouble(PieIniFile *self,
                                   char *sectionName,
                                   char *key,
                                   double defaultValue);

int PieIniFile_getProfileVectorDouble(PieIniFile *self,
                                      char *sectionName,
                                      char *key,
                                      int bufferSize,
                                      double *bufferAddr);

char * PieIniFile_setProfileString(PieIniFile *self,
                                   char *sectionName,
                                   char *key,
                                   int stringSize,
                                   char *string);

void PieIniFile_setProfileBoolean(PieIniFile *self, 
                                  char *sectionName,
                                  char *key,
                                  PieBoolean value);

void PieIniFile_setProfileInt(PieIniFile *self, 
                              char *sectionName, 
                              char *key, 
                              int value);

void PieIniFile_setProfileVectorInt(PieIniFile *self,
                                    char *sectionName,
                                    char *key,
                                    int vectorSize,
                                    int *vector);

void PieIniFile_setProfileDouble(PieIniFile *self,
                                 char *sectionName,
                                 char *key,
                                 double value);

void PieIniFile_setProfileVectorDouble(PieIniFile *self,
                                       char *sectionName,
                                       char *key,
                                       int vectorSize,
                                       double *vector);

void PieIniFile_delProfileSection(PieIniFile *self, char *sectionName);

void PieIniFile_delProfile(PieIniFile *self, char *sectionName, char *key);

#endif
