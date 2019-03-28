#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>

#include "GuiStateSelect.h"
#include "GuiContainer.h"
#include "GuiMessageBox.h"

extern "C" {
#include "ArchGlob.h"
#include "ArchThread.h"
#include "FileHistory.h"
#include "ziphelper.h"
}

// Resources
#include "GuiImages.h"

#define SSEL_FADE_FRAMES  10
#define SSEL_FADE_DELAY   8

#define SSEL_YPITCH       51
#define SSEL_HEIGHT       256
#define SSEL_MENU_SPACING 8
#define SSEL_X_SPACING    12
#define SSEL_LIST_WIDTH   280

void GuiStateSelect::CreateStateFileList(Properties *properties, char *directory)
{
    ArchGlob* glob;
    static char filename[512];
    char baseName[128];
    int numMod = 1;
    char filenameFormat[32] = "%s/%s%s_";
    char destfileFormat[32];
    int digits = 2;
    const char *prefix = "";
    const char *extension = ".sta";

    for (int i = 0; i < digits; i++) {
        strcat(filenameFormat, "?");
        numMod *= 10;
    }
    strcat(filenameFormat, "%s");
    sprintf(destfileFormat, "%%s/%%s%%s_%%0%di%%s", digits);

    createSaveFileBaseName(baseName, properties, 0);

    sprintf(filename, filenameFormat, directory, prefix, baseName, extension);

    glob = archGlob(filename, ARCH_GLOB_FILES);

    // Free old list if exists
    FreeStateFileList();

    if (glob) {
        if (glob->count > 0) {
            for (int i = 0; i < glob->count; i++) {
                struct stat s;
                if( stat(glob->pathVector[i], &s) >= 0 ) {
                    int j, k;
                    time_t t = s.st_mtime;
                    // libogc bug: year = 1929
                    struct tm * timeinfo;
                    timeinfo = localtime(&t);
                    timeinfo->tm_year += 80;
                    t = mktime(timeinfo);
                    // get date/time string and strip off milliseconds
                    char *str = strdup(ctime(&t));
                    char *p = &str[strlen(str)-1];
                    while( *p != ' ' ) p--;
                    *p = '\0';
                    // to get sorted list, find position where to store in list
                    for(k = 0; k < num_states && filetimes[k] > t; k++);
                    // shift all entries behind
                    for(j = num_states; j > k; j--) {
                        filetimes[j] = filetimes[j-1];
                        timestrings[j] = timestrings[j-1];
                        filenames[j] = filenames[j-1];
                    }
                    // store in list
                    filetimes[k] = t;
                    timestrings[k] = str;
                    filenames[k] = strdup(glob->pathVector[i]);
                    num_states++;
                }
            }
        }
        archGlobFree(glob);
    }
}

void GuiStateSelect::FreeStateFileList(void)
{
    for(int i = 0; i < num_states; i++) {
        free(timestrings[i]);
        timestrings[i] = NULL;
        free(filenames[i]);
        filenames[i] = NULL;
    }
    num_states = 0;
}

void GuiStateSelect::UpdateScreenShot(char *file)
{
    if( sprScreenShot != NULL ) {
        manager->RemoveAndDelete(sprScreenShot, imgScreenShot,
                                 SSEL_FADE_FRAMES, file? SSEL_FADE_DELAY:0);
        sprScreenShot = NULL;
    }
    if( file != NULL ) {
        int size;
        void* buffer = zipLoadFile(file, "screenshot.png", &size);
        imgScreenShot = new Image;
        if( buffer != NULL ) {
            if(imgScreenShot->LoadImage((const unsigned char*)buffer) != IMG_LOAD_ERROR_NONE) {
                free(buffer);
                buffer = NULL;
            }else{
                free(buffer);
            }
        }
        if( buffer == NULL ) {
            delete imgScreenShot;
            imgScreenShot = new Image(g_imgNoise);
        }

        sprScreenShot = new Sprite;
        sprScreenShot->SetPosition(posx+sizex-283-SSEL_X_SPACING-2*SSEL_MENU_SPACING,
                                   posy+sizey/2-106);
        sprScreenShot->SetImage(imgScreenShot);
        sprScreenShot->SetRefPixelPositioning(REFPIXEL_POS_PIXEL);
        sprScreenShot->SetRefPixelPosition(0, 0);
        sprScreenShot->SetStretchWidth(283.0f/imgScreenShot->GetWidth());
        sprScreenShot->SetStretchHeight(212.0f/imgScreenShot->GetHeight());
        manager->AddTop(sprScreenShot, SSEL_FADE_FRAMES);
    }
}

void GuiStateSelect::OnSetSelected(int index, int selected)
{
    // Update screenshot
    if( selected >= 0 ) {
        UpdateScreenShot(filenames[index+selected]);
    }
}

char *GuiStateSelect::DoModal(Properties *properties, char *directory)
{
    char *returnValue = NULL;

    // Load states
    CreateStateFileList(properties, directory);
    if( num_states == 0 ) {
        GuiMessageBox *msgbox = new GuiMessageBox(manager);
        msgbox->Show("No state files found!", NULL, false, 160);
        archThreadSleep(1000);
        delete msgbox;
        return NULL;
    }

    // Claim UI
    manager->Lock();

    // Container
    posx = 14;
    posy = manager->GetHeight()/2-(SSEL_HEIGHT/2)-16;
    sizex = 640-28;
    sizey = SSEL_HEIGHT+32;
    GuiContainer *container = new GuiContainer(posx, posy, sizex, sizey, 160);
    manager->AddTop(container, SSEL_FADE_FRAMES);
    sizex = container->GetWidth();
    sizey = container->GetHeight();

    // Selection
    ShowSelection((const char **)timestrings, num_states, 0, 26, SSEL_YPITCH,
                  posx+SSEL_X_SPACING,
                  posy+sizey/2-(NUM_STATE_ITEMS*SSEL_YPITCH)/2,
                  SSEL_MENU_SPACING, SSEL_LIST_WIDTH, false, SSEL_FADE_FRAMES);

    // Release UI
    manager->Unlock();

    // Menu loop
    int sel;
    do {
        sel = DoSelection();

        if( sel >= 0 ) {
            returnValue = filenames[sel];
            // confirmation
            char str[256];
            strcpy(str, "Do you want to load\n\"");
            strcat(str, timestrings[sel]);
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
    UpdateScreenShot(NULL);
    RemoveSelection();
    manager->RemoveAndDelete(container, NULL, SSEL_FADE_FRAMES);

    // Release UI
    manager->Unlock();

    return returnValue;
}

GuiStateSelect::GuiStateSelect(GuiManager *man) : GuiSelectionList(man, NUM_STATE_ITEMS)
{
    manager = man;
    num_states = 0;
    sprScreenShot = NULL;
}

GuiStateSelect::~GuiStateSelect()
{
    // Free stuff
    FreeStateFileList();
}

