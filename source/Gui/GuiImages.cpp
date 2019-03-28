
#include "GuiImages.h"
#include "image_arrow.inc"
#include "image_background.inc"
#include "image_mousecursor.inc"
#include "image_noise.inc"
#include "image_selector.inc"
#include "image_floppy_disk.inc"
#include "image_keyboard.inc"
#include "image_button_yes.inc"
#include "image_button_no.inc"
#include "image_scanlines.inc"

Image *g_imgArrow       = NULL;
Image *g_imgBackground  = NULL;
Image *g_imgMousecursor = NULL;
Image *g_imgNoise       = NULL;
Image *g_imgSelector    = NULL;
Image *g_imgFloppyDisk  = NULL;
Image *g_imgKeyboard    = NULL;
Image *g_imgButtonYes   = NULL;
Image *g_imgButtonNo    = NULL;
Image *g_scanlines    = NULL;

void GuiImageInit(void)
{
    g_imgArrow = new Image;
    if(g_imgArrow->LoadImage(image_arrow) != IMG_LOAD_ERROR_NONE) exit(0);
    g_imgBackground = new Image;
    if(g_imgBackground->LoadImage(image_background) != IMG_LOAD_ERROR_NONE) exit(0);
    g_imgMousecursor = new Image;
    if(g_imgMousecursor->LoadImage(image_mousecursor) != IMG_LOAD_ERROR_NONE) exit(0);
    g_imgNoise = new Image;
    if(g_imgNoise->LoadImage(image_noise) != IMG_LOAD_ERROR_NONE) exit(0);
    g_imgSelector = new Image;
    if(g_imgSelector->LoadImage(image_selector) != IMG_LOAD_ERROR_NONE) exit(0);
    g_imgFloppyDisk = new Image;
    if(g_imgFloppyDisk->LoadImage(image_floppy_disk) != IMG_LOAD_ERROR_NONE) exit(0);
    g_imgKeyboard = new Image;
    if(g_imgKeyboard->LoadImage(image_keyboard) != IMG_LOAD_ERROR_NONE) exit(0);
    g_imgButtonYes = new Image;
    if(g_imgButtonYes->LoadImage(image_button_yes) != IMG_LOAD_ERROR_NONE) exit(0);
    g_imgButtonNo = new Image;
    if(g_imgButtonNo->LoadImage(image_button_no) != IMG_LOAD_ERROR_NONE) exit(0);
    g_scanlines = new Image;
    if(g_scanlines->LoadImage(image_scanlines) != IMG_LOAD_ERROR_NONE) exit(0);
}

void GuiImageClose(void)
{
    delete g_imgButtonNo;
    g_imgButtonNo = NULL;
    delete g_imgButtonYes;
    g_imgButtonYes = NULL;
    delete g_imgKeyboard;
    g_imgKeyboard = NULL;
    delete g_imgFloppyDisk;
    g_imgFloppyDisk = NULL;
    delete g_imgSelector;
    g_imgSelector = NULL;
    delete g_imgNoise;
    g_imgNoise = NULL;
    delete g_imgMousecursor;
    g_imgMousecursor = NULL;
    delete g_imgBackground;
    g_imgBackground = NULL;
    delete g_imgArrow;
    g_imgArrow = NULL;
    delete g_scanlines;
    g_scanlines = NULL;
}

