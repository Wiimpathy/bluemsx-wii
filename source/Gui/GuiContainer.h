#ifndef _GUI_CONTAINER_H
#define _GUI_CONTAINER_H

#include <wiisprite.h>

using namespace wsp;

class GuiContainer : public TiledLayer
{
	public:
		GuiContainer(int posx, int posy, int width, int height, int alpha = 128);
		virtual ~GuiContainer();
        void Render(void);
        void SetPosition(int posx,  int posy);
        int GetWidth(void);
        int GetHeight(void);

	private:
		int cols, rows;
		Image image;
};

#endif
