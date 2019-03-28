#include "image.h"
#include "gamewindow.h"

#include <stdio.h>
#include <malloc.h>

namespace wsp{
	Image::Image() : _pixels(NULL), _width(0), _height(0), _bytespp(0), _initialized(false){
	}

	Image::Image(Image *src) {
        _width = src->_width;
        _height = src->_height;
        _bytespp = src->_bytespp;
        _initialized = src->_initialized;
        if( src->_pixels != NULL ) {
            _pixels = (u8*)(memalign(32, _width*_height*_bytespp));
            memcpy(_pixels, src->_pixels, _width*_height*_bytespp);
            _Flush();
        }else{
            _pixels = NULL;
        }
	}

	Image::~Image(){
		DestroyImage();
	}

	// This is some helper stuff to please libpng and to enable loading from a buffer.
	static u32 __image_load_pos = 0;
	static void __image_load_buffer(png_structp png_ptr, png_bytep data, png_size_t length){
		unsigned char* buffer= (unsigned char*)png_get_io_ptr(png_ptr);
		memcpy(data, buffer + __image_load_pos, length);
		__image_load_pos += length;
	}

	IMG_LOAD_ERROR Image::LoadImage(const char* path, IMG_LOAD_TYPE loadtype){
		return LoadImage((const unsigned char*)path, loadtype);
	}

	IMG_LOAD_ERROR Image::LoadImage(const unsigned char* path, IMG_LOAD_TYPE loadtype){
		if(path == NULL)return IMG_LOAD_ERROR_NOT_FOUND;
		if(_initialized)return IMG_LOAD_ERROR_ALREADY_INIT;
		int color_type;
		int bit_depth;

		png_structp png_ptr;
		png_infop info_ptr;
		png_bytep *row_pointers;

		char header[8]; // Check for support PNG header.
		FILE* fp = NULL;
		void* buffer = (void*)path;

		if(loadtype == IMG_LOAD_TYPE_PATH){ // Load from file.
			fp = fopen((const char*)path, "rb");
			if(!fp)
				return IMG_LOAD_ERROR_NOT_FOUND;

			if(fread(header, 1, 8, fp)!=8){
				fclose(fp);
				return IMG_LOAD_ERROR_INV_PNG;
			}
		}else if(loadtype == IMG_LOAD_TYPE_BUFFER){ // Check the header directly from the buffer.
			memcpy(header, buffer, 8);
			if(png_sig_cmp((png_bytep)header, 0, 8)){
				return IMG_LOAD_ERROR_INV_PNG;
			}
		}

		if(png_sig_cmp((png_bytep)header, 0, 8)){
			if(loadtype == IMG_LOAD_TYPE_PATH)fclose(fp);
			return IMG_LOAD_ERROR_INV_PNG;
		}

		// Create a PNG handle
		png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);

		if(!png_ptr){
			if(loadtype == IMG_LOAD_TYPE_PATH)fclose(fp);
			return IMG_LOAD_ERROR_PNG_FAIL;
		}

		info_ptr = png_create_info_struct(png_ptr);
		if(!info_ptr){
			png_destroy_read_struct(&png_ptr, (png_infopp)NULL, (png_infopp)NULL);
			if(loadtype == IMG_LOAD_TYPE_PATH)fclose(fp);
			return IMG_LOAD_ERROR_PNG_FAIL;
		}

		if(setjmp(png_jmpbuf(png_ptr))){
			png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp)NULL);
			if(loadtype == IMG_LOAD_TYPE_PATH)fclose(fp);
			return IMG_LOAD_ERROR_PNG_FAIL;
		}

		if(loadtype == IMG_LOAD_TYPE_PATH){
			png_init_io(png_ptr, fp);
			png_set_sig_bytes(png_ptr, 8);
		}else if(loadtype == IMG_LOAD_TYPE_BUFFER){
			__image_load_pos = 0;
			png_set_read_fn(png_ptr, buffer, __image_load_buffer); // Why, libpng, oh why?
		}

		png_read_info(png_ptr, info_ptr);

        int interlace_type;
        png_get_IHDR(png_ptr, info_ptr, &_width, &_height, &bit_depth, &color_type,
            &interlace_type, NULL, NULL);

		if(_width % 4 != 0 || _height % 4 != 0){
			if(loadtype == false)fclose(fp);
			return IMG_LOAD_ERROR_WRONG_SIZE;
		}
		if(bit_depth > 8){
			if(loadtype == false)fclose(fp);
			return IMG_LOAD_ERROR_INV_PNG;
		}

        if (color_type == PNG_COLOR_TYPE_PALETTE)
            png_set_expand(png_ptr);
        if (color_type == PNG_COLOR_TYPE_GRAY && bit_depth < 8)
            png_set_expand(png_ptr);
        if (png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS))
            png_set_expand(png_ptr);
        if (color_type == PNG_COLOR_TYPE_GRAY ||
            color_type == PNG_COLOR_TYPE_GRAY_ALPHA)
            png_set_gray_to_rgb(png_ptr);
		if(color_type == PNG_COLOR_TYPE_RGBA) // Swap RGBA to ARGB
			png_set_swap_alpha(png_ptr);

		png_read_update_info(png_ptr, info_ptr);

        int rowbytes=png_get_rowbytes(png_ptr, info_ptr);
        int channels=(int)png_get_channels(png_ptr, info_ptr);
        if(channels<3 || channels>4) {
			png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp)NULL);
			if(loadtype == false)fclose(fp);
			return IMG_LOAD_ERROR_PNG_FAIL;
        }

		// Read File
		if(setjmp(png_jmpbuf(png_ptr))){
			png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp)NULL);
			if(loadtype == false)fclose(fp);
			return IMG_LOAD_ERROR_PNG_FAIL;
		}

		row_pointers = (png_bytep*) malloc(sizeof(png_bytep) * _height);
		if(!row_pointers){
			png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp)NULL);
			return IMG_LOAD_ERROR_PNG_FAIL;
		}
		for(u32 y = 0; y<_height; y++)
			row_pointers[y] = (png_byte*)malloc(rowbytes);

		png_read_image(png_ptr, row_pointers);

		if(loadtype == false)fclose(fp);

		if(_pixels)
			free(_pixels); _pixels = NULL;

        if(channels == 4) {
            _bytespp = 4;
        }else{
            _bytespp = 2;
        }
		_pixels = (u8*)(memalign(32, _width*_height*_bytespp));
		_ConvertTexture(color_type, channels, row_pointers);

		// Free up some memory
		if(row_pointers){
			for(u32 y = 0; y < _height; y++){
				free(row_pointers[y]); row_pointers[y] = NULL;
			}
			free(row_pointers); row_pointers = NULL;
		}
		png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp)NULL);

		// Everything was loaded fine
		_initialized = true;
		return IMG_LOAD_ERROR_NONE;
	}

	void Image::DestroyImage(){
		if(_initialized == false)return;
		_initialized = false;
		if(_pixels)
			free(_pixels); _pixels = NULL;
	}

	u32 Image::GetWidth() const{
		return _width;
	}
	u32 Image::GetHeight() const{
		return _height;
	}

	bool Image::IsInitialized() const{
		return _initialized;
	}

	void Image::BindTexture(bool bilinear){
		if(!_initialized)return;
		if(GameWindow::_lastimagebilinear == bilinear)
			if(GameWindow::_lastimage == this)
				return;

		GX_InitTexObj(&_texObj, (void*)_pixels, _width, _height, (_bytespp==2)? GX_TF_RGB565 : GX_TF_RGBA8, GX_CLAMP, GX_CLAMP, GX_FALSE);

		// This disables bilinear filtering if applicable
		if(!bilinear)GX_InitTexObjLOD(&_texObj, GX_NEAR, GX_NEAR, 0.0f, 0.0f, 0.0f, GX_DISABLE, GX_DISABLE, GX_ANISO_1);

		GX_LoadTexObj(&_texObj,GX_TEXMAP0);

		GameWindow::_lastimage = this;
		GameWindow::_lastimagebilinear = bilinear;
	}

	bool Image::_InitializeImage(u32 width, u32 height, u32 bytespp){
		if(width % 4 != 0 || height %4 != 0 || height == 0 || width == 0)return false;

		// Check for image data.
		if(_initialized)return false;
		if(_pixels)
			free(_pixels); _pixels = NULL;

		_width = width; _height = height; _bytespp = bytespp;

		// Allocate memory and erase it completely.
		_pixels = (u8*)(memalign(32, _width*_height*_bytespp));
		memset(_pixels, 0, _width*_height*_bytespp);

		_Flush();

		_initialized = true;
		return true;
	}

	void Image::_Flush(){
		// Move flush cached memory.
		DCFlushRange(_pixels, _width * _height * _bytespp);
	}

	void Image::_ConvertTexture(png_byte color_type, int channels, png_bytep* row_pointers){
        if(_bytespp == 2) {
            u16 *d = (u16*)(_pixels);
            u8 *s = NULL;
            for (u32 y = 0; y < _height; y += 4) {
                for (u32 x = 0; x < _width; x += 4) {
                    for (u32 r = 0; r < 4; ++r) {
                        s = &row_pointers[y + r][x*channels];
                        for (u32 k = 0; k < 4; ++k) {
                            u16 c;
                            c =  ((u16)(*s++) & 0xf8) << (11-3); //R
                            c |= ((u16)(*s++) & 0xf8) << (6-3); //G
                            c |= ((u16)(*s++) & 0xf8) >> 3; //B
                            if(channels==4) ++s;
                            *d++ = c;
                        }
                    }
                }
            }
        }else{
            // THANKS DHEWG!! My first born is yours.
            u8 *d = (u8*)(_pixels);
            u8 *s = NULL;
    		if(color_type == PNG_COLOR_TYPE_RGBA ||
              (color_type == PNG_COLOR_TYPE_PALETTE && channels==4) ||
              (color_type == PNG_COLOR_TYPE_GRAY && channels==4)) { // 32bit
    			for (u32 y = 0; y < _height; y += 4) {
    				for (u32 x = 0; x < _width; x += 4) {
    					for (u32 r = 0; r < 4; ++r) {
    						s = &row_pointers[y + r][x << 2];
    						*d++ = s[0];  *d++ = s[1];
    						*d++ = s[4];  *d++ = s[5];
    						*d++ = s[8];  *d++ = s[9];
    						*d++ = s[12]; *d++ = s[13];
    					}
    					for (int r = 0; r < 4; ++r) {
    						s = &row_pointers[y + r][x << 2];
    						*d++ = s[2];  *d++ = s[3];
    						*d++ = s[6];  *d++ = s[7];
    						*d++ = s[10]; *d++ = s[11];
    						*d++ = s[14]; *d++ = s[15];
    					}
    				}
    			}
    		}else
    		if(color_type == PNG_COLOR_TYPE_RGB ||
              (color_type == PNG_COLOR_TYPE_PALETTE && channels==3) ||
              (color_type == PNG_COLOR_TYPE_GRAY && channels==3)) { // 24bit
    			for (u32 y = 0; y < _height; y += 4) {
    				for (u32 x = 0; x < _width; x += 4) {
    					for (u32 r = 0; r < 4; ++r) {
    						s = &row_pointers[y + r][x * 3];
    						*d++ = 0xff;  *d++ = s[0];
    						*d++ = 0xff;  *d++ = s[3];
    						*d++ = 0xff;  *d++ = s[6];
    						*d++ = 0xff; *d++ = s[9];
    					}

    					for (int r = 0; r < 4; ++r) {
    						s = &row_pointers[y + r][x * 3];
    						*d++ = s[1];  *d++ = s[2];
    						*d++ = s[4];  *d++ = s[5];
    						*d++ = s[7]; *d++ = s[8];
    						*d++ = s[10]; *d++ = s[11];
    					}
    				}
    			}
    		}
        }

		_Flush();
	}
};
