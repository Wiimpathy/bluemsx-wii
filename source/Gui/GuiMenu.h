
#ifndef _GUI_MENU_H
#define _GUI_MENU_H

#include "GuiSelectionList.h"

class GuiMenu : public GuiSelectionList {
public:
    GuiMenu(GuiManager *man, int rows);
    virtual ~GuiMenu();

    int DoModal(const char **items, int num, int width);
private:
    int num_item_rows;
};

#endif


