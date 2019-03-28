
#ifndef _GUI_DIRSELECT_H
#define _GUI_DIRSELECT_H

#include "GuiSelectionList.h"
#include "DirectoryList.h"

#define MAX_PATH        256
#define NUM_DIR_ITEMS   7

class GuiDirSelect : public GuiSelectionList {
public:
    GuiDirSelect(GuiManager *man, const char *startdir, const char *filename);
    virtual ~GuiDirSelect();

    char *DoModal(bool autoboot = false);
private:
    DirList dirs;
    const char **title_list;
    int num_dirs;
    int dir_level;
    char current_dir[MAX_PATH];
    char xmlfile[MAX_PATH];
};

#endif

