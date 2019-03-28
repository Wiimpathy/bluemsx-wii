/*****************************************************************************
** $Source: WiiInput.c,v $
**
** $Revision: 1.6 $
**
** $Date: 2006/06/24 02:27:08 $
**
** More info: http://www.bluemsx.com
**
** Copyright (C) 2003-2006 Daniel Vik
**
**  This software is provided 'as-is', without any express or implied
**  warranty.  In no event will the authors be held liable for any damages
**  arising from the use of this software.
**
**  Permission is granted to anyone to use this software for any purpose,
**  including commercial applications, and to alter it and redistribute it
**  freely, subject to the following restrictions:
**
**  1. The origin of this software must not be misrepresented; you must not
**     claim that you wrote the original software. If you use this software
**     in a product, an acknowledgment in the product documentation would be
**     appreciated but is not required.
**  2. Altered source versions must be plainly marked as such, and must not be
**     misrepresented as being the original software.
**  3. This notice may not be removed or altered from any source distribution.
**
******************************************************************************
*/
#include "ArchInput.h"


#include "Language.h"
#include "InputEvent.h"
#include "IniFileParser.h"
#include "WiiShortcuts.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "kbdlib.h"

#define MAX_JOYSTICKS 8


#define KBD_TABLE_LEN 512

static Shortcuts* shortcuts = NULL;
static int kbdTable[KBD_TABLE_LEN];
static int keyStatus[KBD_TABLE_LEN];
static int kbdModifiers;
static int hasFocus = 1;

static char keyboardConfigDir[512];

// initKbdTable initializes the keyboard table with default keys
static void initKbdTable()
{
    memset(kbdTable, 0, sizeof(kbdTable));

    kbdTable[KEY_1          ] = EC_1;
    kbdTable[KEY_2          ] = EC_2;
    kbdTable[KEY_3          ] = EC_3;
    kbdTable[KEY_4          ] = EC_4;
    kbdTable[KEY_5          ] = EC_5;
    kbdTable[KEY_6          ] = EC_6;
    kbdTable[KEY_7          ] = EC_7;
    kbdTable[KEY_8          ] = EC_8;
    kbdTable[KEY_9          ] = EC_9;
    kbdTable[KEY_0          ] = EC_0;

    kbdTable[KEY_EXCLAIM    ] = EC_1;
    kbdTable[KEY_AT         ] = EC_2;
    kbdTable[KEY_HASH       ] = EC_3;
    kbdTable[KEY_DOLLAR     ] = EC_4;
    kbdTable[KEY_PERCENT    ] = EC_5;
    kbdTable[KEY_CARET      ] = EC_6;
    kbdTable[KEY_AMPERSAND  ] = EC_7;
    kbdTable[KEY_ASTERISK   ] = EC_8;
    kbdTable[KEY_LEFTPAREN  ] = EC_9;
    kbdTable[KEY_RIGHTPAREN ] = EC_0;

    kbdTable[KEY_A          ] = EC_A;
    kbdTable[KEY_B          ] = EC_B;
    kbdTable[KEY_C          ] = EC_C;
    kbdTable[KEY_D          ] = EC_D;
    kbdTable[KEY_E          ] = EC_E;
    kbdTable[KEY_F          ] = EC_F;
    kbdTable[KEY_G          ] = EC_G;
    kbdTable[KEY_H          ] = EC_H;
    kbdTable[KEY_I          ] = EC_I;
    kbdTable[KEY_J          ] = EC_J;
    kbdTable[KEY_K          ] = EC_K;
    kbdTable[KEY_L          ] = EC_L;
    kbdTable[KEY_M          ] = EC_M;
    kbdTable[KEY_N          ] = EC_N;
    kbdTable[KEY_O          ] = EC_O;
    kbdTable[KEY_P          ] = EC_P;
    kbdTable[KEY_Q          ] = EC_Q;
    kbdTable[KEY_R          ] = EC_R;
    kbdTable[KEY_S          ] = EC_S;
    kbdTable[KEY_T          ] = EC_T;
    kbdTable[KEY_U          ] = EC_U;
    kbdTable[KEY_V          ] = EC_V;
    kbdTable[KEY_W          ] = EC_W;
    kbdTable[KEY_X          ] = EC_X;
    kbdTable[KEY_Y          ] = EC_Y;
    kbdTable[KEY_Z          ] = EC_Z;

    kbdTable[KEY_MINUS      ] = EC_NEG;
    kbdTable[KEY_EQUALS     ] = EC_CIRCFLX;
    kbdTable[KEY_BACKSLASH  ] = EC_BKSLASH;
    kbdTable[KEY_LEFTBRACKET   ] = EC_AT;
    kbdTable[KEY_RIGHTBRACKET  ] = EC_LBRACK;
    kbdTable[KEY_COLON      ] = EC_SEMICOL;
    kbdTable[KEY_QUOTE      ] = EC_COLON;
    kbdTable[KEY_BACKQUOTE  ] = EC_RBRACK;
    kbdTable[KEY_COMMA      ] = EC_COMMA;
    kbdTable[KEY_PERIOD     ] = EC_PERIOD;
    kbdTable[KEY_SLASH      ] = EC_DIV;

    kbdTable[KEY_UNDERSCORE ] = EC_NEG;
    kbdTable[KEY_PLUS       ] = EC_CIRCFLX;
    kbdTable[KEY_BAR        ] = EC_BKSLASH;
    kbdTable[KEY_LEFTBRACE  ] = EC_AT;
    kbdTable[KEY_RIGHTBRACE ] = EC_LBRACK;
    kbdTable[KEY_SEMICOLON  ] = EC_SEMICOL;
    kbdTable[KEY_QUOTEDBL   ] = EC_COLON;
    kbdTable[KEY_ASCIITILDE ] = EC_RBRACK;
    kbdTable[KEY_LESS       ] = EC_COMMA;
    kbdTable[KEY_GREATER    ] = EC_PERIOD;
    kbdTable[KEY_QUESTION   ] = EC_DIV;

    kbdTable[KEY_RCTRL      ] = EC_UNDSCRE;

    kbdTable[KEY_F1         ] = EC_F1;
    kbdTable[KEY_F2         ] = EC_F2;
    kbdTable[KEY_F3         ] = EC_F3;
    kbdTable[KEY_F4         ] = EC_F4;
    kbdTable[KEY_F5         ] = EC_F5;
    kbdTable[KEY_F6         ] = EC_SELECT;
    kbdTable[KEY_F7         ] = EC_STOP;
    kbdTable[KEY_F8         ] = EC_CLS;
    kbdTable[KEY_F9         ] = EC_INS;
    kbdTable[KEY_F10        ] = EC_DEL;
    kbdTable[KEY_F11        ] = EC_HOT_DISK_QUICK_CHANGE;
    kbdTable[KEY_ESCAPE     ] = EC_ESC;
    kbdTable[KEY_TAB        ] = EC_TAB;
    kbdTable[KEY_PAGEUP     ] = EC_STOP;
    kbdTable[KEY_BACKSPACE  ] = EC_BKSPACE;
    kbdTable[KEY_END        ] = EC_SELECT;
    kbdTable[KEY_RETURN     ] = EC_RETURN;
    kbdTable[KEY_SPACE      ] = EC_SPACE;
    kbdTable[KEY_HOME       ] = EC_CLS;
    kbdTable[KEY_INSERT     ] = EC_INS;
    kbdTable[KEY_DELETE     ] = EC_DEL;
    kbdTable[KEY_LEFT       ] = EC_LEFT;
    kbdTable[KEY_UP         ] = EC_UP;
    kbdTable[KEY_RIGHT      ] = EC_RIGHT;
    kbdTable[KEY_DOWN       ] = EC_DOWN;

    kbdTable[KEY_KP_MULTIPLY] = EC_NUMMUL;
    kbdTable[KEY_KP_PLUS    ] = EC_NUMADD;
    kbdTable[KEY_KP_DIVIDE  ] = EC_NUMDIV;
    kbdTable[KEY_KP_MINUS   ] = EC_NUMSUB;
    kbdTable[KEY_KP_PERIOD  ] = EC_NUMPER;
    kbdTable[KEY_PAGEDOWN   ] = EC_NUMCOM;
    kbdTable[KEY_KP0        ] = EC_NUM0;
    kbdTable[KEY_KP1        ] = EC_NUM1;
    kbdTable[KEY_KP2        ] = EC_NUM2;
    kbdTable[KEY_KP3        ] = EC_NUM3;
    kbdTable[KEY_KP4        ] = EC_NUM4;
    kbdTable[KEY_KP5        ] = EC_NUM5;
    kbdTable[KEY_KP6        ] = EC_NUM6;
    kbdTable[KEY_KP7        ] = EC_NUM7;
    kbdTable[KEY_KP8        ] = EC_NUM8;
    kbdTable[KEY_KP9        ] = EC_NUM9;
    kbdTable[KEY_KP_ENTER   ] = EC_RETURN;

    kbdTable[KEY_LWIN       ] = EC_TORIKE;
    kbdTable[KEY_RWIN       ] = EC_JIKKOU;
    kbdTable[KEY_LSHIFT     ] = EC_LSHIFT;
    kbdTable[KEY_RSHIFT     ] = EC_RSHIFT;
    kbdTable[KEY_LCTRL      ] = EC_CTRL;
    kbdTable[KEY_LALT       ] = EC_GRAPH;
    kbdTable[KEY_RALT       ] = EC_CODE;
    kbdTable[KEY_CAPSLOCK   ] = EC_CAPS;
    kbdTable[KEY_KP_ENTER   ] = EC_PAUSE;
    kbdTable[KEY_PRINT      ] = EC_PRINT;

    kbdTable[KEY_JOY1_UP        ] = EC_JOY1_UP;
    kbdTable[KEY_JOY1_DOWN      ] = EC_JOY1_DOWN;
    kbdTable[KEY_JOY1_LEFT      ] = EC_JOY1_LEFT;
    kbdTable[KEY_JOY1_RIGHT     ] = EC_JOY1_RIGHT;
    kbdTable[KEY_JOY1_WIIMOTE_A ] = EC_JOY1_BUTTON1;
    kbdTable[KEY_JOY1_WIIMOTE_B ] = EC_JOY1_BUTTON2;
    kbdTable[KEY_JOY1_WIIMOTE_1 ] = EC_JOY1_BUTTON2;
    kbdTable[KEY_JOY1_WIIMOTE_2 ] = EC_JOY1_BUTTON1;
    kbdTable[KEY_JOY1_NUNCHUCK_C] = EC_JOY1_BUTTON2;
    kbdTable[KEY_JOY1_NUNCHUCK_Z] = EC_JOY1_BUTTON1;
    kbdTable[KEY_JOY1_CLASSIC_A ] = EC_JOY1_BUTTON1;
    kbdTable[KEY_JOY1_CLASSIC_B ] = EC_JOY1_BUTTON2;
    kbdTable[KEY_JOY1_CLASSIC_X ] = EC_JOY1_BUTTON1;
    kbdTable[KEY_JOY1_CLASSIC_Y ] = EC_JOY1_BUTTON2;
    kbdTable[KEY_JOY1_CLASSIC_L ] = EC_JOY1_BUTTON1;
    kbdTable[KEY_JOY1_CLASSIC_R ] = EC_JOY1_BUTTON2;
    kbdTable[KEY_JOY1_CLASSIC_ZL] = EC_JOY1_BUTTON1;
    kbdTable[KEY_JOY1_CLASSIC_ZR] = EC_JOY1_BUTTON2;

    kbdTable[KEY_JOY2_UP        ] = EC_JOY2_UP;
    kbdTable[KEY_JOY2_DOWN      ] = EC_JOY2_DOWN;
    kbdTable[KEY_JOY2_LEFT      ] = EC_JOY2_LEFT;
    kbdTable[KEY_JOY2_RIGHT     ] = EC_JOY2_RIGHT;
    kbdTable[KEY_JOY2_WIIMOTE_A ] = EC_JOY2_BUTTON1;
    kbdTable[KEY_JOY2_WIIMOTE_B ] = EC_JOY2_BUTTON2;
    kbdTable[KEY_JOY2_WIIMOTE_1 ] = EC_JOY2_BUTTON2;
    kbdTable[KEY_JOY2_WIIMOTE_2 ] = EC_JOY2_BUTTON1;
    kbdTable[KEY_JOY2_NUNCHUCK_C] = EC_JOY2_BUTTON2;
    kbdTable[KEY_JOY2_NUNCHUCK_Z] = EC_JOY2_BUTTON1;
    kbdTable[KEY_JOY2_CLASSIC_A ] = EC_JOY2_BUTTON1;
    kbdTable[KEY_JOY2_CLASSIC_B ] = EC_JOY2_BUTTON2;
    kbdTable[KEY_JOY2_CLASSIC_X ] = EC_JOY2_BUTTON1;
    kbdTable[KEY_JOY2_CLASSIC_Y ] = EC_JOY2_BUTTON2;
    kbdTable[KEY_JOY2_CLASSIC_L ] = EC_JOY2_BUTTON1;
    kbdTable[KEY_JOY2_CLASSIC_R ] = EC_JOY2_BUTTON2;
    kbdTable[KEY_JOY2_CLASSIC_ZL] = EC_JOY2_BUTTON1;
    kbdTable[KEY_JOY2_CLASSIC_ZR] = EC_JOY2_BUTTON2;

}

void keyboardRemapKey(KEY key, int event)
{
    kbdTable[key] = event;
}

int keyboardGetMapping(KEY key)
{
    return kbdTable[key];
}

void keyboardSetDirectory(char* directory)
{
    strcpy(keyboardConfigDir, directory);
}

void keyboardInit(void)
{
    initKbdTable();
    inputEventReset();

    kbdModifiers = 0;

    (void)KBD_Init();

    shortcuts = shortcutsCreate();
}

void keyboardClose(void)
{
    KBD_DeInit();
    if( shortcuts ) {
        shortcutsDestroy(shortcuts);
        shortcuts = NULL;
    }
}

void keyboardReset(void)
{
    initKbdTable();
    inputEventReset();
}

void keyboardSetFocus(int handle, int focus)
{
    if (focus) {
        hasFocus |= handle;
    }
    else {
        hasFocus &= ~handle;
    }
}

static void keyboardHandleKeypress(KEY code, int pressed)
{
    int wasPressed = keyStatus[code];
    int keyCode = kbdTable[code];
    keyStatus[code] = pressed;

    if (pressed != wasPressed && keyCode != 0) {
        if (pressed) {
            inputEventSet(keyCode);
            shortcutCheckDown(shortcuts, keyCode);
        }
        else {
            inputEventUnset(keyCode);
            shortcutCheckUp(shortcuts, keyCode);
        }
    }
}

static void keyboardCallbackKeypress(KEY code, int pressed)
{
    keyboardHandleKeypress(code, pressed);
}

static void keyboardResetKbd()
{
    int i;
    for (i = 0; i < KBD_TABLE_LEN; i++) {
        kbdModifiers = 0;
        keyboardHandleKeypress(kbdTable[i], 0);
    }
    inputEventReset();
}

void keyboardUpdate(void)
{
    if (!hasFocus) {
        keyboardResetKbd();
        return;
    }
    KBD_GetKeys(keyboardCallbackKeypress);

    kbdModifiers = ((keyStatus[KEY_LSHIFT] ? 1 : 0) << 0) | ((keyStatus[KEY_RSHIFT] ? 1 : 0) << 1) |
                   ((keyStatus[KEY_LCTRL]  ? 1 : 0) << 2) | ((keyStatus[KEY_RCTRL]  ? 1 : 0) << 3) |
                   ((keyStatus[KEY_LALT]   ? 1 : 0) << 4) | ((keyStatus[KEY_RALT]   ? 1 : 0) << 5) |
                   ((keyStatus[KEY_LWIN]   ? 1 : 0) << 6) | ((keyStatus[KEY_RWIN]   ? 1 : 0) << 7);
}

int keyboardGetModifiers(void)
{
    return kbdModifiers;
}

void  archUpdateJoystick() {}

UInt8 archJoystickGetState(int joystickNo) { return 0; }
int  archJoystickGetCount() { return 0; }
char* archJoystickGetName(int index) { return ""; }
void archMouseSetForceLock(int lock) { }

void  archPollInput()
{
    keyboardUpdate();
}
void  archKeyboardSetSelectedKey(int keyCode) {}
char* archGetSelectedKey() { return ""; }
char* archGetMappedKey() { return ""; }
int   archKeyboardIsKeyConfigured(int msxKeyCode) { return 0; }
int   archKeyboardIsKeySelected(int msxKeyCode) { return 0; }
char* archKeyconfigSelectedKeyTitle() { return ""; }
char* archKeyconfigMappedToTitle() { return ""; }
char* archKeyconfigMappingSchemeTitle() { return ""; }

