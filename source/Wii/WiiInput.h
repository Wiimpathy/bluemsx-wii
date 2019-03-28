#ifndef _WII_INPUT_H
#define _WII_INPUT_H

#ifdef __cplusplus
extern "C" {
#endif

#include "kbdlib.h"

extern void keyboardSetDirectory(char* directory);
extern void keyboardInit(void);
extern void keyboardClose(void);
extern void keyboardReset(void);
extern void keyboardRemapKey(KEY key, int event);
extern int keyboardGetMapping(KEY key);
extern void keyboardSetFocus(int handle, int focus);
extern void keyboardUpdate(void);
extern int keyboardGetModifiers(void);

#ifdef __cplusplus
}
#endif

#endif

