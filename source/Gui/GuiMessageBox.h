
#ifndef _GUI_MESSAGEBOX_H
#define _GUI_MESSAGEBOX_H

#include "GuiManager.h"
#include "GuiContainer.h"
#include "DrawableImage.h"

class GuiMessageBox {
public:
    GuiMessageBox(GuiManager *man);
    virtual ~GuiMessageBox();

    void ShowPopup(const char *txt, Image *image = NULL, int alpha = 128);
    bool Show(const char *txt, Image *image = NULL, bool yesno = false, int alpha = 128);
    void Remove(void);
    void SetText(const char *fmt, ...);

    void MessageBoxPopupThread(void);
private:
    bool is_showing;
    GuiManager *manager;
    GuiContainer *container;
    Sprite *txt_sprite;
    Sprite *img_sprite;
    Sprite *spr_yes;
    Sprite *spr_no;
    DrawableImage *txt_image;
    void *thread_popup;
    bool quit_thread;

    bool DoSelection(Sprite *yes, Sprite *no);
};

#endif


