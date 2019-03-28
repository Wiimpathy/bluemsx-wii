
#include "GuiConsole.h"

using namespace wsp;

// Do not use the instrumented malloc to ignore the memory leak created here.
// Unfortunately we can't solve it because libogc does support a 'deinit' of the console
#undef malloc

GuiConsole::GuiConsole(GuiManager *manager, int posx, int posy, int width, int height)
{
    _manager = manager;
    _width = width;
    _height = height;
    _imgwidth = (_width-24)/2;
    _imgheight = (_height-24)/2;

    _container = new GuiContainer(posx, posy, _width, _height);
    _image = new DrawableImage;
    _image->CreateImage(_imgwidth, _imgheight);
    _sprite = new Sprite;
    _sprite->SetImage(_image->GetImage());
    _sprite->SetStretchWidth(2.0f);
    _sprite->SetStretchHeight(2.0f);
    _sprite->SetRefPixelPositioning(REFPIXEL_POS_PIXEL);
    _sprite->SetRefPixelPosition(0, 0);
    _sprite->SetPosition(posx+12, posy+12);
    SetVisible(false);

	_console_buffer = (u16*)malloc(_imgwidth*_imgheight*VI_DISPLAY_PIX_SZ);
	if( _console_buffer != NULL ) {
        console_init(_console_buffer, 0, 0, _imgwidth, _imgheight,
                     _imgwidth * VI_DISPLAY_PIX_SZ);
	}

	Render();
    Add();
    _manager->AddRenderCallback(RenderWrapper, (void*)this);
}

GuiConsole::~GuiConsole()
{
    _manager->RemoveRenderCallback(RenderWrapper, (void*)this);
    _manager->Remove(_sprite);
    _manager->Remove(_container);
    delete _sprite;
    delete _image;
    delete _container;
}

void GuiConsole::Add(void)
{
    _manager->AddTopFixed(_container);
    _manager->AddTopFixed(_sprite);
}

void GuiConsole::Remove(void)
{
    _manager->Remove(_container);
    _manager->Remove(_sprite);
}

void GuiConsole::SetPosition( int posx,  int posy)
{
    _sprite->SetPosition(posx+12, posy+12);
    _container->SetPosition(posx, posy);
}

void GuiConsole::SetVisible(bool state)
{
    _visible = state;
    _sprite->SetVisible(state);
    _container->SetVisible(state);
}

bool GuiConsole::IsVisible(void)
{
    return _visible;
}

void GuiConsole::RenderWrapper(void *arg)
{
    GuiConsole *me = (GuiConsole*)arg;
    me->Render();
}

void GuiConsole::Render(void)
{
    static int count = 0;
    if( _visible ) {
        u16 *pixels = (u16*)_image->GetTextureBuffer();
        _console_buffer[count] = 0;
        if( ++count >= _imgwidth ) count = 0;
        _console_buffer[count] = 0xffff;
        for(int y = 0; y < _imgheight; y += 4) {
            u16 *s1 = &_console_buffer[y * _imgwidth];
            u16 *s2 = &_console_buffer[(y+1) * _imgwidth];
            u16 *s3 = &_console_buffer[(y+2) * _imgwidth];
            u16 *s4 = &_console_buffer[(y+3) * _imgwidth];
            u16 *dst = pixels + y * _imgwidth * 2;
            for(int x = 0; x < _imgwidth; x += 4) {
                *dst++ = (*s1++ & 0xff00)? 0xffff : 0x0000;
                *dst++ = (*s1++ & 0xff00)? 0xffff : 0x0000;
                *dst++ = (*s1++ & 0xff00)? 0xffff : 0x0000;
                *dst++ = (*s1++ & 0xff00)? 0xffff : 0x0000;
                *dst++ = (*s2++ & 0xff00)? 0xffff : 0x0000;
                *dst++ = (*s2++ & 0xff00)? 0xffff : 0x0000;
                *dst++ = (*s2++ & 0xff00)? 0xffff : 0x0000;
                *dst++ = (*s2++ & 0xff00)? 0xffff : 0x0000;
                *dst++ = (*s3++ & 0xff00)? 0xffff : 0x0000;
                *dst++ = (*s3++ & 0xff00)? 0xffff : 0x0000;
                *dst++ = (*s3++ & 0xff00)? 0xffff : 0x0000;
                *dst++ = (*s3++ & 0xff00)? 0xffff : 0x0000;
                *dst++ = (*s4++ & 0xff00)? 0xffff : 0x0000;
                *dst++ = (*s4++ & 0xff00)? 0xffff : 0x0000;
                *dst++ = (*s4++ & 0xff00)? 0xffff : 0x0000;
                *dst++ = (*s4++ & 0xff00)? 0xffff : 0x0000;
                s1 -= 4;
                s2 -= 4;
                s3 -= 4;
                s4 -= 4;
                *dst++ = (*s1++ & 0xff00)? 0xffff : 0x0000;
                *dst++ = (*s1++ & 0xff00)? 0xffff : 0x0000;
                *dst++ = (*s1++ & 0xff00)? 0xffff : 0x0000;
                *dst++ = (*s1++ & 0xff00)? 0xffff : 0x0000;
                *dst++ = (*s2++ & 0xff00)? 0xffff : 0x0000;
                *dst++ = (*s2++ & 0xff00)? 0xffff : 0x0000;
                *dst++ = (*s2++ & 0xff00)? 0xffff : 0x0000;
                *dst++ = (*s2++ & 0xff00)? 0xffff : 0x0000;
                *dst++ = (*s3++ & 0xff00)? 0xffff : 0x0000;
                *dst++ = (*s3++ & 0xff00)? 0xffff : 0x0000;
                *dst++ = (*s3++ & 0xff00)? 0xffff : 0x0000;
                *dst++ = (*s3++ & 0xff00)? 0xffff : 0x0000;
                *dst++ = (*s4++ & 0xff00)? 0xffff : 0x0000;
                *dst++ = (*s4++ & 0xff00)? 0xffff : 0x0000;
                *dst++ = (*s4++ & 0xff00)? 0xffff : 0x0000;
                *dst++ = (*s4++ & 0xff00)? 0xffff : 0x0000;
            }
        }
    	DCFlushRange(pixels, _imgwidth * _imgheight * 4);
    }
}

