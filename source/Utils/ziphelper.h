/*****************************************************************************
** $Source: /cvsroot/bluemsx/blueMSX/Src/Utils/ziphelper.h,v $
**
** $Revision: 1.4 $
**
** $Date: 2008/03/30 18:38:47 $
**
** More info: http://www.bluemsx.com
**
** Copyright (C) 2003-2006 Daniel Vik
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
#ifndef ZIPHELPER_H
#define ZIPHELPER_H

#include "zip.h"
#include "unzip.h"
#include "ZipFromMem.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef void(*ZIP_EXTRACT_CB)(int, int);

typedef struct {
    zipFile zip;
    const char *fileName;
    MemZip *memzip;
} ZipFile;

ZipFile* zipOpenFileForRead(const char* zipName, int cached);
void* zipLoadFileFromOpenZip(ZipFile* zip, const char* fileName, int* size);
void* zipLoadFile(const char* zipName, const char* fileName, int* size);
int zipCreateFile(const char* zipName);
int zipAppendFile(const char* fileName, void* buffer, int size);
void zipCloseReadFile(ZipFile *zip);
void zipCloseWriteFile(void);
int zipFileExists(const char* zipName, const char* fileName);
char* zipGetFileList(const char* zipName, const char* ext, int* count);
int zipHasFileType(char* zipName, char* ext);
int zipExtractCurrentfile(unzFile uf, int overwrite, const char* password);
int zipExtract(unzFile uf, int overwrite, const char* password, ZIP_EXTRACT_CB progress_callback);
void* zipCompress(void* buffer, int size, unsigned long* retSize);

#ifdef __cplusplus
}
#endif

#endif
