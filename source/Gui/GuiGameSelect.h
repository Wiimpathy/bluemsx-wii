
#ifndef _GUI_GAME_SELECT_H
#define _GUI_GAME_SELECT_H

#include "GuiSelectionList.h"
#include "GameElement.h"
#include "GameList.h"

#define NUM_LIST_ITEMS   12

class GuiGameSelect : public GuiSelectionList {
public:
    GuiGameSelect(GuiManager *man);
    virtual ~GuiGameSelect();

    void OnSetSelected(int index, int selected);
    bool Load(const char *dir, const char *filename);
    GameElement *DoModal(GameElement *select = NULL, char *filename = NULL);
private:
    GameList games;
    int num_games;
    const char **title_list;
    Sprite *sprScreenShot[2];

    void SetScreenShotImage(int index, Image *img);
};

#endif

