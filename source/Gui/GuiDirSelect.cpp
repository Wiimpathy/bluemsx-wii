#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "GuiDirSelect.h"
#include "GuiContainer.h"
#include "GuiMessageBox.h"

#define DSEL_YPITCH 50

#define DIRSEL_FADE_FRAMES 10

//char *GuiDirSelect::DoModal(void)
char *GuiDirSelect::DoModal(bool autoboot)
{
    char *return_value = NULL;

    // Claim UI
    manager->Lock();

    // Containers
    GuiContainer *containerDirList = new GuiContainer(320-180, 22, 2*180, 440-48);
    manager->AddTop(containerDirList, DIRSEL_FADE_FRAMES);

    // Release UI
    manager->Unlock();

    // On re-entry, go back one level if not on root level
    char *prevsel = NULL;
    if( dir_level > 0 ) {
        char *p = current_dir + strlen(current_dir) - 1;
        while(*p != '/') p--;
        *p = '\0';
        prevsel = p + 1;
        dir_level--;
    }

    // Seletion loop
    bool quit = false;
    do {
        DirElement *selected_dir = NULL;

        // Load dirs database
        chdir(current_dir);
        dirs.Load(xmlfile);
        num_dirs = dirs.GetNumberOfDirs();
        if( num_dirs == 0 ) {
            return_value = current_dir;
            break;
        }
        title_list = (const char**)realloc(title_list, num_dirs * sizeof(const char*));
        for(int i = 0; i < num_dirs; i++) {
            title_list[i] = dirs.GetDir(i)->GetName();
        }

        // When just gone back one level, find entry comming from
        int sel = 0;
        if( prevsel ) {
            for( int i = 0; i < num_dirs; i++ ) {
                if( strcmp(prevsel, dirs.GetDir(i)->GetDirectory()) == 0 ) {
                    sel = i;
                }
            }
        }

				// Skip menu
				if(autoboot)
				{
						quit = true;
						break;
				}

        // Selection
        if( IsShowing() ) {
            RemoveSelection();
        }
        ShowSelection(title_list, num_dirs, sel, 30, DSEL_YPITCH,
                      320-180+8, 42, 24, 2*180-16, false, DIRSEL_FADE_FRAMES);
        for(;;) {
            sel = DoSelection();

            if( sel >= 0 ) {
                // enter selected directory
                selected_dir = dirs.GetDir(sel);
                strcat(current_dir, "/");
                strcat(current_dir, selected_dir->GetDirectory());
                dir_level++;
                break;
            }else{
                if( dir_level == 0 ) {
                    // on root level, leave after confirmation
                    GuiMessageBox msgbox(manager);
                    bool ok = msgbox.Show("Do you want to quit?", NULL, true, 192);
                    msgbox.Remove();
                    if( ok ) {
                        RemoveSelection();
                        quit = true;
                        break;
                    }
                }else{
                    // go back one level
                    char *p = current_dir + strlen(current_dir) - 1;
                    while(*p != '/') p--;
                    *p = '\0';
                    prevsel = p + 1;
                    dir_level--;
                    break;
                }
            }
        }
    }while( !quit );
    RemoveSelection();
    dirs.Clear();

    // Claim UI
    manager->Lock();

    // Remove container
    manager->RemoveAndDelete(containerDirList, NULL, DIRSEL_FADE_FRAMES);

    // Release UI
    manager->Unlock();

    return return_value;
}

GuiDirSelect::GuiDirSelect(GuiManager *man, const char *startdir, const char *filename)
            : GuiSelectionList(man, NUM_DIR_ITEMS)
{
    manager = man;
    strcpy(current_dir, startdir);
    strcpy(xmlfile, filename);
    dir_level = 0;
    title_list = NULL;
}

GuiDirSelect::~GuiDirSelect()
{
    if( title_list ) {
        free(title_list);
    }
}

