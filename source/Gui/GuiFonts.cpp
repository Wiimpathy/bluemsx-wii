
#include "GuiFonts.h"
#include "font_arial.inc"

TextRender *g_fontArial = NULL;

void GuiFontInit(void)
{
    g_fontArial = new TextRender;
    g_fontArial->SetFont(font_arial, sizeof(font_arial));
}

void GuiFontClose(void)
{
    delete g_fontArial;
    g_fontArial = NULL;
}

