#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <gccore.h>
#include <ogc/lwp_watchdog.h>

#include <wiiuse/wpad.h>

#include "kbdlib.h"
#include "GuiMessageBox.h"
#include "GuiContainer.h"

// Resources
#include "GuiImages.h"
#include "GuiFonts.h"

extern "C" {
#include "ArchThread.h"
}

#define BUTTON_TRANSPARENCY_NORMAL    160
#define BUTTON_TRANSPARENCY_HIGHLIGHT 255

#define MESSAGE_BOX_FADE_FRAMES       10

bool GuiMessageBox::DoSelection(Sprite *yes, Sprite *no)
{
    bool use_keyboard = true;

    manager->Lock();

    // Cursor
    Sprite *cursor = new Sprite;
    cursor->SetImage(g_imgMousecursor);
    cursor->SetPosition(0, 0);
    cursor->SetVisible(false);
    manager->AddTop(cursor);

    // Set default button
    bool selected = true;
    yes->SetTransparency(BUTTON_TRANSPARENCY_HIGHLIGHT);
    no->SetTransparency(BUTTON_TRANSPARENCY_NORMAL);

    manager->Unlock();

    (void)KBD_GetPadButtons(); // flush first

    for(;;) {
        // Buttons
        u32 buttons = KBD_GetPadButtons();

        manager->Lock();

        // Infrared
        int x, y, angle;
        if( manager->GetWiiMoteIR(&x, &y, &angle) ) {
            cursor->SetPosition(x, y);
            cursor->SetRotation(angle/2);
            cursor->SetVisible(true);
        }else{
            cursor->SetVisible(false);
            cursor->SetPosition(0, 0);
        }

        // Check mouse cursor colisions
        if( cursor->CollidesWith(yes, true) ) {
            yes->SetTransparency(BUTTON_TRANSPARENCY_HIGHLIGHT);
            no->SetTransparency(BUTTON_TRANSPARENCY_NORMAL);
            selected = true;
            use_keyboard = false;
            if( buttons & WPAD_BUTTON_A ) {
                break;
            }
        }else
        if( cursor->CollidesWith(no, true) ) {
            yes->SetTransparency(BUTTON_TRANSPARENCY_NORMAL);
            no->SetTransparency(BUTTON_TRANSPARENCY_HIGHLIGHT);
            selected = false;
            use_keyboard = false;
            if( buttons & WPAD_BUTTON_A ) {
                break;
            }
        }else
        if( !use_keyboard ) {
            yes->SetTransparency(BUTTON_TRANSPARENCY_NORMAL);
            no->SetTransparency(BUTTON_TRANSPARENCY_NORMAL);
        }

        // Check keys
        if( buttons & WPAD_BUTTON_LEFT ) {
            yes->SetTransparency(BUTTON_TRANSPARENCY_HIGHLIGHT);
            no->SetTransparency(BUTTON_TRANSPARENCY_NORMAL);
            selected = true;
            use_keyboard = true;
        }
        if( buttons & WPAD_BUTTON_RIGHT ) {
            yes->SetTransparency(BUTTON_TRANSPARENCY_NORMAL);
            no->SetTransparency(BUTTON_TRANSPARENCY_HIGHLIGHT);
            selected = false;
            use_keyboard = true;
        }
        if( use_keyboard && (buttons & WPAD_BUTTON_A) != 0 ) {
            break;
        }
        if( (buttons & (WPAD_BUTTON_HOME | WPAD_BUTTON_B)) != 0 ) {
            selected = false;
            break;
        }

        manager->Unlock();
        VIDEO_WaitVSync();
    }
    manager->Unlock();

    // Cleanup
    manager->Remove(cursor);
    delete cursor;

    return selected;
}

void GuiMessageBox::SetText(const char *fmt, ...)
{
    manager->Lock();

    va_list marker;
    va_start(marker,fmt);
    txt_image->RenderTextVA(true, fmt, marker);
    va_end(marker);

    manager->Unlock();
}

bool GuiMessageBox::Show(const char *txt, Image *image, bool yesno, int alpha)
{
    manager->Lock();

    if( is_showing ) {
        Remove();
    }

    // Prepare text
    int txtwidth;
    int textheight;
    txt_image = new DrawableImage;
    txt_image->SetFont(g_fontArial);
    txt_image->SetSize(32);
    txt_image->SetYSpacing(2);
    txt_image->SetColor((GXColor){255,255,255,255});
    txt_image->GetTextSize(&txtwidth, &textheight, txt);
    txtwidth = (txtwidth + 3) & ~3;
    textheight = (textheight + 3) & ~3;
    txt_image->CreateImage(txtwidth, textheight);
    txt_image->RenderText(true, txt);

    // container
    int minsizex = image? 560 : 400;
    int sizex = txtwidth + (image? image->GetWidth()+24 : 0) + 48;
    int sizey = 100 + textheight;
    if( sizex < minsizex ) {
        sizex = minsizex;
    }
    if( yesno ) {
        sizex += 120;
        sizey += 40;
    }
    int x = manager->GetWidth()/2-sizex/2;
    int y = manager->GetHeight()/2-sizey/2;
    container = new GuiContainer(x, y , sizex, sizey, alpha);
    manager->AddTop(container, MESSAGE_BOX_FADE_FRAMES);
    sizex = container->GetWidth();
    sizey = container->GetHeight();

    // image (optional)
    if( image ) {
        img_sprite = new Sprite;
        img_sprite->SetImage(image);
        img_sprite->SetRefPixelPosition(0, 0);
        img_sprite->SetPosition(x+24, y+sizey/2-image->GetHeight()/2);
        manager->AddTop(img_sprite, MESSAGE_BOX_FADE_FRAMES);
        x += 24+image->GetWidth();
        sizex -= 24+image->GetWidth();
    }
    // yes/no buttons (optional)
    if( yesno ) {
        int bx = x+sizex/2-g_imgButtonYes->GetWidth()-12;
        int by = y+sizey-g_imgButtonYes->GetHeight()-36;
        spr_yes = new Sprite;
        spr_yes->SetImage(g_imgButtonYes);
        spr_yes->SetPosition(bx, by);
        manager->AddTop(spr_yes, MESSAGE_BOX_FADE_FRAMES);
        bx = x+sizex/2+12;
        by = y+sizey-g_imgButtonYes->GetHeight()-36;
        spr_no = new Sprite;
        spr_no->SetImage(g_imgButtonNo);
        spr_no->SetPosition(bx, by);
        manager->AddTop(spr_no, MESSAGE_BOX_FADE_FRAMES);
        sizey -= g_imgButtonYes->GetHeight() + 36;
   }
    // text
    txt_sprite = new Sprite;
    txt_sprite->SetImage(txt_image);
    txt_sprite->SetPosition(x+(sizex-txtwidth)/2, y+(sizey-textheight)/2-4);
    manager->AddTop(txt_sprite, MESSAGE_BOX_FADE_FRAMES);
    is_showing = true;
    manager->Unlock();
    // selection
    if( yesno ) {
        return DoSelection(spr_yes, spr_no);
    }else{
        return true;
    }
}

void GuiMessageBox::Remove(void)
{
    if( is_showing ) {
        manager->Lock();
        if( txt_sprite ) {
            manager->RemoveAndDelete(txt_sprite, txt_image, MESSAGE_BOX_FADE_FRAMES);
            txt_sprite = NULL;
            txt_image = NULL;
        }
        if( spr_yes ) {
            manager->RemoveAndDelete(spr_yes, NULL, MESSAGE_BOX_FADE_FRAMES);
            spr_yes = NULL;
        }
        if( spr_no ) {
            manager->RemoveAndDelete(spr_no, NULL, MESSAGE_BOX_FADE_FRAMES);
            spr_no = NULL;
        }
        if( img_sprite ) {
            manager->RemoveAndDelete(img_sprite, NULL, MESSAGE_BOX_FADE_FRAMES);
            img_sprite = NULL;
        }
        if( container ) {
            manager->RemoveAndDelete(container, NULL, MESSAGE_BOX_FADE_FRAMES);
            container = NULL;
        }
        is_showing = false;
        manager->Unlock();
    }
}

static GuiMessageBox *myself;
static void MessageBoxPopupThreadWrapper(void)
{
    myself->MessageBoxPopupThread();
}

void GuiMessageBox::MessageBoxPopupThread(void)
{
    // Wait about 2 seconds
    for(int i = 0; !quit_thread && i < 20; i++) {
        archThreadSleep(100);
    }
    // Remove the popup
    Remove();
}

void GuiMessageBox::ShowPopup(const char *txt, Image *image, int alpha)
{
    // Terminate old thread
    if( thread_popup ) {
        quit_thread = true;
        archThreadJoin(thread_popup, -1);
        archThreadDestroy(thread_popup);
        thread_popup = NULL;
    }
    manager->Lock();
    // Show popup
    (void)Show(txt, image, false, alpha);
    // Start thread
    myself = this;
    quit_thread = false;
    thread_popup = archThreadCreateEx(MessageBoxPopupThreadWrapper, THREAD_PRIO_NORMAL, 16*1024);
    // done
    manager->Unlock();
}

GuiMessageBox::GuiMessageBox(GuiManager *man)
{
    manager = man;
    is_showing = false;
    container = NULL;
    txt_sprite = NULL;
    img_sprite = NULL;
    txt_image = NULL;
    spr_yes = NULL;
    spr_no = NULL;
    thread_popup = NULL;
}

GuiMessageBox::~GuiMessageBox()
{
    if( is_showing ) {
        Remove();
    }
    if( thread_popup ) {
        quit_thread = true;
        archThreadJoin(thread_popup, -1);
        archThreadDestroy(thread_popup);
    }
}

