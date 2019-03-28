
#include <stdio.h>
#include <stdlib.h>
#include <gccore.h>
#include <ogc/lwp_watchdog.h>
#include <wiiuse/wpad.h>

extern "C" {
#include "../IoDevice/Led.h"
}

#include "InputEvent.h"
#include "GuiKeyboard.h"
#include "WiiInput.h"

// Resources
#include "GuiImages.h"
#include "GuiFonts.h"

// Colors                 R            G          B
#define COLOR_HOVER   ((0x08 << 11)+(0x08 << 6)+(0x08))
#define COLOR_PRESSED ((0x00 << 11)+(0x00 << 6)+(0x1f))

#define KEYBOARD_FADE_FRAMES 30

typedef struct {
  int key;
  int x;
  int y;
  int sx;
  int sy;
  bool toggle;
  Sprite **spr;
} KEY_RECT;

static Sprite *spr_shift_l;
static Sprite *spr_shift_r;
static Sprite *spr_ctrl;
static Sprite *spr_code;
static Sprite *spr_graph;
static Sprite *spr_caps;

static KEY_RECT key_rects[] = {
    { EC_ESC,     2,    3,   61,  29, false, NULL },
    { EC_F1,      64,   3,   70,  29, false, NULL },
    { EC_F2,      135,  3,   70,  29, false, NULL },
    { EC_F3,      206,  3,   70,  29, false, NULL },
    { EC_F4,      277,  3,   70,  29, false, NULL },
    { EC_F5,      348,  3,   70,  29, false, NULL },
    { EC_SELECT,  419,  3,   70,  29, false, NULL },
    { EC_STOP,    490,  3,   70,  29, false, NULL },
    { EC_CLS,     561,  3,   70,  29, false, NULL },
    { EC_INS,     632,  3,   70,  29, false, NULL },
    { EC_DEL,     703,  3,   71,  29, false, NULL },
    { EC_RBRACK,  2,    33,  49,  51, false, NULL },
    { EC_1,       52,   33,  53,  51, false, NULL },
    { EC_2,       106,  33,  53,  51, false, NULL },
    { EC_3,       160,  33,  53,  51, false, NULL },
    { EC_4,       214,  33,  53,  51, false, NULL },
    { EC_5,       268,  33,  53,  51, false, NULL },
    { EC_6,       322,  33,  53,  51, false, NULL },
    { EC_7,       376,  33,  53,  51, false, NULL },
    { EC_8,       429,  33,  53,  51, false, NULL },
    { EC_9,       483,  33,  53,  51, false, NULL },
    { EC_0,       537,  33,  53,  51, false, NULL },
    { EC_NEG,     591,  33,  53,  51, false, NULL },
    { EC_CIRCFLX, 645,  33,  53,  51, false, NULL },
    { EC_BKSPACE, 699,  33,  75,  51, false, NULL },
    { EC_TAB,     2,    85,  75,  52, false, NULL },
    { EC_Q,       78,   85,  53,  52, false, NULL },
    { EC_W,       132,  85,  52,  52, false, NULL },
    { EC_E,       186,  85,  52,  52, false, NULL },
    { EC_R,       239,  85,  53,  52, false, NULL },
    { EC_T,       293,  85,  53,  52, false, NULL },
    { EC_Y,       347,  85,  53,  52, false, NULL },
    { EC_U,       401,  85,  53,  52, false, NULL },
    { EC_I,       455,  85,  53,  52, false, NULL },
    { EC_O,       509,  85,  52,  52, false, NULL },
    { EC_P,       563,  85,  52,  52, false, NULL },
    { EC_AT,      616,  85,  53,  52, false, NULL },
    { EC_LBRACK,  670,  85,  53,  52, false, NULL },
    { EC_BKSLASH, 724,  85,  50,  52, false, NULL },
    { EC_CTRL,    2,   138,  89,  51, true,  &spr_ctrl },
    { EC_A,       92,  138,  53,  51, false, NULL },
    { EC_S,       146, 138,  52,  51, false, NULL },
    { EC_D,       200, 138,  52,  51, false, NULL },
    { EC_F,       253, 138,  53,  51, false, NULL },
    { EC_G,       307, 138,  53,  51, false, NULL },
    { EC_H,       361, 138,  53,  51, false, NULL },
    { EC_J,       415, 138,  53,  51, false, NULL },
    { EC_K,       469, 138,  53,  51, false, NULL },
    { EC_L,       523, 138,  52,  51, false, NULL },
    { EC_SEMICOL, 577, 138,  52,  51, false, NULL },
    { EC_COLON,   630, 138,  53,  51, false, NULL },
    { EC_RETURN,  684, 138,  90,  51, false, NULL },
    { EC_LSHIFT,  2,   190, 116,  52, true,  &spr_shift_l },
    { EC_Z,       119, 190,  52,  52, false, NULL },
    { EC_X,       173, 190,  52,  52, false, NULL },
    { EC_C,       227, 190,  52,  52, false, NULL },
    { EC_V,       280, 190,  53,  52, false, NULL },
    { EC_B,       334, 190,  53,  52, false, NULL },
    { EC_N,       388, 190,  53,  52, false, NULL },
    { EC_M,       442, 190,  53,  52, false, NULL },
    { EC_COMMA,   496, 190,  53,  52, false, NULL },
    { EC_PERIOD,  550, 190,  52,  52, false, NULL },
    { EC_DIV,     604, 190,  52,  52, false, NULL },
    { EC_RSHIFT,  657, 190, 117,  52, true,  &spr_shift_r },
    { EC_CAPS,    2,   243, 103,  58, false, &spr_caps },
    { EC_GRAPH,   106, 243,  65,  58, true,  &spr_graph },
    { EC_SPACE,   173, 243, 376,  58, false, NULL },
    { EC_CODE,    550, 243,  65,  58, false, &spr_code },
    { EC_UP,      669, 243,  51,  28, false, NULL },
    { EC_LEFT,    616, 272,  52,  29, false, NULL },
    { EC_DOWN,    669, 272,  51,  29, false, NULL },
    { EC_RIGHT,   722, 272,  52,  29, false, NULL }
};
#define NUM_KEYS (sizeof(key_rects) / sizeof(key_rects[0]))

void GuiKeyboard::RenderWrapper(void *arg)
{
    GuiKeyboard *me = (GuiKeyboard*)arg;
    me->Render();
}

// Function is called every frame
void GuiKeyboard::Render(void)
{
    static KEY_RECT *key_pressed = NULL;

    // Buttons
    WPAD_ScanPads();
    u32 buttons = WPAD_ButtonsHeld(WPAD_CHAN_0) | WPAD_ButtonsHeld(WPAD_CHAN_1);

    // Infrared
    int x, y, angle;
    bool validpos = manager->GetWiiMoteIR(&x, &y, &angle);
    if( validpos && is_enabled && !is_hidden ) {
        spr_cursor->SetPosition(x, y);
        spr_cursor->SetRotation(angle/2);
        spr_cursor->SetVisible(true);
    }else{
        spr_cursor->SetVisible(false);
        spr_cursor->SetPosition(0, 0);
    }

    x = (int)((x - xpos) / xscale);
    y = (int)((y - ypos) / yscale);

    if( validpos && is_enabled &&
        x >= -40 && x <= xsize+40 &&
        y >= -70 && y <= ysize+60 ) {
        if( !is_showing && !is_hidden ) {
            Show();
        }
    }else{
        is_hidden = false;
        if( is_showing ) {
            Remove();
        }
    }
    if( !validpos || !is_showing ) {
        return;
    }

    spr_hover->SetVisible(false);
    spr_pressed->SetVisible(false);

    // Loop all keys
    for(unsigned i = 0; i < NUM_KEYS; i++) {
        KEY_RECT *key = &key_rects[i];
        Sprite *spr = NULL;
        if( key->spr ) {
            spr = *key->spr;
        }
        // Check if above key
        if( x >= key->x && x < key->x + key->sx &&
            y >= key->y && y < key->y + key->sy )
        {
            // update hover sprite
            spr_hover->SetStretchWidth(((float)key->sx / 4) * xscale);
            spr_hover->SetStretchHeight(((float)key->sy / 4) * yscale);
            spr_hover->SetRefPixelPositioning(REFPIXEL_POS_PIXEL);
            spr_hover->SetRefPixelPosition(0,0);
            spr_hover->SetPosition(xscale * key->x + xpos, yscale * key->y + ypos);
            spr_hover->SetTransparency(128);
            spr_hover->SetVisible(true);
            // unset previous pressed key
            if( key_pressed && key != key_pressed && !key_pressed->toggle ) {
                inputEventUnset(key_pressed->key);
            }
            // set/unset/toggle current key
            if( buttons & (WPAD_BUTTON_A | WPAD_CLASSIC_BUTTON_A) ) {
                if( key->toggle ) {
                    if( key != key_pressed ) {
                        if( inputEventGetState(key->key) ) {
                            inputEventUnset(key->key);
                        }else{
                            inputEventSet(key->key);
                        }
                    }
                }else{
                    inputEventSet(key->key);
                }
                if( spr == NULL ) {
                    spr = spr_pressed;
                }
                key_pressed = key;
            }else{
                if( key_pressed && !key_pressed->toggle ) {
                    inputEventUnset(key_pressed->key);
                }
                key_pressed = NULL;
            }
        }
        // set/unset pressed key
        if( spr ) {
            spr->SetStretchWidth(((float)key->sx / 4) * xscale);
            spr->SetStretchHeight(((float)key->sy / 4) * yscale);
            spr->SetRefPixelPositioning(REFPIXEL_POS_PIXEL);
            spr->SetRefPixelPosition(0,0);
            spr->SetPosition(xscale * key->x + xpos, yscale * key->y + ypos);
            spr->SetTransparency(128);
            if( key->key == EC_CAPS ) {
                spr->SetVisible(ledGetCapslock());
            }else{
                spr->SetVisible(inputEventGetState(key->key));
            }
        }
    }
}

void GuiKeyboard::Show(void)
{
    if( is_showing ) {
        return;
    }

    // Keyboard image
    spr_image = new Sprite;
    spr_image->SetImage(g_imgKeyboard);
    spr_image->SetStretchWidth(xscale);
    spr_image->SetStretchHeight(yscale);
    spr_image->SetRefPixelPositioning(REFPIXEL_POS_PIXEL);
    spr_image->SetRefPixelPosition(0,0);
    spr_image->SetPosition(xpos, ypos);
    spr_image->SetTransparency(256-32);

    img_hover = new DrawableImage;
    img_hover->CreateImage(4, 4, GX_TF_RGB565);
    u16 *p1 = (u16*)img_hover->GetTextureBuffer();
    img_pressed = new DrawableImage;
    img_pressed->CreateImage(4, 4, GX_TF_RGB565);
    u16 *p2 = (u16*)img_pressed->GetTextureBuffer();
    for(int i = 0; i < 4*4; i++) {
        *p1++ = COLOR_HOVER;
        *p2++ = COLOR_PRESSED;
    }
    img_hover->FlushBuffer();
    img_pressed->FlushBuffer();
    spr_hover = new Sprite;
    spr_hover->SetImage(img_hover->GetImage());
    spr_pressed = new Sprite;
    spr_pressed->SetImage(img_pressed->GetImage());
    spr_caps = new Sprite;
    spr_caps->SetImage(img_pressed->GetImage());
    spr_shift_l = new Sprite;
    spr_shift_l->SetImage(img_pressed->GetImage());
    spr_shift_r = new Sprite;
    spr_shift_r->SetImage(img_pressed->GetImage());
    spr_ctrl = new Sprite;
    spr_ctrl->SetImage(img_pressed->GetImage());
    spr_graph = new Sprite;
    spr_graph->SetImage(img_pressed->GetImage());
    spr_code = new Sprite;
    spr_code->SetImage(img_pressed->GetImage());

    keymap1 = keyboardGetMapping(KEY_JOY1_WIIMOTE_A);
    keymap2 = keyboardGetMapping(KEY_JOY2_WIIMOTE_A);
    keyboardRemapKey(KEY_JOY1_WIIMOTE_A, EC_NONE);
    keyboardRemapKey(KEY_JOY2_WIIMOTE_A, EC_NONE);

    manager->Lock();

    // Keyboard image
    manager->AddTop(spr_image, KEYBOARD_FADE_FRAMES);

    // Selectors
    manager->AddTop(spr_hover, KEYBOARD_FADE_FRAMES);
    spr_hover->SetVisible(false);
    manager->AddTop(spr_caps, KEYBOARD_FADE_FRAMES);
    spr_caps->SetVisible(false);
    manager->AddTop(spr_pressed, KEYBOARD_FADE_FRAMES);
    spr_pressed->SetVisible(false);
    manager->AddTop(spr_shift_l, KEYBOARD_FADE_FRAMES);
    spr_shift_l->SetVisible(false);
    manager->AddTop(spr_shift_r, KEYBOARD_FADE_FRAMES);
    spr_shift_r->SetVisible(false);
    manager->AddTop(spr_ctrl, KEYBOARD_FADE_FRAMES);
    spr_ctrl->SetVisible(false);
    manager->AddTop(spr_graph, KEYBOARD_FADE_FRAMES);
    spr_graph->SetVisible(false);
    manager->AddTop(spr_code, KEYBOARD_FADE_FRAMES);
    spr_code->SetVisible(false);

    // Cursor
    spr_cursor->SetVisible(false);
    manager->AddTopFixed(spr_cursor);

    is_showing = true;
    manager->Unlock();
}

void GuiKeyboard::Remove(void)
{
    if( !is_showing ) {
        return;
    }

    manager->Lock();

    manager->RemoveAndDelete(spr_code, NULL, KEYBOARD_FADE_FRAMES);
    manager->RemoveAndDelete(spr_graph, NULL, KEYBOARD_FADE_FRAMES);
    manager->RemoveAndDelete(spr_ctrl, NULL, KEYBOARD_FADE_FRAMES);
    manager->RemoveAndDelete(spr_shift_l, NULL, KEYBOARD_FADE_FRAMES);
    manager->RemoveAndDelete(spr_shift_r, NULL, KEYBOARD_FADE_FRAMES);
    manager->RemoveAndDelete(spr_caps, NULL, KEYBOARD_FADE_FRAMES);
    manager->RemoveAndDelete(spr_pressed, img_pressed, KEYBOARD_FADE_FRAMES);
    manager->RemoveAndDelete(spr_hover, img_hover, KEYBOARD_FADE_FRAMES);
    manager->RemoveAndDelete(spr_image, NULL, KEYBOARD_FADE_FRAMES);
    is_showing = false;
    manager->Unlock();

    keyboardRemapKey(KEY_JOY1_WIIMOTE_A, keymap1);
    keyboardRemapKey(KEY_JOY2_WIIMOTE_A, keymap2);
}

bool GuiKeyboard::IsShowing(void)
{
    return is_showing;
}

void GuiKeyboard::SetEnabled(bool enable)
{
    is_enabled = is_hidden = enable;
}

GuiKeyboard::GuiKeyboard(GuiManager *man)
{
    manager = man;
    is_showing = false;
    is_enabled = true;
    is_hidden = true;
    xscale = 0.8f;
    yscale = 1.0f;
    xsize = g_imgKeyboard->GetWidth();
    ysize = g_imgKeyboard->GetHeight();
    xpos = 10;
    ypos = (manager->GetHeight()-ysize)/2;

    // Cursor
    spr_cursor = new Sprite;
    spr_cursor->SetImage(g_imgMousecursor);
    spr_cursor->SetRefPixelPositioning(REFPIXEL_POS_PIXEL);
    spr_cursor->SetRefPixelPosition(11, 4);
    spr_cursor->SetPosition(0, 0);

    manager->Lock();
    manager->AddRenderCallback(RenderWrapper, (void*)this);
    manager->Unlock();
}

GuiKeyboard::~GuiKeyboard()
{
    manager->Lock();
    manager->RemoveRenderCallback(RenderWrapper, (void*)this);
    manager->RemoveAndDelete(spr_cursor);
    manager->Unlock();
    if( is_showing ) {
        Remove();
    }
}

