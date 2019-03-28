#include <stdio.h>
#include <stdlib.h>

#include "GuiMenu.h"
#include "GuiContainer.h"

#define MENU_YPITCH      56
#define MENU_FADE_FRAMES 10

int GuiMenu::DoModal(const char **items, int num, int width)
{
    // Claim UI
    manager->Lock();

    // Add container
    int height = num_item_rows*MENU_YPITCH+(MENU_YPITCH/2);
    int posx = manager->GetWidth()/2-width/2;
    int posy = manager->GetHeight()/2-height/2;
    GuiContainer *container = new GuiContainer(posx, posy, width, height, 192);
    manager->AddTop(container, MENU_FADE_FRAMES);
    width = container->GetWidth();
    height = container->GetHeight();
    posx = manager->GetWidth()/2-width/2;
    posy = manager->GetHeight()/2-height/2;

    // Start displaying
    manager->Unlock();

    // Menu list
    ShowSelection(items, num, 0, 32, MENU_YPITCH,
                  posx+16, posy+24, 24, width-32, false, MENU_FADE_FRAMES);
    int sel = DoSelection();
    RemoveSelection();

    // Claim UI
    manager->Lock();

    // Remove container
    manager->RemoveAndDelete(container, NULL, MENU_FADE_FRAMES);

    // Release UI
    manager->Unlock();

    return sel;
}

GuiMenu::GuiMenu(GuiManager *man, int rows)
       : GuiSelectionList(man, rows)
{
    manager = man;
    num_item_rows = rows;
}

GuiMenu::~GuiMenu()
{
}

