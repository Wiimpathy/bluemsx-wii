#ifndef _GUI_IMAGES_H
#define _GUI_IMAGES_H

#include <gccore.h>
#include <wiisprite.h>

using namespace wsp;

extern Image *g_imgArrow;
extern Image *g_imgBackground;
extern Image *g_imgMousecursor;
extern Image *g_imgNoise;
extern Image *g_imgSelector;
extern Image *g_imgFloppyDisk;
extern Image *g_imgKeyboard;
extern Image *g_imgButtonYes;
extern Image *g_imgButtonNo;
extern Image *g_scanlines;

extern void GuiImageInit(void);
extern void GuiImageClose(void);

#endif
