#include <stdio.h>
#include <string.h>
#include <fat.h>
#include <wiisprite.h>
#include "GameElement.h"
#include "GuiImages.h"

#define SCREENSHOT_DIR "Screenshots/"

/*************************************************
  Game Element
 *************************************************/

GameElement::GameElement()
{
    next = NULL;
    name = NULL;
    cmdline = NULL;
    screenshot[0] = NULL;
    screenshot[1] = NULL;
    image[0] = NULL;
    image[1] = NULL;
    memset(key_map, 0xff, sizeof(key_map)); /* set to -1 */
}

GameElement::GameElement(GameElement *parent)
{
    next = NULL;
    name = NULL;
    cmdline = NULL;
    screenshot[0] = NULL;
    screenshot[1] = NULL;
    image[0] = NULL;
    image[1] = NULL;
    if( parent->name != NULL ) name = strdup(parent->name);
    if( parent->cmdline != NULL ) cmdline = strdup(parent->cmdline);
    if( parent->screenshot[0] != NULL ) screenshot[0] = strdup(parent->screenshot[0]);
    if( parent->screenshot[1] != NULL ) screenshot[1] = strdup(parent->screenshot[1]);
    memcpy(key_map, parent->key_map, sizeof(key_map));
}

GameElement::~GameElement()
{
    if( name ) free(name);
    if( cmdline ) free(cmdline);
    if( screenshot[0] ) free(screenshot[0]);
    if( screenshot[1] ) free(screenshot[1]);
    if( image[0] ) delete image[0];
    if( image[1] ) delete image[1];
}

void GameElement::SetName(const char *str)
{
    if( name ) free(name);
    if( str ) {
        name = strdup(str);
    }else{
        name = NULL;
    }
}

void GameElement::SetCommandLine(const char *str)
{
    if( cmdline ) free(cmdline);
    if( str ) {
        cmdline = strdup(str);
    }else{
        cmdline = NULL;
    }
}

void GameElement::SetScreenShot(int number, const char *str)
{
    if( number < 2 ) {
        if( screenshot[number] ) free(screenshot[number]);
        if( str ) {
            screenshot[number] = (char*)malloc(strlen(str)+strlen(SCREENSHOT_DIR)+1);
            strcpy(screenshot[number], SCREENSHOT_DIR);
            strcat(screenshot[number], str);
        }
    }
}

void GameElement::SetKeyMapping(KEY key, int event)
{
    key_map[key] = event;
}

char* GameElement::GetName()
{
    return name;
}

char* GameElement::GetCommandLine()
{
    return cmdline;
}

char* GameElement::GetScreenShot(int number)
{
    if( number < 2 ) {
        return screenshot[number];
    }else{
        return NULL;
    }
}

int GameElement::GetKeyMapping(KEY key)
{
    return key_map[key];
}

void GameElement::FreeImage(int number)
{
    if( image[number] ) {
        delete image[number];
        image[number] = NULL;
    }
}

Image* GameElement::GetImage(int number)
{
    if( image[number] == NULL ) {
        char *filename = GetScreenShot(number);
        if( filename ) {
            image[number] = new Image;
            if(image[number]->LoadImage(filename) != IMG_LOAD_ERROR_NONE) {
                delete image[number];
                image[number] = NULL;
            }
        }
    }
    if( image[number] != NULL ) {
        return image[number];
    }else{
        return g_imgNoise;
    }
}

