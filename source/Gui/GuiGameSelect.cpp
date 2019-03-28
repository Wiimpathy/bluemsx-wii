#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "GuiGameSelect.h"
#include "GuiContainer.h"
#include "GuiMessageBox.h"

// Resources
#include "GuiImages.h"

#define GAMESEL_FADE_DEFAULT      10
#define GAMESEL_FADE_SCREENSHOTS  10
#define GAMESEL_DELAY_SCREENSHOTS 8

void GuiGameSelect::SetScreenShotImage(int index, Image *img)
{
    if( img == NULL ) {
        img = g_imgNoise;
    }
    Sprite *spr = sprScreenShot[index];
    spr->SetImage(img);
    spr->SetRefPixelPositioning(REFPIXEL_POS_PIXEL);
    spr->SetRefPixelPosition(0, 0);
    spr->SetStretchWidth(252.0f/img->GetWidth());
    spr->SetStretchHeight(168.0f/img->GetHeight());
}

void GuiGameSelect::OnSetSelected(int index, int selected)
{
    bool first = true;
    // (Re)create screenshot sprites
    if( sprScreenShot[0] != NULL ) {
        first = false;
        manager->RemoveAndDelete(sprScreenShot[0], sprScreenShot[0]->GetImage(),
                                 GAMESEL_FADE_SCREENSHOTS, GAMESEL_DELAY_SCREENSHOTS);
    }
    if( sprScreenShot[1] != NULL ) {
        first = false;
        manager->RemoveAndDelete(sprScreenShot[1], sprScreenShot[1]->GetImage(),
                                 GAMESEL_FADE_SCREENSHOTS, GAMESEL_DELAY_SCREENSHOTS);
    }
    sprScreenShot[0] = new Sprite;
    sprScreenShot[0]->SetPosition(344+12-8, 18+16);
    sprScreenShot[1] = new Sprite;
    sprScreenShot[1]->SetPosition(344+12-8, 222+16);
    // Update screenshots
    if( selected >= 0 ) {
        GameElement *game = games.GetGame(index+selected);
        SetScreenShotImage(0, new Image(game->GetImage(0)));
        SetScreenShotImage(1, new Image(game->GetImage(1)));
    }
    // Add to screen
    manager->AddTop(sprScreenShot[0], first? GAMESEL_FADE_DEFAULT : GAMESEL_FADE_SCREENSHOTS);
    manager->AddTop(sprScreenShot[1], first? GAMESEL_FADE_DEFAULT : GAMESEL_FADE_SCREENSHOTS);
    // Free images of games that are not on the screen
    for(int i = 0; i < games.GetNumberOfGames(); i++) {
        GameElement *game = games.GetGame(i);
        if( game != NULL && (i < index || i >= (index + NUM_LIST_ITEMS)) ) {
            game->FreeImage(0);
            game->FreeImage(1);
        }
    }
}

bool GuiGameSelect::Load(const char *dir, const char *filename)
{
    // Load games database
    chdir(dir);
    games.Load(filename);
    num_games = games.GetNumberOfGames();
    if( num_games == 0 ) {
        return false;
    }
    title_list = (const char**)malloc(num_games * sizeof(const char*));
    for(int i = 0; i < num_games; i++) {
        title_list[i] = games.GetGame(i)->GetName();
    }
    return true;
}

GameElement *GuiGameSelect::DoModal(GameElement *select, char *filename)
{
    GameElement *returnValue = NULL;

		// Auto boot file with command line values
		if(filename)
		{
				for( int i = 0; i < num_games; i++ ) 
				{
						if(strstr(games.GetGame(i)->GetCommandLine(), filename))
						{
				        returnValue = games.GetGame(i);

								if( returnValue != NULL ) {
										GameElement *game = new GameElement(returnValue);
										games.Clear();
										return game;
								}else{
										games.Clear();
										return NULL;
								}
						}

				}
		}

    // Claim UI
    manager->Lock();

    // Containers
    GuiContainer *grWinList = new GuiContainer(32-8, 22, 288, 33*12);
    manager->AddTop(grWinList, GAMESEL_FADE_DEFAULT);
    GuiContainer *grWinTitle = new GuiContainer(344-8, 22, 264+12, 16*12);
    manager->AddTop(grWinTitle, GAMESEL_FADE_DEFAULT);
    GuiContainer *grWinPlay = new GuiContainer(344-8, 226, 264+12, 16*12);
    manager->AddTop(grWinPlay, GAMESEL_FADE_DEFAULT);

    // On re-enter, find selected entry
    int sel = 0;
    if( select != NULL ) {
        for( int i = 0; i < num_games; i++ ) {
            if( strcmp(select->GetName(), title_list[i])==0 ) {
                sel = i;
                break;
            }
        }
    }

    // Initialize screenshot variables
    sprScreenShot[0] = NULL;
    sprScreenShot[1] = NULL;

    // Add selection list
    ShowSelection(title_list, num_games, sel, 22, 31,
                  30, 32, 12, 264+12, false, GAMESEL_FADE_DEFAULT);

    // Release UI
    manager->Unlock();

    // Menu loop
    do {
        sel = DoSelection();

        if( sel >= 0 ) {
            returnValue = games.GetGame(sel);
            // confirmation
            char str[256];
            strcpy(str, "Do you want to start\n\"");
            strcat(str, returnValue->GetName());
            strcat(str, "\"");
            GuiMessageBox *msgbox = new GuiMessageBox(manager);
            bool ok = msgbox->Show(str, NULL, true, 192);
            msgbox->Remove();
            delete msgbox;
            if( ok ) {
                break;
            }
        }else{
            returnValue = NULL;
        }
    }while(sel >= 0);

    // Claim UI
    manager->Lock();

    // Remove UI elements
    RemoveSelection();
    manager->RemoveAndDelete(sprScreenShot[0], sprScreenShot[0]->GetImage(), GAMESEL_FADE_DEFAULT);
    manager->RemoveAndDelete(sprScreenShot[1], sprScreenShot[1]->GetImage(), GAMESEL_FADE_DEFAULT);
    manager->RemoveAndDelete(grWinList, NULL, GAMESEL_FADE_DEFAULT);
    manager->RemoveAndDelete(grWinTitle, NULL, GAMESEL_FADE_DEFAULT);
    manager->RemoveAndDelete(grWinPlay, NULL, GAMESEL_FADE_DEFAULT);

    // Release UI
    manager->Unlock();

    if( returnValue != NULL ) {
        GameElement *game = new GameElement(returnValue);
        games.Clear();
        return game;
    }else{
        games.Clear();
        return NULL;
    }
}

GuiGameSelect::GuiGameSelect(GuiManager *man) : GuiSelectionList(man, NUM_LIST_ITEMS)
{
    manager = man;
    title_list = NULL;
}

GuiGameSelect::~GuiGameSelect()
{
    if( title_list ) {
        free(title_list);
    }
}

