/*****************************************************************************
** $Source: /cvsroot/bluemsx/blueMSX/Src/Utils/ZipFromMem.c,v $
**
** $Revision: 0.0 $
**
** $Date: 2008/03/30 21:38:43 $
**
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Software
** Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
**
******************************************************************************
*/
#include "ZipFromMem.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


/******************************************************************************
*** Description
***     Functions to support reading zip files from memory resources.
***
******************************************************************************/

void *fopen_mem_func(void *opaque, const char *filename, int mode)
{
    MemZip *memzip = (MemZip*)opaque;
    memzip->index = 0;
    return (void *)memzip;
}

unsigned long fread_mem_func(void *opaque, void *stream, void *buf, unsigned long size)
{
    MemZip *memzip = (MemZip *)opaque;
    if( memzip->index + size > memzip->size ) {
        size = memzip->size - memzip->index;
    }
    memcpy(buf, (char*)memzip->buffer + memzip->index, size);
    memzip->index += size;
    return size;
}

unsigned long fwrite_mem_func(void *opaque, void *stream, const void *buf, unsigned long size)
{
    MemZip *memzip = (MemZip *)opaque;
    if( memzip->mode == MZMODE_READ_ONLY ) {
        return -1;
    }
    if( memzip->index + size > memzip->size ) {
        memzip->size = memzip->index + size;
        memzip->buffer = realloc(memzip->buffer, memzip->size);
    }
    memcpy((char*)memzip->buffer + memzip->index, buf, size);
    memzip->index += size;
    return size;
}

long ftell_mem_func(void *opaque, void *stream)
{
    MemZip *memzip = (MemZip *)opaque;
    return memzip->index;
}

long fseek_mem_func(void *opaque, void *stream, unsigned long offset, int origin)
{
    MemZip *memzip = (MemZip *)opaque;
    switch (origin)
    {
    case ZLIB_FILEFUNC_SEEK_CUR :
        memzip->index += offset;
        break;
    case ZLIB_FILEFUNC_SEEK_END :
        memzip->index = memzip->size - offset;
        break;
    case ZLIB_FILEFUNC_SEEK_SET :
        memzip->index = offset;
        break;
    default: return -1;
    }
    if( memzip->index > memzip->size ) memzip->index = memzip->size;
    if( memzip->index < 0 ) memzip->index = 0;
    return 0;
}

int fclose_mem_func(void *opaque, void *stream)
{
    return 0;
}

int ferror_mem_func(void *opaque, void *stream)
{
    return 0;
}

MemZip* MemZipOpenResource(void *data, unsigned int size)
{
    MemZip *zip = (MemZip*)malloc(sizeof(MemZip));

    zip->unzip = NULL;
    zip->zip = NULL;
    zip->filename = NULL;
    zip->file = NULL;
    zip->mode = MZMODE_READ_ONLY;
    zip->buffer = data;
    zip->index = 0;
    zip->size = size;

    zip->zlib_filefunc.opaque = zip;
    zip->zlib_filefunc.zopen_file = fopen_mem_func;
    zip->zlib_filefunc.zread_file = fread_mem_func;
    zip->zlib_filefunc.zwrite_file = fwrite_mem_func;
    zip->zlib_filefunc.ztell_file = ftell_mem_func;
    zip->zlib_filefunc.zseek_file = fseek_mem_func;
    zip->zlib_filefunc.zclose_file = fclose_mem_func;
    zip->zlib_filefunc.zerror_file = ferror_mem_func;

    zip->unzip = unzOpen2(NULL, &zip->zlib_filefunc);
    if( zip->unzip == NULL ) {
        free(zip);
        return NULL;
    }
    return zip;
}

MemZip* MemZipOpenZip(const char *filename, MZMODE mode)
{
    MemZip *zip = (MemZip*)malloc(sizeof(MemZip));

    zip->unzip = NULL;
    zip->zip = NULL;
    zip->filename = filename;
    zip->file = NULL;
    zip->mode = mode;
    zip->buffer = NULL;
    zip->index = 0;
    zip->size = 0;

    /* read existing zip */
    if( mode == MZMODE_READ_ONLY || mode == MZMODE_MODIFY ) {
        FILE *file = fopen(filename, "rb");
        if( file == NULL ) {
            free(zip);
            return NULL;
        }
        fseek(file, 0, SEEK_END);
        zip->size = ftell(file);
        fseek(file, 0, SEEK_SET);
        zip->buffer = malloc(zip->size);
        fread(zip->buffer, 1, zip->size, file);
        fclose(file);
    }

    /* create file if needed */
    if( mode == MZMODE_CREATE || mode == MZMODE_MODIFY ) {
        zip->file = fopen(filename, "wb");
        if( zip->file == NULL ) {
            if( zip->buffer != NULL ) {
                free(zip->buffer);
            }
            free(zip);
            return NULL;
        }
    }

    zip->zlib_filefunc.opaque = zip;
    zip->zlib_filefunc.zopen_file = fopen_mem_func;
    zip->zlib_filefunc.zread_file = fread_mem_func;
    zip->zlib_filefunc.zwrite_file = fwrite_mem_func;
    zip->zlib_filefunc.ztell_file = ftell_mem_func;
    zip->zlib_filefunc.zseek_file = fseek_mem_func;
    zip->zlib_filefunc.zclose_file = fclose_mem_func;
    zip->zlib_filefunc.zerror_file = ferror_mem_func;

    if( mode == MZMODE_READ_ONLY ) {
        zip->unzip = unzOpen2(NULL, &zip->zlib_filefunc);
    }else{
        zip->zip = zipOpen2(NULL, (mode == MZMODE_MODIFY)? APPEND_STATUS_ADDINZIP : APPEND_STATUS_CREATE,
                            NULL, &zip->zlib_filefunc);
    }
    if( zip->zip == NULL && zip->unzip == NULL ) {
        if( zip->buffer != NULL ) {
            free(zip->buffer);
        }
        free(zip);
        return NULL;
    }
    return zip;
}

void MemZipClose(MemZip *zip)
{
    if( zip->unzip ) {
        unzClose(zip->unzip);
    }
    if( zip->zip ) {
        zipClose(zip->zip, NULL);
    }
    if( zip->file ) {
        fwrite(zip->buffer, 1, zip->size, zip->file);
        fclose(zip->file);
    }
    if( zip->buffer != NULL && zip->filename != NULL /* no resource */) {
        free(zip->buffer);
    }
    free(zip);
}

