#ifndef _GUI_CONSOLE_H
#define _GUI_CONSOLE_H

#include "GuiManager.h"
#include "GuiContainer.h"
#include "DrawableImage.h"

class GuiConsole {
	public:
		GuiConsole(GuiManager *manager, int posx, int posy, int width, int height);
		virtual ~GuiConsole();
        void Remove(void);
        void Add(void);
		void SetVisible(bool state);
        bool IsVisible(void);
        void Render(void);
        void SetPosition( int posx,  int posy);

	private:
        static void RenderWrapper(void *arg);
        bool _visible;
		int _width, _height, _imgwidth, _imgheight;
        u16 *_console_buffer;
        GuiContainer *_container;
        GuiManager *_manager;
		DrawableImage *_image;
		Sprite* _sprite;
};

#endif
