#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <gccore.h>
#include <ogc/lwp_watchdog.h>

#include <fat.h>
#include <wiiuse/wpad.h>

#include "kbdlib.h"
#include "GuiSelectionList.h"
#include "GuiContainer.h"

// Resources
#include "GuiImages.h"
#include "GuiFonts.h"

#define RUMBLE 0
#define SCROLL_TIME 500
#define REPEAT_TIME 200

#define SELECTION_FADE_TIME  10
#define SELECTION_FADE_DELAY 4

void GuiSelectionList::InitTitleList(TextRender *fontArial,
                                     int x, int y, int width, int ypitch)
{
    // Fill titles
    for(int i = 0, yy = y; i < num_item_rows; i++) {
        DrawableImage *img = new DrawableImage;
        img->CreateImage((width + 3) & ~3, ((fontsize * 3) / 2) & ~3);
        img->SetFont(fontArial);
        img->SetColor((GXColor){255,255,255,255});
        img->SetSize(fontsize);
        titleTxtImgPtr[i] = img;
        titleTxtSprite[i] = new Sprite;
        titleTxtSprite[i]->SetImage(img);
        titleTxtSprite[i]->SetPosition(x + xspacing, yy + fontsize/5);
        manager->AddTop(titleTxtSprite[i], fade);
        yy += ypitch;
    }

    // Arrows
    sprArrowUp = new Sprite;
    sprArrowUp->SetImage(g_imgArrow);
    sprArrowUp->SetRefPixelPositioning(REFPIXEL_POS_PIXEL);
    sprArrowUp->SetRefPixelPosition(g_imgArrow->GetWidth()/2, g_imgArrow->GetHeight()/2);
    sprArrowUp->SetPosition(x + xspacing + width/2, y+ypitch/2+fontsize/5);
    sprArrowUp->SetStretchWidth((float)(width/2)/g_imgArrow->GetWidth());
    sprArrowUp->SetStretchHeight(((float)fontsize/g_imgArrow->GetHeight())*0.8f);
    sprArrowUp->SetRotation(180.0f/2);
    sprArrowUp->SetVisible(false);
    manager->AddTop(sprArrowUp, fade);

    sprArrowDown = new Sprite;
    sprArrowDown->SetImage(g_imgArrow);
    sprArrowDown->SetRefPixelPositioning(REFPIXEL_POS_PIXEL);
    sprArrowUp->SetRefPixelPosition(g_imgArrow->GetWidth()/2, g_imgArrow->GetHeight()/2);
    sprArrowDown->SetPosition(x + xspacing + width/2, y+(num_item_rows-1)*ypitch+ypitch/2+fontsize/5);
    sprArrowDown->SetStretchWidth((float)(width/2)/g_imgArrow->GetWidth());
    sprArrowDown->SetStretchHeight(((float)fontsize/g_imgArrow->GetHeight())*0.8f);
    sprArrowDown->SetVisible(false);
    manager->AddTop(sprArrowDown, fade);

    current_index = -1;
}

void GuiSelectionList::RemoveTitleList(void)
{
    manager->RemoveAndDelete(sprArrowUp, NULL, fade);
    manager->RemoveAndDelete(sprArrowDown, NULL, fade);
    for(int i = 0; i < num_item_rows; i++) {
        manager->RemoveAndDelete(titleTxtSprite[i], titleTxtSprite[i]->GetImage(), fade);
    }
}

void GuiSelectionList::ClearTitleList(void)
{
    current_index = -1;
}

void GuiSelectionList::SetSelected(int index, int selected)
{
    // Claim UI
    manager->Lock();
    // Update dir info
    for(int i = 0; i < num_item_rows; i++) {
        if( i+index < num_items ) {
            visible_items[i] = item_list[i+index];
        }else{
            visible_items[i] = "";
        }
    }
    // Render text (slow)
    if( index == current_index+1 && current_index != -1 ) {
        DrawableImage *p = titleTxtImgPtr[0];
        for(int i = 0; i < num_item_rows-1; i++) {
            titleTxtImgPtr[i] = titleTxtImgPtr[i+1];
        }
        titleTxtImgPtr[num_item_rows-1] = p;
        titleTxtImgPtr[num_item_rows-1]->RenderText(center, visible_items[num_item_rows-1]);
    }else
    if( index == current_index-1 ) {
        DrawableImage *p = titleTxtImgPtr[num_item_rows-1];
        for(int i = num_item_rows-1; i > 0; i--) {
            titleTxtImgPtr[i] = titleTxtImgPtr[i-1];
        }
        titleTxtImgPtr[0] = p;
        titleTxtImgPtr[0]->RenderText(center, visible_items[0]);
    }else
    for(int i = 0; i < num_item_rows; i++) {
        titleTxtImgPtr[i]->RenderText(center, visible_items[i]);
    }
    current_index = index;
    // Update sprites
    for(int i = 0; i < num_item_rows; i++) {
        titleTxtSprite[i]->SetImage(titleTxtImgPtr[i]->GetImage());
    }
    // Up button
    if( index > 0  ) {
        titleTxtSprite[0]->SetVisible(false);
        sprArrowUp->SetVisible(true);
        upper_index = 1;
    }else{
        titleTxtSprite[0]->SetVisible(true);
        sprArrowUp->SetVisible(false);
        upper_index = 0;
    }
    // Down button
    if( index+num_item_rows < num_items ) {
        titleTxtSprite[num_item_rows-1]->SetVisible(false);
        sprArrowDown->SetVisible(true);
        lower_index = num_item_rows-2;
    }else{
        titleTxtSprite[num_item_rows-1]->SetVisible(true);
        sprArrowDown->SetVisible(false);
        lower_index = num_item_rows-1;
    }
    // Update seletion
    if( sprSelector != NULL ) {
        manager->RemoveAndDelete(sprSelector, NULL, SELECTION_FADE_TIME, SELECTION_FADE_DELAY);
        sprSelector = NULL;
    }
    if( selected >= 0 ) {
        Sprite *selectedsprite = titleTxtSprite[selected];
        sprSelector = new Sprite;
        sprSelector->SetImage(g_imgSelector);
        sprSelector->SetRefPixelPositioning(REFPIXEL_POS_PIXEL);
        sprSelector->SetRefPixelPosition(0, 0);
        sprSelector->SetPosition(selectedsprite->GetX()-xspacing,selectedsprite->GetY()-fontsize/5);
        sprSelector->SetStretchWidth((float)xsize / 4);
        sprSelector->SetStretchHeight((float)fontsize * 1.8f / 44);
        manager->AddBehind(selectedsprite, sprSelector, SELECTION_FADE_TIME);
    }
    // Call hook
    OnSetSelected(index, selected);
    // Release UI
    manager->Unlock();
}


void GuiSelectionList::ShowSelection(const char **items, int num, int select, int fontsz, int ypitch,
                                     int posx, int posy, int xspace, int width, bool centr, int fad)
{
#if RUMBLE
    u64 time2rumble = 0;
    bool rumbeling = false;
#endif

    if( is_showing ) {
        return;
    }

    sprSelector = NULL;
    xsize = width;
    xspacing = xspace;
    fontsize = fontsz;
    center = centr;
    fade = fad;

    // Init items
    item_list = items;
    num_items = num;

    // Claim UI
    manager->Lock();

    // Menu list
    InitTitleList(g_fontArial, posx, posy, width-2*xspace, ypitch);

    // Cursor
    sprCursor = new Sprite;
    sprCursor->SetImage(g_imgMousecursor);
    sprCursor->SetPosition(0, 0);
    sprCursor->SetVisible(false);
    manager->AddTop(sprCursor);

    // Start displaying
    manager->Unlock();

    // Selected initial entry
    if( select ) {
        if( select < num_item_rows-1 ) {
            index = 0;
            selected = select;
        }else{
            index = select-1;
            selected = 1;
            if( index + num_item_rows - num_items > 0 ) {
                selected += index + num_item_rows - num_items;
                index -= index + num_item_rows - num_items;
            }
        }
    }else{
        selected = 0;
        index = 0;
    }
    current = -1;

    // Update title list
    ClearTitleList();
    SetSelected(index, selected);

    is_showing = true;
}

int GuiSelectionList::DoSelection(void)
{
    // Menu loop
    u64 scroll_time = 0;
    (void)KBD_GetPadButtons(); // flush first
    current = selected;
    for(;;) {
        u32 buttons = KBD_GetPadButtons();

        // Exit on 'home' or 'B'
        if( buttons & (WPAD_BUTTON_HOME | WPAD_BUTTON_B) ) {
            sprCursor->SetVisible(false);
            return -1;
        }

        // Claim UI
        manager->Lock();

        // Infrared
        int x, y, angle;
        if( manager->GetWiiMoteIR(&x, &y, &angle) ) {
            sprCursor->SetPosition(x, y);
            sprCursor->SetRotation(angle/2);
            sprCursor->SetVisible(true);
        }else{
            sprCursor->SetVisible(false);
            sprCursor->SetPosition(0, 0);
        }

        // Check mouse cursor colisions
        int cursor_visible = false;
        for(int i = 0; i < num_item_rows; i++) {
            if( strlen(visible_items[i]) &&
                sprCursor->CollidesWith(titleTxtSprite[i]) ) {
                cursor_visible = true;
                selected = i;
                manager->Unlock();
                break;
            }
        }
        if( selected == current ) {
            // Scroll when mouse stays on the arrows for a while
            if( cursor_visible && (selected == 0 || selected == num_item_rows-1) ) {
                if( ticks_to_millisecs(gettime()) > scroll_time ) {
                    if( selected == 0 ) {
                        buttons |= WPAD_BUTTON_UP;
                    }else{
                        buttons |= WPAD_BUTTON_DOWN;
                    }
                    scroll_time = ticks_to_millisecs(gettime()) + REPEAT_TIME;
                }
            }else{
                scroll_time = ticks_to_millisecs(gettime()) + SCROLL_TIME;
            }

            // WPAD keys
            if( buttons & WPAD_BUTTON_DOWN ) {
                if( current < lower_index &&
                    strlen(visible_items[current+1]) ) {
                    selected++;
                }else{
                    if( index+current < num_items-1 ) {
                        index++;
                        SetSelected(index, selected);
                    }
                }
            }
            if( buttons & WPAD_BUTTON_UP ) {
                if( current > upper_index ) {
                    selected--;
                }else{
                    if( index > 0 ) {
                        index--;
                        SetSelected(index, selected);
                    }
                }
            }
        }
        if( selected != current ) {
            SetSelected(index, selected);
#if RUMBLE
            if( selected >= 0 && !rumbeling ) {
                time2rumble = ticks_to_millisecs(gettime());
                WPAD_Rumble(0,1);
                rumbeling = true;
            }
#endif
            current = selected;
            scroll_time = ticks_to_millisecs(gettime()) + SCROLL_TIME;
        }

#if RUMBLE
        //stop rumble after 50ms
        if(ticks_to_millisecs(gettime())>time2rumble+50 && rumbeling){ WPAD_Rumble(0,0); }
        //let it rumble again after 250ms
        if(ticks_to_millisecs(gettime())>time2rumble+250 && rumbeling){ rumbeling = false; }
#endif

        // Release UI
        manager->Unlock();

        if( (buttons & WPAD_BUTTON_A) &&
            (selected >= upper_index && selected <= lower_index) ) {
            break;
        }

        // wait a frame
        VIDEO_WaitVSync();
    }
    sprCursor->SetVisible(false);
#if RUMBLE
    if( rumbeling ) {
        WPAD_Rumble(0,0);
    }
#endif
    return index+selected;
}

void GuiSelectionList::RemoveSelection(void)
{
    if( !is_showing ) {
        return;
    }

    // Claim UI
    manager->Lock();

    manager->RemoveAndDelete(sprCursor);
    RemoveTitleList();
    manager->RemoveAndDelete(sprSelector, NULL, fade*2);

    // Release UI
    manager->Unlock();

    is_showing = false;
}

bool GuiSelectionList::IsShowing(void)
{
    return is_showing;
}

GuiSelectionList::GuiSelectionList(GuiManager *man, int rows)
{
    manager = man;
    num_item_rows = rows;
    is_showing = false;
    visible_items = new const char*[rows];
    titleTxtSprite = new Sprite*[rows];
    titleTxtImgPtr = new DrawableImage*[rows];
}

GuiSelectionList::~GuiSelectionList()
{
    if( is_showing ) {
        RemoveSelection();
    }
    delete[] titleTxtImgPtr;
    delete[] titleTxtSprite;
    delete[] visible_items;
}

