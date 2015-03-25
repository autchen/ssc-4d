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
#include "pie_ini_file.h"

// Project headers
#include "plus/pie_assert.h"
#include "plus/pie_alloc.h"
#include "plus/pie_slice_pool.h"
#include "plus/pie_cstring.h"

// Lib headers
#include "stdio.h"

// Macros
#define PIE_INI_FILE_BUF_SIZE 100

// Internal structs

typedef struct PieIniPair {
        struct PieIniPair *next;
        char *value;
        char key[PIE_INI_FILE_BUF_SIZE];
} PieIniPair;

typedef struct PieIniSection {
        struct PieIniSection *next;
        struct PieIniPair *pair;
        char sectionName[PIE_INI_FILE_BUF_SIZE];
} PieIniSection;
// sizeof(PieIniPair) == sizeof(PieIniSection) for easier memory management

// Method implementations

/**
 * @brief Create the entity and buffer the ini file
 *
 * @param self
 * @param filePath
 *
 * @return false if ini file does not exist
 */
PieBoolean PieIniFile_init(PieIniFile *self, char *filePath)
{
        strcpy(self->filePath, filePath);
        self->isChanged = PIE_FALSE;
        self->buffer = Pie_malloc(sizeof *self->buffer);
        PieBoolean isPoolInitOK = PieSlicePool_init(
                        self->buffer, sizeof(PieIniPair));

        PIE_ASSERT_LOW(sizeof(PieIniSection) == sizeof(PieIniPair));
        PIE_ASSERT_HIGH(isPoolInitOK);

        FILE *fp = fopen(filePath, "r");
        if (!fp)
                return PIE_FALSE;
        char buf[PIE_INI_FILE_BUF_SIZE];

        PieIniSection *curSection = (PieIniSection *)
                        PieSlicePool_alloc(self->buffer);
        strcpy(curSection->sectionName, PIE_INI_UNDEFINED_SECTION);
        curSection->pair = PIE_NULL;
        curSection->next = PIE_NULL;
        self->section = curSection;
        PieIniPair *curPair = PIE_NULL;

        while (fgets(buf, PIE_INI_FILE_BUF_SIZE, fp) != PIE_NULL) {
                char *c1 = PIE_NULL, *c2 = PIE_NULL;
                // Comment
                if ((c1 = strchr(buf, ';')) != PIE_NULL)
                        *c1 = '\0';
                // Section
                if ((c1 = strchr(buf, '[')) != PIE_NULL
                                && (c2 = strchr(buf, ']')) != PIE_NULL
                                && c2 > c1) {
                        *c2 = '\0';
                        PieIniSection *section = (PieIniSection *)
                                        PieSlicePool_alloc(self->buffer);
                        PieCString_trim(c1 + 1);
                        strcpy(section->sectionName, c1 + 1);
                        section->pair = PIE_NULL;
                        section->next = PIE_NULL;
                        curSection->next = section;
                        curSection = section;
                        curPair = PIE_NULL;
                // Value pair
                } else {
                        PieCString_trim(buf);
                        int l = strlen(buf);
                        if (l == 0)
                                continue;
                        PieIniPair *pair = (PieIniPair *)
                                PieSlicePool_alloc(self->buffer);
                        strcpy(pair->key, buf);
                        if ((c1 = strchr(pair->key, '=')) != PIE_NULL)
                                *c1 = '\0';
                        else {
                                c1 = pair->key + l;
                                *(c1 + 1) = '1';
                                *(c2 + 2) = '\0';
                        }
                        pair->value = c1 + 1;
                        PieCString_trimRight(pair->key);
                        PieCString_trimLeft(pair->value);
                        pair->next = PIE_NULL;
                        if (curPair)
                                curPair->next = pair;
                        else
                                curSection->pair = pair;
                        curPair = pair;
                }
        }
        fclose(fp);

        return PIE_TRUE;
}

void PieIniFile_destroy(PieIniFile *self)
{
        PieSlicePool_destroy(self->buffer);
}

PieBoolean PieIniFile_save(PieIniFile *self)
{
        if (!self->isChanged)
                return PIE_TRUE;
        return PieIniFile_saveAs(self, self->filePath);
}

/**
 * @brief Save ini file to indicated path
 *
 * @param self
 * @param filePath
 *
 * @return op
 */
PieBoolean PieIniFile_saveAs(PieIniFile *self, char *filePath)
{
        FILE *fp = fopen(filePath, "w");
        if (!fp)
                return PIE_FALSE;

        PieIniSection *curSection = self->section;
        while (curSection) {
                PieIniPair *curPair = curSection->pair;
                if (!curPair) {
                        curSection = curSection->next;
                        continue;
                }
                fprintf(fp, "[%s]\n", curSection->sectionName);
                curSection = curSection->next;
                while (curPair) {
                        fprintf(fp, "%s=%s\n", curPair->key, curPair->value);
                        curPair = curPair->next;
                }
        }
        fclose(fp);
        self->isChanged = PIE_FALSE;

        return PIE_TRUE;
}

/**
 * @brief Get value as string. For conflicted section names or key, the value
 * appears ealier in the file will be returned.
 *
 * @param self
 * @param sectionName
 * @param key
 * @param bufferSize - size of output buffer in char
 * @param bufferAddr - output buffer address
 *
 * @return null if not found
 */
char * PieIniFile_getProfileString(PieIniFile *self,
                                  char *sectionName,
                                  char *key,
                                  int bufferSize,
                                  char *bufferAddr)
{
        PieIniSection *curSection = self->section;
        while (curSection) {
                if (strcmp(curSection->sectionName, sectionName) != 0) {
                        curSection = curSection->next;
                        continue;
                }
                PieIniPair *curPair = curSection->pair;
                while (curPair) {
                        if (strcmp(curPair->key, key) != 0) {
                                curPair = curPair->next;
                                continue;
                        }
                        return strncpy(bufferAddr, 
                                       curPair->value, 
                                       bufferSize);
                }
                curSection = curSection->next;
        }
        return PIE_NULL;
}

/**
 * @brief Return profile as boolean type
 *
 * @param self
 * @param sectionName
 * @param key
 * @param defaultValue - default return if entry not found
 *
 * @return 
 */
PieBoolean PieIniFile_getProfileBoolean(PieIniFile *self, 
                                        char *sectionName,
                                        char *key,
                                        PieBoolean defaultValue)
{
        char buf[PIE_INI_FILE_BUF_SIZE];
        char *stringValue = PieIniFile_getProfileString(self, 
                        sectionName, key, PIE_INI_FILE_BUF_SIZE, buf);
        if (stringValue) {
                if (strcmp(stringValue, "true") == 0)
                        return PIE_TRUE;
                else if (strcmp(stringValue, "false") == 0)
                        return PIE_FALSE;
        }
        return defaultValue;
}

/**
 * @brief Return entry as integer
 *
 * @param self
 * @param sectionName
 * @param key
 * @param defaultValue - default return when entry not found
 *
 * @return 
 */
int PieIniFile_getProfileInt(PieIniFile *self, 
                             char *sectionName, 
                             char *key, 
                             int defaultValue)
{
        char buf[PIE_INI_FILE_BUF_SIZE];
        char *stringValue = PieIniFile_getProfileString(self, 
                        sectionName, key, PIE_INI_FILE_BUF_SIZE, buf);
        if (stringValue) {
                int a = 0;
                sscanf(stringValue, "%d", &a);
                return a;
        }
        return defaultValue;
}

/**
 * @brief Read integer vector; entries divided with ','
 *
 * @param self
 * @param sectionName
 * @param key
 * @param bufferSize - max buffer size in int
 * @param bufferAddr - int array start address
 *
 * @return number of integers in the output buffer
 */
int PieIniFile_getProfileVectorInt(PieIniFile *self,
                                   char *sectionName,
                                   char *key,
                                   int bufferSize,
                                   int *bufferAddr)
{
        char buf[PIE_INI_FILE_BUF_SIZE];
        char *stringValue = PieIniFile_getProfileString(self, 
                        sectionName, key, PIE_INI_FILE_BUF_SIZE, buf);
        if (!stringValue)
                return 0;

        int i = 0;
        char *p = strtok(stringValue, ",");
        while (i < bufferSize) {
                sscanf(p, "%d", &bufferAddr[i++]);
                p = strtok(PIE_NULL, ",");
                if (!p) 
                        break;
        }

        return i;
}

double PieIniFile_getProfileDouble(PieIniFile *self,
                                   char *sectionName,
                                   char *key,
                                   double defaultValue)
{
        char buf[PIE_INI_FILE_BUF_SIZE];
        char *stringValue = PieIniFile_getProfileString(self, 
                        sectionName, key, PIE_INI_FILE_BUF_SIZE, buf);
        if (stringValue) {
                double a = 0;
                sscanf(stringValue, "%lf", &a);
                return a;
        }
        return defaultValue;
}

int PieIniFile_getProfileVectorDouble(PieIniFile *self,
                                      char *sectionName,
                                      char *key,
                                      int bufferSize,
                                      double *bufferAddr)
{
        char buf[PIE_INI_FILE_BUF_SIZE];
        char *stringValue = PieIniFile_getProfileString(self, 
                        sectionName, key, PIE_INI_FILE_BUF_SIZE, buf);
        if (!stringValue)
                return 0;

        int i = 0;
        char *p = strtok(stringValue, ",");
        while (i < bufferSize) {
                sscanf(p, "%lf", &bufferAddr[i++]);
                p = strtok(PIE_NULL, ",");
                if (!p) 
                        break;
        }

        return i;
}

/**
 * @brief Set string profile; If the entry does not exist, new one will be 
 * created
 *
 * @param self
 * @param sectionName
 * @param key
 * @param stringSize
 * @param string
 *
 * @return op
 */
char * PieIniFile_setProfileString(PieIniFile *self,
                                   char *sectionName,
                                   char *key,
                                   int stringSize,
                                   char *string)
{
        PieIniSection *curSection = self->section;
        PieIniSection *tmpSection = PIE_NULL;
        self->isChanged = PIE_TRUE;
        while (curSection) {
                if (strcmp(curSection->sectionName, sectionName) != 0) {
                        curSection = curSection->next;
                        continue;
                }
                if (!tmpSection)
                        tmpSection = curSection;
                PieIniPair *curPair = curSection->pair;
                while (curPair) {
                        if (strcmp(curPair->key, key) != 0) {
                                curPair = curPair->next;
                                continue;
                        }
                        return strcpy(curPair->value, string);
                }
                curSection = curSection->next;
        }
        if (!tmpSection) {
                curSection = PieSlicePool_alloc(self->buffer);
                strcpy(curSection->sectionName, sectionName);
                curSection->pair = PIE_NULL;
                curSection->next = self->section;
                self->section = curSection;
        }
        PieIniPair *curPair = PieSlicePool_alloc(self->buffer);
        curPair->next = curSection->pair;
        curSection->pair = curPair;
        strcpy(curPair->key, key);
        curPair->value = curPair->key + strlen(curPair->key) + 1;
        return strcpy(curPair->value, string);
}

void PieIniFile_setProfileBoolean(PieIniFile *self, 
                                  char *sectionName,
                                  char *key,
                                  PieBoolean value)
{
        char buf[PIE_INI_FILE_BUF_SIZE];
        if (value == PIE_TRUE)
                strcpy(buf, "true");
        else
                strcpy(buf, "false");
        PieIniFile_setProfileString(self, 
                        sectionName, key, PIE_INI_FILE_BUF_SIZE, buf);
}

void PieIniFile_setProfileInt(PieIniFile *self, 
                              char *sectionName, 
                              char *key, 
                              int value)
{
        char buf[PIE_INI_FILE_BUF_SIZE];
        sprintf(buf, "%d", value);
        PieIniFile_setProfileString(self, 
                        sectionName, key, PIE_INI_FILE_BUF_SIZE, buf);
}

void PieIniFile_setProfileVectorInt(PieIniFile *self,
                                    char *sectionName,
                                    char *key,
                                    int vectorSize,
                                    int *vector)
{
        char buf[PIE_INI_FILE_BUF_SIZE];
        char *pBuf = buf;
        for (int i = 0; i < vectorSize; i++) {
                sprintf(pBuf, "%d,", vector[i]);
                pBuf = strchr(pBuf, ',') + 1;
        }
        *(pBuf - 1) = '\0';
        PieIniFile_setProfileString(self, 
                        sectionName, key, PIE_INI_FILE_BUF_SIZE, buf);
}

void PieIniFile_setProfileDouble(PieIniFile *self,
                                 char *sectionName,
                                 char *key,
                                 double value)
{
        char buf[PIE_INI_FILE_BUF_SIZE];
        sprintf(buf, "%lf", value);
        PieIniFile_setProfileString(self, 
                        sectionName, key, PIE_INI_FILE_BUF_SIZE, buf);
}

void PieIniFile_setProfileVectorDouble(PieIniFile *self,
                                       char *sectionName,
                                       char *key,
                                       int vectorSize,
                                       double *vector)
{
        char buf[PIE_INI_FILE_BUF_SIZE];
        char *pBuf = buf;
        for (int i = 0; i < vectorSize; i++) {
                sprintf(pBuf, "%f,", vector[i]);
                pBuf = strchr(pBuf, ',') + 1;
        }
        *(pBuf - 1) = '\0';
        PieIniFile_setProfileString(self, 
                        sectionName, key, PIE_INI_FILE_BUF_SIZE, buf);
}

void PieIniFile_delProfileSection(PieIniFile *self, char *sectionName)
{
        PieIniSection *curSection = self->section;
        PieIniSection *preSection = PIE_NULL;
        while (curSection) {
                if (strcmp(curSection->sectionName, sectionName) == 0) {
                        if (self->section == curSection)
                                self->section = curSection->next;
                        else
                                preSection->next = curSection->next;
                        PieIniSection *tmp = curSection;
                        curSection = curSection->next;
                        PieSlicePool_free(self->buffer, tmp);
                } else {
                        preSection = curSection;
                        curSection = curSection->next;
                }
        }
        self->isChanged = PIE_TRUE;
}

void PieIniFile_delProfile(PieIniFile *self, char *sectionName, char *key)
{
        PieIniSection *curSection = self->section;
        while (curSection) {
                if (strcmp(curSection->sectionName, sectionName) != 0) {
                        curSection = curSection->next;
                        continue;
                }
                PieIniPair *curPair = curSection->pair;
                PieIniPair *prePair = curPair;
                while (curPair) {
                        if (strcmp(curPair->key, key) != 0) {
                                prePair = curPair;
                                curPair = curPair->next;
                                continue;
                        }
                        if(prePair == curPair)
                                curSection->pair = curPair->next;
                        else
                                prePair->next = curPair->next;
                        PieIniPair *tmp = curPair;
                        curPair = curPair->next;
                        PieSlicePool_free(self->buffer, tmp);
                }
                curSection = curSection->next;
        }
        self->isChanged = PIE_TRUE;
}

// TestBench example

/* #include <stdio.h> */
/* #include <stdlib.h> */
/* #include <string.h> */
/* #include "plus/pie_ini_file.h" */

/* int main(int argc, char *argv[]) */
/* { */
        /* PieIniFile ini; */
        /* PieBoolean b = PieIniFile_init(&ini, "resource/test.ini"); */
        /* if (!b) */
                /* printf("file unloaded\n"); */
        /* char buf[200]; */
        /* char buf1[200]; */
        /* char buf2[200]; */
        /* char buf3[200]; */
        /* if (!b) */
                /* printf("file unwritten\n"); */
        /* while (1) { */
                /* printf("section:"); */
                /* gets(buf); */
                /* if (strcmp(buf, ".") == 0) */
                        /* break; */
                /* printf("key:"); */
                /* gets(buf1); */
                /* if (strcmp(buf1, "ddd") == 0) { */
                        /* PieIniFile_delProfileSection(&ini, buf); */
                        /* continue; */
                /* } */
                /* PieIniFile_getProfileString(&ini, buf, buf1, 200, buf2); */
                /* printf("string value:%s\n", buf2); */
                /* gets(buf3); */
                /* if (strcmp(buf3, "ddd") == 0) { */
                        /* PieIniFile_delProfile(&ini, buf, buf1); */
                        /* continue; */
                /* } */
                /* else if (strcmp(buf3, ".") != 0) */
                        /* PieIniFile_setProfileString( */
                                        /* &ini, buf, buf1, 200, buf3); */
                /* PieBoolean b = PieIniFile_getProfileBoolean( */
                                /* &ini, buf, buf1, PIE_TRUE); */
                /* printf("boolean value:%d\n", b); */
                /* [> gets(buf3); <] */
                /* [> if (strcmp(buf3, ".") != 0) <] */
                        /* [> PieIniFile_setProfileBoolean( <] */
                                        /* [> &ini, buf, buf1, !b); <] */
                /* int ii = PieIniFile_getProfileInt(&ini, buf, buf1, 12345); */
                /* printf("int value:%d\n", ii); */
                /* [> gets(buf3); <] */
                /* [> if (strcmp(buf3, ".") != 0) <] */
                        /* [> PieIniFile_setProfileInt( <] */
                                        /* [> &ini, buf, buf1, ii + 1); <] */
                /* double dd = PieIniFile_getProfileDouble(&ini, buf, buf1, 1.23); */
                /* printf("double value:%f\n", dd); */
                /* [> gets(buf3); <] */
                /* [> if (strcmp(buf3, ".") != 0) <] */
                        /* [> PieIniFile_setProfileInt( <] */
                                        /* [> &ini, buf, buf1, dd + 1.0); <] */
                /* int aa[10]; */
                /* ii = PieIniFile_getProfileVectorInt(&ini, buf, buf1, 10, aa); */
                /* printf("int vector:"); */
                /* for (int i = 0; i < ii; i++) { */
                        /* printf("%d,", aa[i]); */
                /* } */
                /* printf("\n"); */
                /* [> gets(buf3); <] */
                /* [> if (strcmp(buf3, ".") != 0) { <] */
                        /* [> aa[0] ++; <] */
                        /* [> PieIniFile_setProfileVectorInt( <] */
                                        /* [> &ini, buf, buf1, ii, aa); <] */
                /* [> } <] */
                /* double bb[10]; */
                /* ii = PieIniFile_getProfileVectorDouble( */
                                /* &ini, buf, buf1, 10, bb); */
                /* printf("double vector:"); */
                /* for (int i = 0; i < ii; i++) { */
                        /* printf("%f,", bb[i]); */
                /* } */
                /* printf("\n"); */
                /* [> gets(buf3); <] */
                /* [> if (strcmp(buf3, ".") != 0) { <] */
                        /* [> bb[0]+=1.2; <] */
                        /* [> PieIniFile_setProfileVectorDouble( <] */
                                        /* [> &ini, buf, buf1, ii, bb); <] */
                /* [> } <] */
        /* } */
        /* b = PieIniFile_saveAs(&ini, "resource/backup.ini"); */
        /* PieIniFile_destroy(&ini); */
        /* return 0; */
/* } */
