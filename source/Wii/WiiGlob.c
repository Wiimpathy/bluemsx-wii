/*****************************************************************************
** $Source: WiiGlob.c,v $
**
** $Revision: 1.2 $
**
** $Date: 2006/06/24 02:27:08 $
**
** More info: http://www.bluemsx.com
**
** Copyright (C) 2003-2006 Daniel Vik
**
**  This software is provided 'as-is', without any express or implied
**  warranty.  In no event will the authors be held liable for any damages
**  arising from the use of this software.
**
**  Permission is granted to anyone to use this software for any purpose,
**  including commercial applications, and to alter it and redistribute it
**  freely, subject to the following restrictions:
**
**  1. The origin of this software must not be misrepresented; you must not
**     claim that you wrote the original software. If you use this software
**     in a product, an acknowledgment in the product documentation would be
**     appreciated but is not required.
**  2. Altered source versions must be plainly marked as such, and must not be
**     misrepresented as being the original software.
**  3. This notice may not be removed or altered from any source distribution.
**
******************************************************************************
*/
#if 1

#include "ArchGlob.h"
#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#include <fat.h>
#include <string.h>
#include <sys/dir.h>
#include <unistd.h>

#define MAX_PATH FILENAME_MAX
#define FILE_ATTRIBUTE_DIRECTORY 1
#define INVALID_HANDLE_VALUE NULL

typedef unsigned int DWORD;

typedef void* HANDLE;

typedef struct {
    // Private
	DIR *dirPointer;
	int isdir;
	char cPattern[MAX_PATH + 1];
	// Public
	char cFileName[MAX_PATH + 1];
} WIN32_FIND_DATA;

void GetCurrentDirectory(int max, char *path)
{
    (void)getcwd(path, max);
}

void SetCurrentDirectory(char *path)
{
    chdir(path);
}

void FindClose(HANDLE handle)
{
    WIN32_FIND_DATA *wfd = (WIN32_FIND_DATA *)handle;

	if( !wfd || !wfd->dirPointer ) {
		fprintf(stderr, "Couldn't close directory!\n");
        return;
	}
	// Pass the DIR_ITER pointer to libfat and shut 'er down.
    (void)closedir(wfd->dirPointer);
	wfd->dirPointer = NULL;
}

int FindNextFile(HANDLE handle, WIN32_FIND_DATA *wfd)
{
    (void)handle;

	if( wfd == NULL || wfd->dirPointer == NULL ) {
		fprintf(stderr, "Invalid find data in FinNextFile!\n");
        return 0;
	}

    struct dirent *entry = readdir(wfd->dirPointer);


    for(;;) {
		if(entry == NULL )
		{
            //fprintf(stderr, "No\n");
			return 0; // No more files found
		}else{

            snprintf(wfd->cFileName, sizeof(wfd->cFileName), "%s", entry->d_name);
            if (entry->d_type == DT_DIR)
            {
                wfd->isdir = 1;
            }
            else
            {
                wfd->isdir = 0;
            }

			// Pattern validation
			//fprintf(stderr, "Matching '%s' with '%s' ... ", wfd->cPattern, wfd->cFileName);
			char *pp = wfd->cPattern;
			char *pf = wfd->cFileName;
			while( *pp != '\0' ) {
				if( *pp == '?' && *pf != '\0' ) pf++;
				if( *pp == '*' ) {
					while( *pf != '\0' && *pf != *(pp+1) ) pf++;
				}
				if( *pp != '?' && *pp != '*' ) {
				  if( toupper(*pp) != toupper(*pf) ) {
					break; // No match
				  }
				  pf++;
				}
				pp++;
				if( *pf == '\0' && *pp == '\0' ) {
                    //fprintf(stderr, "Yes\n");
					return 1; // Match
				}
			}
        }
        entry = readdir(wfd->dirPointer);
    }
    //fprintf(stderr, "Error\n");
	return 0; // never get here
}

HANDLE FindFirstFile(const char *pattern, WIN32_FIND_DATA *wfd)
{
    char current_dir[MAX_PATH+1] = "/";

    (void)getcwd(current_dir, sizeof(current_dir));
    wfd->dirPointer = opendir(current_dir);
	if( wfd->dirPointer == NULL ) {
		fprintf(stderr, "Could not open directory '%s'!\n", current_dir);
		return NULL; // diropen sets errno for us, so just return NULL.
	}

    strncpy(wfd->cPattern, pattern, MAX_PATH);

    if( FindNextFile(NULL, wfd) ) { // Find first entry
		return wfd;
	}else{
		FindClose(wfd);
		return NULL;
	}
}

DWORD GetFileAttributesWfd(WIN32_FIND_DATA *wfd)
{
    DWORD attr = 0;

	if( !wfd || !wfd->dirPointer ) {
		fprintf(stderr, "Invalid handle in GetFileAttributes!\n");
        return 0;
	}
    //fprintf(stderr, "<%X>", wfd->fileStat.st_mode);

    if( wfd->isdir == 1 ) {
        attr |= FILE_ATTRIBUTE_DIRECTORY;
    }

	return attr;
}

// This glob only support very basic globbing, dirs in the patterns are only
// supported without any wildcards

ArchGlob* archGlob(const char* pattern, int flags)
{
    char oldPath[MAX_PATH];
    const char* filePattern;
    ArchGlob* glob;
    WIN32_FIND_DATA wfd;
    HANDLE handle;

    GetCurrentDirectory(MAX_PATH, oldPath);

    filePattern = strrchr(pattern, '/');
    if (filePattern == NULL) {
        filePattern = pattern;
    }
    else {
        char relPath[MAX_PATH];
        strcpy(relPath, pattern);
        relPath[filePattern - pattern] = '\0';
        pattern = filePattern + 1;
        SetCurrentDirectory(relPath);
    }

    handle = FindFirstFile(pattern, &wfd);
    if (handle == INVALID_HANDLE_VALUE) {
        SetCurrentDirectory(oldPath);
        return NULL;
    }

    glob = (ArchGlob*)calloc(1, sizeof(ArchGlob));

    do {
        DWORD fa;
        if (0 == strcmp(wfd.cFileName, ".") || 0 == strcmp(wfd.cFileName, "..")) {
            continue;
        }
        //fprintf(stderr, "found '%s' ... ", wfd.cFileName);
		fa = GetFileAttributesWfd(&wfd);
        if (((flags & ARCH_GLOB_DIRS) && (fa & FILE_ATTRIBUTE_DIRECTORY) != 0) ||
            ((flags & ARCH_GLOB_FILES) && (fa & FILE_ATTRIBUTE_DIRECTORY) == 0))
        {
            char* path = (char*)malloc(MAX_PATH);
            GetCurrentDirectory(MAX_PATH, path);
            //strcat(path, "/");
            strcat(path, wfd.cFileName);
            //fprintf(stderr, "ok\n");

            glob->count++;
            glob->pathVector = realloc(glob->pathVector, sizeof(char*) * glob->count);
            glob->pathVector[glob->count - 1] = path;
        }else{
            //fprintf(stderr, "bad attr (%d, %d)\n", flags, fa);
        }
        //archThreadSleep(1000);
    } while (FindNextFile(handle, &wfd));

    FindClose(handle);

    SetCurrentDirectory(oldPath);

    return glob;
}

void archGlobFree(ArchGlob* globHandle)
{
    int i;

    if (globHandle == NULL) {
        return;
    }

    for (i = 0; i < globHandle->count; i++) {
        free(globHandle->pathVector[i]);
    }

    if(globHandle->pathVector)
        free(globHandle->pathVector);
    free(globHandle);
}

#else
#ifdef WINDOWS_HOST

#include "ArchGlob.h"
#include <windows.h>
#include <stdlib.h>

// This glob only support very basic globbing, dirs in the patterns are only
// supported without any wildcards

ArchGlob* archGlob(const char* pattern, int flags)
{
    char oldPath[MAX_PATH];
    const char* filePattern;
    ArchGlob* glob;
    WIN32_FIND_DATA wfd;
    HANDLE handle;

    GetCurrentDirectory(MAX_PATH, oldPath);

    filePattern = strrchr(pattern, '/');
    if (filePattern == NULL) {
        filePattern = pattern;
    }
    else {
        char relPath[MAX_PATH];
        strcpy(relPath, pattern);
        relPath[filePattern - pattern] = '\0';
        pattern = filePattern + 1;
        SetCurrentDirectory(relPath);
    }

    handle = FindFirstFile(pattern, &wfd);
    if (handle == INVALID_HANDLE_VALUE) {
        SetCurrentDirectory(oldPath);
        return NULL;
    }

    glob = (ArchGlob*)calloc(1, sizeof(ArchGlob));

    do {
        DWORD fa;
        if (0 == strcmp(wfd.cFileName, ".") || 0 == strcmp(wfd.cFileName, "..")) {
            continue;
        }
		fa = GetFileAttributes(wfd.cFileName);
        if (((flags & ARCH_GLOB_DIRS) && (fa & FILE_ATTRIBUTE_DIRECTORY) != 0) ||
            ((flags & ARCH_GLOB_FILES) && (fa & FILE_ATTRIBUTE_DIRECTORY) == 0))
        {
            char* path = (char*)malloc(MAX_PATH);
            GetCurrentDirectory(MAX_PATH, path);
            strcat(path, "\\");
            strcat(path, wfd.cFileName);

            glob->count++;
            glob->pathVector = realloc(glob->pathVector, sizeof(char*) * glob->count);
            glob->pathVector[glob->count - 1] = path;
        }
    } while (FindNextFile(handle, &wfd));

    FindClose(handle);

    SetCurrentDirectory(oldPath);

    return glob;
}

void archGlobFree(ArchGlob* globHandle)
{
    int i;

    if (globHandle == NULL) {
        return;
    }

    for (i = 0; i < globHandle->count; i++) {
        free(globHandle->pathVector[i]);
    }
    free(globHandle->pathVector);
    free(globHandle);
}

#else

#include "ArchGlob.h"
#include "glob.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

ArchGlob* archGlob(const char* pattern, int flags)
{
    ArchGlob* globHandle;
    glob_t g;
    int rv;
    int i;

    rv = glob(pattern, GLOB_MARK, NULL, &g);
    if (rv != 0) {
        return NULL;
    }

    globHandle = (ArchGlob*)calloc(1, sizeof(ArchGlob));

    for (i = 0; i < g.gl_pathc; i++) {
        char* path = g.gl_pathv[i];
        int len = strlen(path);

        if ((flags & ARCH_GLOB_DIRS) && path[len - 1] == '/') {
            char* storePath = calloc(1, len);
            memcpy(storePath, path, len - 1);
            globHandle->count++;
            globHandle->pathVector = realloc(globHandle->pathVector, sizeof(char*) * globHandle->count);
            globHandle->pathVector[globHandle->count - 1] = storePath;
        }

        if ((flags & ARCH_GLOB_FILES) && path[len - 1] != '/') {
            char* storePath = calloc(1, len + 1);
            memcpy(storePath, path, len);
            globHandle->count++;
            globHandle->pathVector = realloc(globHandle->pathVector, sizeof(char*) * globHandle->count);
            globHandle->pathVector[globHandle->count - 1] = storePath;
        }
    }

    globfree(&g);

    return globHandle;
}

void archGlobFree(ArchGlob* globHandle)
{
    int i;

    if (globHandle == NULL) {
        return;
    }

    for (i = 0; i < globHandle->count; i++) {
        free(globHandle->pathVector[i]);
    }
    if (globHandle->pathVector != NULL) {
        free(globHandle->pathVector);
    }
    free(globHandle);
}

#endif
#endif
