/*****************************************************************************
** $Source: WiiShortcuts.c,v $
**
** $Revision: 1.4 $
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
#include <stdlib.h>
#include "WiiShortcuts.h"
#include "Actions.h"
#include "InputEvent.h"

struct _Shortcuts {
    int dummy;
};

Shortcuts* shortcutsCreate(void)
{
    Shortcuts* shortcuts = (Shortcuts*)calloc(1, sizeof(Shortcuts));
    return shortcuts;
}

void shortcutsDestroy(Shortcuts* shortcuts)
{
    free(shortcuts);
}

void shortcutCheckDown(Shortcuts* s, int key)
{
}

void shortcutCheckUp(Shortcuts* s, int key)
{
    if (key == EC_HOT_QUIT)                    actionQuit();
    if (key == EC_HOT_TOGGLE_FDC_TIMING)       actionToggleFdcTiming();
    if (key == EC_HOT_TOGGLE_SPRITE_ENABLE)    actionToggleSpriteEnable();
    if (key == EC_HOT_TOGGLE_MSX_AUDIO_SWITCH) actionToggleMsxAudioSwitch();
    if (key == EC_HOT_TOGGLE_FRONT_SWITCH)     actionToggleFrontSwitch();
    if (key == EC_HOT_TOGGLE_PAUSE_SWITCH)     actionTogglePauseSwitch();
    if (key == EC_HOT_TOGGLE_WAVE_CAPTURE)     actionToggleWaveCapture();
    if (key == EC_HOT_SCREEN_CAPTURE)          actionScreenCapture();
    if (key == EC_HOT_QUICK_LOAD_STATE)        actionQuickLoadState();
    if (key == EC_HOT_QUICK_SAVE_STATE)        actionQuickSaveState();
    if (key == EC_HOT_CARD_REMOVE_1)           actionCartRemove1();
    if (key == EC_HOT_CARD_REMOVE_2)           actionCartRemove2();
    if (key == EC_HOT_TOGGLE_CARD_AUTO_RESET)  actionToggleCartAutoReset();
    if (key == EC_HOT_DISK_QUICK_CHANGE)       actionDiskQuickChange();
    if (key == EC_HOT_DISK_REMOVE_A)           actionDiskRemoveA();
    if (key == EC_HOT_DISK_REMOVE_B)           actionDiskRemoveB();
    if (key == EC_HOT_TOGGLE_DISK_AUTO_RESET)  actionToggleDiskAutoReset();
    if (key == EC_HOT_CAS_REWIND)              actionCasRewind();
    if (key == EC_HOT_CAS_REMOVE)              actionCasRemove();
    if (key == EC_HOT_CAS_TOGGLE_READ_ONLY)    actionCasToggleReadonly();
    if (key == EC_HOT_TOGGLE_CAS_AUTO_REWIND)  actionToggleCasAutoRewind();
    if (key == EC_HOT_CAS_SAVE)                actionCasSave();
    if (key == EC_HOT_EMU_TOGGLE_PAUSE)        actionEmuTogglePause();
    if (key == EC_HOT_EMU_STOP)                actionEmuStop();
    if (key == EC_HOT_EMU_SPEED_NORMAL)        actionEmuSpeedNormal();
    if (key == EC_HOT_EMU_SPEED_INCREASE)      actionEmuSpeedIncrease();
    if (key == EC_HOT_EMU_SPEED_DECREASE)      actionEmuSpeedDecrease();
    if (key == EC_HOT_MAX_SPEED_TOGGLE)        actionMaxSpeedToggle();
    if (key == EC_HOT_EMU_RESET_SOFT)          actionEmuResetSoft();
    if (key == EC_HOT_EMU_RESET_HARD)          actionEmuResetHard();
    if (key == EC_HOT_EMU_RESET_CLEAN)         actionEmuResetClean();
    if (key == EC_HOT_VOLUME_INCREASE)         actionVolumeIncrease();
    if (key == EC_HOT_VOLUME_DECREASE)         actionVolumeDecrease();
    if (key == EC_HOT_MUTE_TOGGLE_MASTER)      actionMuteToggleMaster();
    if (key == EC_HOT_VOLUME_TOGGLE_STEREO)    actionVolumeToggleStereo();
    if (key == EC_HOT_WINDOW_SIZE_NORMAL)      actionWindowSizeNormal();
    if (key == EC_HOT_WINDOW_SIZE_FULLSCREEN)  actionWindowSizeFullscreen();
    if (key == EC_HOT_FULLSCREEN_TOGGLE)       actionFullscreenToggle();
}

