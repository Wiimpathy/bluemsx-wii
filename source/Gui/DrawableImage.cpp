// Code by DragonMinded
// Please attribute, nothing more

#include <string.h>
#include "DrawableImage.h"
#include <malloc.h>
#include <wiisprite.h>

DrawableImage::DrawableImage()
{
	// Ensure we never draw if we haven't created
	_initialized = false;

	// Ensure proper null pointer
	_pixels = NULL;

    // Set default font style
    _font_size = 20;
    _font_yspacing = 0;
    _font_color = (GXColor){255,255,255,255};
}

DrawableImage::~DrawableImage()
{
	// Clear memory if needed
	if(_pixels)
	{
		free(_pixels);
		_pixels = NULL;
	}
}

void DrawableImage::CreateImage(int width, int height, int format)
{
	// Set parameters
	_format = format;
	_width = width;
	_height = height;
    int bytespp = (format == GX_TF_RGB565)? 2 : 4;

	// Allocate room
	if(_pixels)
	{
		free(_pixels);
	}
	_pixels = (u8 *)memalign(32, _width * _height * bytespp);

	// Set to zero's for now
	memset(_pixels, 0, _width * _height * bytespp);

	// Move flush cached memory
	DCFlushRange (_pixels, _width * _height * bytespp);

	// Set sprite as valid
	_initialized = true;
}

void DrawableImage::DestroyImage(void)
{
	if(_pixels)
	{
		free(_pixels);
		_pixels = NULL;
	}
}

void DrawableImage::BindTexture(bool bilinear)
{
	if(!_initialized)return;
		if(GameWindow::_lastimagebilinear == bilinear)
			if(GameWindow::_lastimage == this)
				return;

	GX_InitTexObj(&_texObj, _pixels, _width, _height, _format, GX_CLAMP, GX_CLAMP, GX_FALSE);

	// This disables bilinear filtering if applicable
	if(!bilinear)
		GX_InitTexObjLOD(&_texObj, GX_NEAR, GX_NEAR, 0.0f, 0.0f, 0.0f, GX_DISABLE, GX_DISABLE, GX_ANISO_1);

	GX_LoadTexObj(&_texObj,GX_TEXMAP0);

	GameWindow::_lastimage = this;
	GameWindow::_lastimagebilinear = bilinear;
}

u32 DrawableImage::GetWidth() const
{
	return _width;
}

u32 DrawableImage::GetHeight() const
{
	return _height;
}

bool DrawableImage::IsInitialized() const
{
	return _initialized;
}

void DrawableImage::CopyBuffer(u8 *buf)
{
	// This function assumes the u8 buffer is RGBA quads
    if( _format != GX_TF_RGBA8 )
        return;

	u8 *pixBuf = _pixels;

	// Loop in chunks of 4
	for(u32 j = 0; j < _height; j += 4)
	{
		for(u32 i = 0; i < _width; i += 4)
		{
			// First, copy AR chunks
			for(int y = 0; y < 4; y++)
			{
				for(int x = 0; x < 4; x++)
				{
					// Alpha
					*pixBuf++ = buf[(((i + x) + ((j + y) * _width)) * 4) + 3];

					// Red
					*pixBuf++ = buf[((i + x) + ((j + y) * _width)) * 4];
				}
			}

			// Now, copy GB chunks
			for(int y = 0; y < 4; y++)
			{
				for(int x = 0; x < 4; x++)
				{
					// Green
					*pixBuf++ = buf[(((i + x) + ((j + y) * _width)) * 4) + 1];

					// Blue
					*pixBuf++ = buf[(((i + x) + ((j + y) * _width)) * 4) + 2];
				}
			}
		}
	}

	// Move flush cached memory
	DCFlushRange (_pixels, _width * _height * 4);
}

Image *DrawableImage::GetImage()
{
	return (Image *)this;
}

u8 *DrawableImage::GetTextureBuffer(void)
{
    return _pixels;
}

void DrawableImage::FlushBuffer(void)
{
    int bytespp = (_format == GX_TF_RGB565)? 2 : 4;
    DCFlushRange (_pixels, _width * _height * bytespp);
}

void DrawableImage::SetFont(TextRender* f)
{
	font = f;
}

void DrawableImage::SetColor(GXColor c)
{
    _font_color = c;
}

void DrawableImage::SetSize(int s)
{
    _font_size = s;
}

void DrawableImage::SetYSpacing(int s)
{
    _font_yspacing = s;
}

void DrawableImage::RenderTextVA(bool center, const char *fmt, va_list list)
{
	// This function assumes the u8 buffer is RGBA quads
    if( _format != GX_TF_RGBA8 )
        return;

    // Set font style
	font->SetColor(_font_color);
	font->SetSize(_font_size);
    font->SetYSpacing(_font_yspacing);

	// Need to make room for the sprintf'd text
	char *out = (char *)memalign(32, 1024);

	// Need temporary blit buffer
	u8 *blitbuf = (u8 *)memalign(32, _width * _height * 4);
	memset(blitbuf, 0, _width * _height * 4);

	// Build using sprintf
	vsprintf(out,fmt,list);

	// Set up buffer
	font->SetBuffer(blitbuf, _width, _height);

	// Call rendering engine
	font->RenderSimple(out, center);

	// Blit to surface
	CopyBuffer(blitbuf);

	// Free memory
	free(blitbuf);
	free(out);
}

void DrawableImage::RenderText(const char *fmt, ...)
{
	va_list marker;
	va_start(marker,fmt);
	RenderTextVA(false, fmt,marker);
	va_end(marker);
}

void DrawableImage::RenderText(bool center, const char *fmt, ...)
{
	va_list marker;
	va_start(marker,fmt);
	RenderTextVA(center, fmt,marker);
	va_end(marker);
}

void DrawableImage::GetTextSize(int *sx, int *sy, const char *fmt, ...)
{
	// Need to make room for the sprintf'd text
	char *out = (char *)memalign(32, 1024);

    // Set font style
	font->SetColor(_font_color);
	font->SetSize(_font_size);
    font->SetYSpacing(_font_yspacing);

	// Build using sprintf
	va_list marker;
	va_start(marker,fmt);
	vsprintf(out,fmt,marker);
	va_end(marker);

	// Call rendering engine
	font->SetBuffer(NULL, 0x7fffffff, 0x7fffffff);
	font->RenderSimple(out, false, sx, sy);

	// Free memory
	free(out);
}

