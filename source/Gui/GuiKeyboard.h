
#ifndef _GUI_KEYBOARD_H
#define _GUI_KEYBOARD_H

#include "GuiManager.h"
#include "DrawableImage.h"

class GuiKeyboard {
public:
    GuiKeyboard(GuiManager *man);
    virtual ~GuiKeyboard();

    void Show(void);
    void Remove(void);
    bool IsShowing(void);
    void SetEnabled(bool enable);
    void Render(void);

private:
    static void RenderWrapper(void *arg);

    bool is_showing;
    bool is_enabled;
    bool is_hidden;
    int xpos;
    int ypos;
    int xsize;
    int ysize;
    float xscale;
    float yscale;
    int keymap1;
    int keymap2;

    GuiManager *manager;
    DrawableImage *img_hover;
    DrawableImage *img_pressed;
    Sprite *spr_image;
    Sprite *spr_cursor;
    Sprite *spr_hover;
    Sprite *spr_pressed;
};

#endif


