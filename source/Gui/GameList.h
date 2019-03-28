#ifndef _H_GAMELIST
#define _H_GAMELIST

#include "expat.h"
#include "GameElement.h"

typedef enum {
  CONTAINER_ROOT,
  CONTAINER_GAMELIST,
  CONTAINER_GAME,
  CONTAINER_COMMANDLINE,
  CONTAINER_SCREENSHOT,
  CONTAINER_KEYMAP,
  CONTAINER_SETTINGS
} CONTAINER;

class GameList
{
public:
    GameList();
    virtual ~GameList();

    int Load(const char *filename);
    void Clear(void);
    int GetNumberOfGames(void);
    GameElement* GetGame(int index);

    static void XMLCALL startElement(void *userData, const char *name, const char **atts);
    static void XMLCALL endElement(void *userData, const char *name);
    static void XMLCALL dataHandler(void *userData, const XML_Char *s, int len);
private:
    int elements;
    char *receiving_string;
    GameElement *current_element;
    GameElement *first_element;
    GameElement *last_element;
    CONTAINER current_container;
};

#endif

