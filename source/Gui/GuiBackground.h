#ifndef _GUI_BACKGROUND_H
#define _GUI_BACKGROUND_H

#include "GuiManager.h"
#include "DrawableImage.h"

class GuiBackground
{
	public:
		GuiBackground(GuiManager *man);
		virtual ~GuiBackground();
        void Show(void);
        void Hide(void);

	private:
        GuiManager *manager;
		DrawableImage *imgTxt;
        Sprite *sprTxt;
        Sprite *sprBackground;
        bool is_shown;
};

#endif
