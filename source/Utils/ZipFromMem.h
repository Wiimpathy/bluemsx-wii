/*****************************************************************************
** $Source: /cvsroot/bluemsx/blueMSX/Src/Utils/ZipFromMem.h,v $
**
** $Revision: 1.4 $
**
** $Date: 2008/03/30 18:38:47 $
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
#ifndef ZIPFROMMEM_H
#define ZIPFROMMEM_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include "zip.h"
#include "unzip.h"

typedef enum {
  MZMODE_READ_ONLY,
  MZMODE_MODIFY,
  MZMODE_CREATE
} MZMODE;

typedef struct {
    FILE *file;
    zlib_filefunc_def zlib_filefunc;
    const char *filename;
    void *buffer;
    int size;
    int index;
    MZMODE mode;
    unzFile unzip;
    zipFile zip;
} MemZip;

extern MemZip* MemZipOpenResource(void *data, unsigned int size);
extern MemZip* MemZipOpenZip(const char *filename, MZMODE mode);
extern void MemZipClose(MemZip *zip);

#ifdef __cplusplus
}
#endif

#endif
