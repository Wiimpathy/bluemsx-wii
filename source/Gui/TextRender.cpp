// Code by DragonMinded
// Please attribute, nothing more

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <malloc.h>

#include "TextRender.h"

TextRender::TextRender()
{
	// Initialize the library
	FT_Error error = FT_Init_FreeType(&library);

    // Default values
    _yspacing = DEFAULT_Y_CUSHION;

	if(error)
	{
		// Better handling in the future
		exit(0);
	}
}

TextRender::~TextRender()
{
	// Free the library (and all resources)
	FT_Done_FreeType(library);
}

void TextRender::SetFont(string path)
{
	// Initialize a font
	FT_Error error = FT_New_Face(library, path.c_str(), 0, &face);

	if ( error )
	{
		// Better handling in the future
		exit(0);
	}
}

void TextRender::SetFont(const unsigned char* font, u32 size)
{
	// Initialize a font
	FT_Error error = FT_New_Memory_Face(library, font, size, 0, &face);
	_fontheight = size;

	if ( error )
	{
		// Better handling in the future
		exit(0);
	}
}

void TextRender::SetColor(GXColor c)
{
	_color = c;
}

void TextRender::SetSize(int s)
{
	// Set up the font face (see freetype samples for magic values)
	FT_Set_Pixel_Sizes(face, 0, s);
	_fontheight = s;
}

void TextRender::SetYSpacing(int s)
{
    _yspacing = s + DEFAULT_Y_CUSHION;
}

void TextRender::SetBuffer(uint8_t *buf, int width, int height)
{
	// Set up a buffer to render to (RGBA quads, most likely to be blitted into a DrawableImage and rendered using libwiisprite)
	_buf = buf;
	_width = width;
	_height = height;
}

void TextRender::Blit(FT_Bitmap *bmp, int left, int top)
{
	int runWidth = bmp->width;
	int runHeight = bmp->rows;

	// Precalculate the width
	if((left + bmp->width) >= _width)
	{
		// Run height needs adjustment
		runWidth = _width - (left + bmp->width);
	}

	// Precalculate the height
	if((top + bmp->rows) >= _height)
	{
		// Run height needs adjustment
		runHeight = _height - (top + bmp->rows);
	}

	// Copy alpha data over, setting the color to the predefined color and the alpha to the value of the glyph
	for(int y = 0; y < runHeight; y++)
	{
		// Precalculate
		int sywidth = (y * bmp->width);
		int dywidth = ((y + top) * _width);

		for(int x = 0; x < runWidth; x++)
		{
			// Precalculate
			int srcloc = x + sywidth;
			int dstloc = ((x + left) + dywidth) << 2;

			// Copy data over
			_buf[dstloc] = _color.r;
			_buf[dstloc + 1] = _color.g;
			_buf[dstloc + 2] = _color.b;
			_buf[dstloc + 3] = ((uint8_t *)bmp->buffer)[srcloc];
		}
	}
}

extern "C" void archThreadSleep(int milliseconds);

void TextRender::RenderSimple(const char *out, bool center, int *sx, int *sy)
{
	// Remember x position
	int maxx = 0;
	int x = DEFAULT_X;
	int y = DEFAULT_Y;

	// Shortcut from examples
	FT_GlyphSlot slot = face->glyph;

	// Render
	bool newline = true;
	for(uint32_t i = 0; i < strlen(out); i++)
	{
        if(center && newline) {
            int j = i;
            int s = 0;
            while(out[j] != '\0' && out[j] != '\r' && out[j] != '\n') {
                FT_Error error = FT_Load_Char(face, out[j], FT_LOAD_RENDER);
                if(error) break;  /* break on error */
                s += slot->advance.x >> 6;
                j++;
            }
            x += (_width - s) >> 1;
            if( x < 0 ) x = 0;
            newline = false;

        }
		if(out[i] == '\r' || out[i] == '\n')
		{
			// Newline
			x = DEFAULT_X;
			y += _fontheight + _yspacing;
            newline = true;

			continue;
		}
		else if(out[i] == '\t')
		{
			// Truncate to floor (nearest bounds)
			x /= DEFAULT_TAB_SPACE;
			x *= DEFAULT_TAB_SPACE;

			// Add tab stop
			x += DEFAULT_TAB_SPACE;

			continue;
		}

		// Render glyph
		FT_Error error = FT_Load_Char(face, out[i], FT_LOAD_RENDER);
		if(error) continue;  /* ignore errors */

		// Blit glyph to surface
		if( sx == NULL && sy == NULL ) {
    		Blit(&slot->bitmap, x + slot->bitmap_left, (y + _fontheight) - slot->bitmap_top);
		}

		// Advance the position
		x += slot->advance.x >> 6;
        if( x > maxx ) maxx = x;
	}
    if( sx ) *sx = maxx + 1;
    if( sy ) *sy = y + _fontheight + _yspacing + 1;
}

void TextRender::GetTextSize(int *sx, int *sy, bool center, const char *fmt, ...)
{
	// Need to make room for the sprintf'd text
	char *out = (char *)memalign(32, 1024);

	// Build using sprintf
	va_list marker;
	va_start(marker,fmt);
	vsprintf(out,fmt,marker);
	va_end(marker);

	// Call rendering engine
	RenderSimple(out, center, sx, sy);

	// Free memory
	free(out);
}

void TextRender::Render(const char *fmt, ...)
{
	// Need to make room for the sprintf'd text
	char *out = (char *)memalign(32, 1024);

	// Build using sprintf
	va_list marker;
	va_start(marker,fmt);
	vsprintf(out,fmt,marker);
	va_end(marker);

	// Call rendering engine
	RenderSimple(out);

	// Free memory
	free(out);
}
