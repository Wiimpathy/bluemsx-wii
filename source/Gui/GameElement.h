#ifndef _H_GAMEELEMENT
#define _H_GAMEELEMENT

#include <wiisprite.h>
#include "expat.h"
#include "kbdlib.h"

using namespace wsp;

class GameElement
{
public:
    GameElement();
    GameElement(GameElement* parent);
    virtual ~GameElement();

    void SetName(const char *str);
    void SetCommandLine(const char *str);
    void SetScreenShot(int number, const char *str);
    void SetKeyMapping(KEY key, int event);
    char *GetName();
    char *GetCommandLine();
    char *GetScreenShot(int number);
    int GetKeyMapping(KEY key);
    void FreeImage(int number);
    Image* GetImage(int number);
    GameElement *next;
private:
    char *name;
    char *cmdline;
    char *screenshot[2];
    Image *image[2];
    int key_map[KEY_LAST];
};

#endif

