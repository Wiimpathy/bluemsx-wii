/*****************************************************************************
** $Source: WiiNotifications.c,v $
**
** $Revision: 1.5 $
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
#include "ArchNotifications.h"
#include "FrameBuffer.h"
#include <stdio.h>
#include <stdlib.h>
#include <png.h>

static png_bytep  png_data = NULL;
static png_size_t png_data_allocated = 0;

static void MyPngErrorFunction(png_structp png_ptr, const char *err_msg)
{
	fprintf(stderr, "%s", err_msg);
}

static void MyPngWarningFunction(png_structp png_ptr, const char *warn_msg)
{
	return;
}

static void MyPngWriteData(png_structp png_ptr, png_bytep data, png_size_t size)
{
    png_size_t index = png_data_allocated;
    (void)png_ptr;
    png_data_allocated += size;
    png_data = realloc(png_data, png_data_allocated);
    memcpy(png_data + index, data, size);
}

static void MyPngFlushData(png_structp png_ptr)
{
    (void)png_ptr;
}

static int WritePng(FrameBuffer *frame, FILE *outfp)
{
    png_structp png_ptr = NULL;
    png_infop  info_ptr = NULL;
	png_color palette[256];
	unsigned char **row_pointers;
	int pal_used;
	int i, x, y;
	int retval = 0;

	memset(&palette, 0, sizeof(palette));

    png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING,
		                              NULL, MyPngErrorFunction, MyPngWarningFunction);
	if (!png_ptr) {
		fprintf(stderr, "Error creating write_struct\n");
		goto done;
	}

    info_ptr = png_create_info_struct(png_ptr);
    if (!info_ptr) {
		fprintf(stderr, "Error creating write_info_struct\n");
        goto done;
    }

    if( outfp ) {
    	png_init_io(png_ptr, outfp);
    }else{
        png_set_write_fn(png_ptr, NULL, MyPngWriteData, MyPngFlushData);
    }

    /* find out height/width and adjustment */
    int right = 0, left = FB_MAX_LINE_WIDTH-1;
    int top = 0, bottom = 0;
    int yscale = (frame->lines > 240)? 2 : 1;
	for(y = 0; y < frame->lines; y += yscale) {
        LineBuffer *line = &frame->line[y];
        int xscale = line->doubleWidth? 2 : 1;
        UInt16 *ps = line->buffer;
        UInt16 border_color = *ps;
        int l, r;
		for(l = 0; l < 272 && ps[l*xscale] == border_color; l++);
		for(r = 272-1; r > 0 && ps[r*xscale] == border_color; r--);
        if( l < r ) {
            if( l < left ) left = l;
            if( r > right ) right = r;
            if( top == 0 ) top = y;
            bottom = y;
        }
	}
    top /= yscale;
    bottom /= yscale;
    int height = bottom - top + 1;
    height = (height > 192)? 212 : 192;
    left = ((right + left + 1) >> 1) - 128;
    left = (left < 0)? 0 : ((left > 16)? 16 : left);
    int min = ((240 - height) / 2) - 8;
    int max = ((240 - height) / 2) + 8;
    top = ((bottom + top + 1) >> 1) - (height >> 1);
    top = (top < min)? min : ((top > max)? max : top);

    /* set png header */
    png_set_IHDR(png_ptr, info_ptr, 256, height, 8,
        		 PNG_COLOR_TYPE_PALETTE, PNG_FILTER_TYPE_BASE,
        		 PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);

    /* create 8 bpp image + pallette */
    row_pointers = (unsigned char**)malloc(height * sizeof(char*));
    pal_used = 0;
	for(y = 0; y < (int)height; y++) {
        LineBuffer *line = &frame->line[(y+top)*yscale];
        int xscale = line->doubleWidth? 2 : 1;
        UInt16 *ps = &line->buffer[left*xscale];
        unsigned char *pd = (unsigned char *)malloc(256);
        row_pointers[y] = pd;
		for(x = 0; x < 256 ; x++) {
            int found;
            png_byte red = (png_byte)((*ps >> 11) & 0x1f);
            png_byte green = (png_byte)((*ps >> 6) & 0x1f);
            png_byte blue = (png_byte)((*ps) & 0x1f);
            red = (red << 3) | (red >> 2);
            green = (green << 3) | (green >> 2);
            blue = (blue << 3) | (blue >> 2);
            for(i = 0, found = 0; i < pal_used; i++) {
                png_color *c = &palette[i];
                if( c->red == red && c->green == green && c->blue == blue ) {
                    *pd = i;
                    found = 1;
                    break;
                }
            }
            if( !found ) {
                if( pal_used < 256 ) {
                    palette[pal_used].red = red;
                    palette[pal_used].green = green;
                    palette[pal_used].blue = blue;
                    *pd = pal_used++;
                }else{
                    *pd = 0;
                }
            }
            ps += xscale; pd++;
		}
	}

	/* ... set palette colors ... */
	png_set_PLTE(png_ptr, info_ptr, palette, pal_used);

	png_write_info(png_ptr, info_ptr);

	png_set_packing(png_ptr);

	png_write_image(png_ptr, row_pointers);

	png_write_end(png_ptr, info_ptr);

	/* free row_pointers array */
	for(i = 0; i < (int)height; i++) {
		free(row_pointers[i]);
	}
    free(row_pointers);

	retval = 1;
done:

	if(png_ptr) {
		png_destroy_write_struct(&png_ptr, &info_ptr);
	}
    if(!retval && png_data) {
        free(png_data);
        png_data = NULL;
        png_data_allocated = 0;
    }
	return retval;
}


void* archScreenCapture(ScreenCaptureType type, int* bitmapSize, int onlyBmp)
{
    FrameBuffer *frame = frameBufferGetViewFrame();
    if( frame ) {
        png_bytep p;
        (void)WritePng(frame, NULL);
        p = png_data;
        if( bitmapSize ) {
            *bitmapSize = png_data_allocated;
        }
        png_data = NULL;
        png_data_allocated = 0;
        return p;
    }
    return NULL;
}

int archScreenCaptureToFile(ScreenCaptureType type, const char *fname)
{
    FrameBuffer *frame = frameBufferGetViewFrame();
    if( frame ) {
        FILE *outfp = fopen(fname, "wb");
        if( outfp ) {
            int success = WritePng(frame, outfp);
            fclose(outfp);
            return success;
        }
    }
    return 0;
}

void archUpdateEmuDisplayConfig() {}

/* Moved to Bluemsxlite.cpp
void archDiskQuickChangeNotify() {}
*/
void archEmulationStartNotification() {}
void archEmulationStopNotification() {}
void archEmulationStartFailure() {}

void archQuit() {}

void archThemeSetNext() {}
void archThemeUpdate(struct Theme* theme) {}

void archVideoOutputChange() {}
void archMinimizeMainWindow() {}

int archGetFramesPerSecond() { return 60; }

void* archWindowCreate(struct Theme* theme, int childWindow) { return NULL; }
void archWindowStartMove() {}
void archWindowMove() {}
void archWindowEndMove() {}

void archVideoCaptureSave() {}
