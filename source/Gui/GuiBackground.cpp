
#include "GuiBackground.h"
#include "GuiFonts.h"
#include "GuiImages.h"
#include "version.h"

#define BACKGROUND_FADE_FRAMES 10

GuiBackground::GuiBackground(GuiManager *man)
{
    manager = man;
    is_shown = false;
}

GuiBackground::~GuiBackground()
{
    Hide();
}

void GuiBackground::Show(void)
{
    int txtwidth;
    int txtheight;

    if( !is_shown ) {
        manager->Lock();

        // Background picture
        sprBackground = new Sprite;
        sprBackground->SetImage(g_imgBackground);
        sprBackground->SetStretchWidth((float)manager->GetWidth() /
                                       (float)g_imgBackground->GetWidth());
        sprBackground->SetStretchHeight((float)manager->GetHeight() /
                                        (float)g_imgBackground->GetHeight());
        sprBackground->SetRefPixelPositioning(REFPIXEL_POS_PIXEL);
        sprBackground->SetRefPixelPosition(0, 0);
        sprBackground->SetPosition(0, 0);

        // Create image with version text
        imgTxt = new DrawableImage;
        imgTxt->SetFont(g_fontArial);
        imgTxt->SetSize(16);
        imgTxt->SetYSpacing(2);
        imgTxt->SetColor((GXColor){255,255,255,255});
        imgTxt->GetTextSize(&txtwidth, &txtheight, VERSION_AS_STRING);
        txtwidth = (txtwidth + 3) & ~3;
        txtheight = (txtheight + 3) & ~3;
        imgTxt->CreateImage(txtwidth, txtheight);
        imgTxt->RenderText(true, VERSION_AS_STRING);

        // Version text sprite
        sprTxt = new Sprite;
        sprTxt->SetImage(imgTxt);
        sprTxt->SetPosition(530, 380);
        sprTxt->SetTransparency(192);

        // Show it
        manager->AddBottom(sprBackground, BACKGROUND_FADE_FRAMES);
        manager->AddOnTopOf(sprBackground, sprTxt, BACKGROUND_FADE_FRAMES);

        manager->Unlock();
        is_shown = true;
    }
}

void GuiBackground::Hide(void)
{
    if( is_shown ) {
        manager->Lock();
        manager->RemoveAndDelete(sprBackground, NULL, BACKGROUND_FADE_FRAMES);
        manager->RemoveAndDelete(sprTxt, imgTxt, BACKGROUND_FADE_FRAMES);
        manager->Unlock();
        is_shown = false;
    }
}

