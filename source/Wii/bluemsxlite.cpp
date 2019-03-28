/*****************************************************************************
** $Source: /cvsroot/bluemsx/blueMSX/Src/Sdl/bluemsxlite.c,v $
**
** $Revision: 1.20 $
**
** $Date: 2006/06/24 08:35:29 $
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
#include <debug.h>
#include <fat.h>
#include <ogc/system.h>      // for syswd_t (required in ogc/usbstorage.h)
#include <ogc/usbstorage.h>
#include <sdcard/wiisd_io.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <wiiuse/wpad.h>

#include "GuiBackground.h"
#include "GuiDirSelect.h"
#include "GuiGameSelect.h"
#include "GuiStateSelect.h"
#include "GuiMenu.h"
#include "GuiMessageBox.h"
#include "GuiKeyboard.h"

extern "C" {
#include "CommandLine.h"
#include "Properties.h"
#include "ArchFile.h"
#include "VideoRender.h"
#include "AudioMixer.h"
#include "Casette.h"
#include "PrinterIO.h"
#include "UartIO.h"
#include "MidiIO.h"
#include "Machine.h"
#include "Board.h"
#include "Emulator.h"
#include "FileHistory.h"
#include "Actions.h"
#include "Language.h"
#include "LaunchFile.h"
#include "ArchEvent.h"
#include "ArchSound.h"
#include "ArchNotifications.h"
#include "JoystickPort.h"
#include "WiiShortcuts.h"
#include "ArchThread.h"
#include "WiiInput.h"
}

#include "GuiConsole.h"
#include "GuiContainer.h"
#include "GuiFonts.h"
#include "GuiImages.h"
#include "StorageSetup.h"

#define ENABLE_GECKO  0
#define CONSOLE_DEBUG 0
#define FORCE_50HZ    0

#define TEX_WIDTH  (512+32)
#define TEX_HEIGHT 480

static Properties* properties;
static Video* video;
static Mixer* mixer;

static int   bitDepth = 16;
static int   zoom = 1;
static int   displayPitch = TEX_WIDTH * 2;

static GuiManager *manager = NULL;
static GuiBackground *background = NULL;
#if CONSOLE_DEBUG
static GuiConsole *console = NULL;
#endif
static GuiMessageBox *msgbox = NULL;
static GuiKeyboard *osk = NULL;

bool autoboot = false;
bool doublestrike = false;
bool scanlines = false;

#define WIDTH  320
#define HEIGHT 240

#define EVENT_UPDATE_DISPLAY 2
#define EVENT_UPDATE_WINDOW  3

extern "C" {
extern void __exception_setreload(int t);
}

extern "C" void archTrap(UInt8 value)
{
}

int  archUpdateEmuDisplay(int syncMode)
{
    return 1;
}

void archUpdateWindow()
{
}

static char stateDir[PROP_MAXPATH]  = "";
static char screenShotDir[PROP_MAXPATH]  = "";

void setDefaultPaths(const char* rootDir)
{
    char buffer[512];

    propertiesSetDirectory(rootDir, rootDir);

    sprintf(buffer, "%s/Audio Capture", rootDir);
    archCreateDirectory(buffer);
    actionSetAudioCaptureSetDirectory(buffer, "");

    sprintf(buffer, "%s/QuickSave", rootDir);
    archCreateDirectory(buffer);
    strcpy(stateDir, buffer);
    actionSetQuickSaveSetDirectory(buffer, "");

    sprintf(buffer, "%s/SRAM", rootDir);
    archCreateDirectory(buffer);
    boardSetDirectory(buffer);

    sprintf(buffer, "%s/Casinfo", rootDir);
    archCreateDirectory(buffer);
    tapeSetDirectory(buffer, "");

    sprintf(buffer, "%s/Databases", rootDir);
    archCreateDirectory(buffer);
    mediaDbLoad(buffer);

    sprintf(buffer, "%s/Keyboard Config", rootDir);
    archCreateDirectory(buffer);
    keyboardSetDirectory(buffer);

    sprintf(buffer, "%s/ScreenShots", rootDir);
    archCreateDirectory(buffer);
    strcpy(screenShotDir, buffer);
}

static char currentDisk[256];

void archDiskQuickChangeNotify(int driveId, char* fileName, const char* fileInZipFile)
{
    printf("DISKCHANGE: %d, '%s', '%s'\n", driveId, fileName, fileInZipFile);
    if( fileInZipFile ) {
        strcpy(currentDisk, fileInZipFile);
    }else{
        strcpy(currentDisk, fileName);
    }
    // Show popup
    msgbox->ShowPopup(currentDisk, g_imgFloppyDisk, 192);
}

void blueMsxInit(int resetProperties)
{
    int width;
    int height;

    setDefaultPaths(archGetCurrentDirectory());

    properties = propCreate(resetProperties, 0, P_KBD_EUROPEAN, 0, "");

    if (resetProperties == 2) {
        propDestroy(properties);
        return;
    }

    video = videoCreate();

    if (properties->video.windowSize == P_VIDEO_SIZEFULLSCREEN) {
        zoom = properties->video.fullscreen.width / WIDTH;
        bitDepth = properties->video.fullscreen.bitDepth;
    }
    else {
        if (properties->video.windowSize == P_VIDEO_SIZEX1) {
            zoom = 1;
        }
        else {
            zoom = 2;
        }
        bitDepth = 16;
    }

    width  = zoom * WIDTH;
    height = zoom * HEIGHT;

    mixer = mixerCreate();

    emulatorInit(properties, mixer);

    actionInit(video, properties, mixer);

    langInit();

    tapeSetReadOnly(properties->cassette.readOnly);

    langSetLanguage((EmuLanguageType)properties->language);

    joystickPortSetType(0, (JoystickPortType)properties->joy1.typeId);
    joystickPortSetType(1, (JoystickPortType)properties->joy2.typeId);
    printerIoSetType((PrinterType)properties->ports.Lpt.type, properties->ports.Lpt.fileName);
    printerIoSetType((PrinterType)properties->ports.Lpt.type, properties->ports.Lpt.fileName);
    uartIoSetType((UartType)properties->ports.Com.type, properties->ports.Com.fileName);
    midiIoSetMidiOutType((MidiType)properties->sound.MidiOut.type, properties->sound.MidiOut.fileName);
    midiIoSetMidiInType((MidiType)properties->sound.MidiIn.type, properties->sound.MidiIn.fileName);
    ykIoSetMidiInType((MidiType)properties->sound.YkIn.type, properties->sound.YkIn.fileName);

    emulatorRestartSound();

    for (int i = 0; i < MIXER_CHANNEL_TYPE_COUNT; i++) {
        mixerSetChannelTypeVolume(mixer, i, properties->sound.mixerChannel[i].volume);
        mixerSetChannelTypePan(mixer, i, properties->sound.mixerChannel[i].pan);
        mixerEnableChannelType(mixer, i, properties->sound.mixerChannel[i].enable);
    }

    mixerSetMasterVolume(mixer, properties->sound.masterVolume);
    mixerEnableMaster(mixer, properties->sound.masterEnable);

    videoUpdateAll(video, properties);

    mediaDbSetDefaultRomType(properties->cartridge.defaultType);

    for (int i = 0; i < PROP_MAX_CARTS; i++) {
        if (properties->media.carts[i].fileName[0]) insertCartridge(properties, i, properties->media.carts[i].fileName, properties->media.carts[i].fileNameInZip, properties->media.carts[i].type, -1);
        updateExtendedRomName(i, properties->media.carts[i].fileName, properties->media.carts[i].fileNameInZip);
    }

    for (int i = 0; i < PROP_MAX_DISKS; i++) {
        if (properties->media.disks[i].fileName[0]) insertDiskette(properties, i, properties->media.disks[i].fileName, properties->media.disks[i].fileNameInZip, -1);
        updateExtendedDiskName(i, properties->media.disks[i].fileName, properties->media.disks[i].fileNameInZip);
    }

    for (int i = 0; i < PROP_MAX_TAPES; i++) {
        if (properties->media.tapes[i].fileName[0]) insertCassette(properties, i, properties->media.tapes[i].fileName, properties->media.tapes[i].fileNameInZip, 0);
        updateExtendedCasName(i, properties->media.tapes[i].fileName, properties->media.tapes[i].fileNameInZip);
    }

    Machine* machine = machineCreate(properties->emulation.machineName);
    if (machine != NULL) {
        boardSetMachine(machine);
        machineDestroy(machine);
    }

    boardSetFdcTimingEnable(properties->emulation.enableFdcTiming);
    boardSetY8950Enable(properties->sound.chip.enableY8950);
    boardSetYm2413Enable(properties->sound.chip.enableYM2413);
    boardSetMoonsoundEnable(properties->sound.chip.enableMoonsound);
    boardSetVideoAutodetect(properties->video.detectActiveMonitor);
}

void RenderEmuImage(void *dpyData)
{
    if( emulatorGetState() == EMU_RUNNING ) {
        FrameBuffer* frameBuffer;
        frameBuffer = frameBufferFlipViewFrame(properties->emulation.syncMethod == P_EMU_SYNCTOVBLANKASYNC);
        if (frameBuffer == NULL) {
            frameBuffer = frameBufferGetWhiteNoiseFrame();
        }

        videoRender(video, frameBuffer, bitDepth, zoom,
                    dpyData, 0, displayPitch, -1);
        DCFlushRange(dpyData, TEX_WIDTH * TEX_HEIGHT * 2);
    }
}

static void blueMsxRun(GameElement *game, char *game_dir)
{
    int i;

    // Loading message
    if(!autoboot)
    {
        msgbox->Show("Loading...");
    }
    /*else
    {
        autoboot = true;
    }*/

    // Set current directory to the MSX-root
    archSetCurrentDirectory(GetMSXRootPath());

    // Reset properties
    propInitDefaults(properties, 0, P_KBD_EUROPEAN, 0, "");

    // Apply some default video propeties
#if FORCE_50HZ
    properties->emulation.vdpSyncMode = P_VDP_SYNC50HZ;
#else
    /*if( manager->GetMode() == GW_VIDEO_MODE_NTSC_440 ) {
        properties->emulation.vdpSyncMode = P_VDP_SYNC60HZ;
    }else{
        properties->emulation.vdpSyncMode = P_VDP_SYNCAUTO;
    }*/
    // Force 60hz for now.
    properties->emulation.vdpSyncMode = P_VDP_SYNC60HZ;
#endif
    properties->emulation.syncMethod = P_EMU_SYNCTOVBLANKASYNC;
    videoUpdateAll(video, properties);

    printf("Title        : '%s'\n", game->GetName());
    printf("Command line : '%s'\n", game->GetCommandLine());
    printf("Screen shot 1: '%s'\n", game->GetScreenShot(0));
    printf("Screen shot 2: '%s'\n", game->GetScreenShot(1));

    // Init keyboard and remap keys
    keyboardReset();
    KBD_SetWpadOrientation(WPADO_HORIZONTAL);
    for(i = 0; i < KEY_LAST; i++) {
        int event = game->GetKeyMapping((KEY)i);
        if( event != -1 ) {
            keyboardRemapKey((KEY)i, event);
        }
    }

    // Start emulator
    i = emuTryStartWithArguments(properties, game->GetCommandLine(), game_dir);
    if (i < 0) {
        printf("Failed to parse command line\n");
        msgbox->Remove();
        return;
    }
    if (i == 0) {
        printf("Starting emulation\n");
        emulatorStart(NULL);
    }
    printf("Waiting for quit event...\n");

    msgbox->Remove();
    manager->Lock();
    DrawableImage *emuImg = new DrawableImage;
    emuImg->CreateImage(TEX_WIDTH, TEX_HEIGHT, GX_TF_RGB565);
    manager->AddRenderCallback(RenderEmuImage, (void *)emuImg->GetTextureBuffer());
    Sprite *emuSpr = new Sprite;
    emuSpr->SetImage(emuImg->GetImage());
    emuSpr->SetStretchWidth(640.0f / (float)TEX_WIDTH);
    emuSpr->SetStretchHeight(1.0f);
    emuSpr->SetRefPixelPositioning(REFPIXEL_POS_PIXEL);
    emuSpr->SetRefPixelPosition(0, 0);
    emuSpr->SetPosition(0, ((int)manager->GetHeight()-480)/2);
    manager->AddTop(emuSpr, 90);

    // Load and display a scanlines image
    if(scanlines && !doublestrike)
    {
        Sprite *spr_image;
        spr_image = new Sprite;
        spr_image->SetImage(g_scanlines);
        float StretchScan = (manager->GetMode() == GW_VIDEO_MODE_PROG_440 ? 1.0f : 1.5f);
        spr_image->SetStretchHeight(StretchScan);
        spr_image->SetTransparency(200);
        manager->AddTop(spr_image, 30);
    }

    manager->Unlock();

    archThreadSleep(2000);
    background->Hide();

    // Create on-screen keyboard
    osk = new GuiKeyboard(manager);

    // Loop while the user hasn't quit
    GuiMenu *menu = new GuiMenu(manager, 4);
    const char *menu_items[] = {
      "Load state",
      "Save state",
      "Screenshot",
      "Quit"
    };
    int refresh = 0;
    bool pressed = true;
    GW_VIDEO_MODE prevVideo = manager->GetMode();
    bool doQuit = false;
    while(!doQuit) {
        if( prevVideo != GW_VIDEO_MODE_NTSC_440 ) {
            int newrfsh = boardGetRefreshRate();
            if( newrfsh != 0 && newrfsh != refresh ) {
                if( newrfsh==50 ) {
                    manager->SetMode(GW_VIDEO_MODE_PAL50_440 /*GW_VIDEO_MODE_PAL528*/);
                }else{
                    manager->SetMode(GW_VIDEO_MODE_PAL60_440);
                }
                emuSpr->SetPosition(0, ((int)manager->GetHeight()-480)/2);
                refresh = newrfsh;
            }
        }

        // Original progressive resolution
        if(doublestrike)
        {
            manager->Set240p();
        }

        if( KBD_GetKeyStatus(KEY_JOY1_WIIMOTE_HOME) || KBD_GetKeyStatus(KEY_JOY2_WIIMOTE_HOME) ||
            KBD_GetKeyStatus(KEY_JOY1_CLASSIC_HOME) || KBD_GetKeyStatus(KEY_JOY2_CLASSIC_HOME) ||
            KBD_GetKeyStatus(KEY_F12) ) {
            if( !pressed ) {
                emulatorSuspend();
                KBD_SetWpadOrientation(WPADO_VERTICAL);
                osk->SetEnabled(false);
                bool leave_menu = false;
                do {
                    int selection = menu->DoModal(menu_items, 4, 344);
                    switch( selection ) {
                        GuiStateSelect *statesel;
                        char *statefile;
                        case 0: /* Load state */
                            statesel = new GuiStateSelect(manager);
                            statefile = statesel->DoModal(properties, stateDir);
                            if( statefile ) {
                                msgbox->Show("Loading state...", NULL, false, 160);
                                emulatorStop();
                                emulatorStart(statefile);
                                msgbox->Remove();
                                leave_menu = true;
                            }
                            delete statesel;
                            break;
                        case 1: /* Save state */
                            msgbox->Show("Saving state...", NULL, false, 160);
                            actionQuickSaveState();
                            msgbox->Remove();
                            msgbox->Show("State saved", NULL, false, 160);
                            archThreadSleep(1000);
                            msgbox->Remove();
                            break;
                        case 2: /* Screenshot */
                            char *p, fname1[256], fname2[256];
                            strcpy(fname1, game_dir);
                            strcat(fname1, "/");
                            strcat(fname1, game->GetScreenShot(0));
                            strcpy(fname2, game_dir);
                            strcat(fname2, "/");
                            strcat(fname2, game->GetScreenShot(1));
                            if( !archFileExists(fname1) ) {
                                p = fname1;
                                /* file does not exist, make sure there is a Screenshots directory */
                                char scrshotdir[256];
                                struct stat s;
                                strcpy(scrshotdir, game_dir);
                                strcat(scrshotdir, "/Screenshots");
                                if( stat(scrshotdir, &s) != 0 ) {
                                    mkdir(scrshotdir, 0x777);
                                }
                            }else
                            if( !archFileExists(fname2) ) {
                                p = fname2;
                            }else{
                                p = generateSaveFilename(properties, screenShotDir, "", ".png", 2);
                            }
                            msgbox->Show("Saving screenshot...", NULL, false, 160);
                            (void)archScreenCaptureToFile(SC_NORMAL, p);
                            msgbox->Remove();
                            msgbox->Show("Screenshot saved", NULL, false, 160);
                            archThreadSleep(1000);
                            msgbox->Remove();
                            break;
                        case 3: /* Quit */
                            doQuit = true;
                            leave_menu = true;
                            // Exit emulator
                            if(autoboot)
                            {
	                            exit(0);
                            }
                            break;
                        case -1: /* leaved menu */
                            leave_menu = true;
                            break;
                    }
                }while(!leave_menu);
                KBD_SetWpadOrientation(WPADO_HORIZONTAL);
                KBD_GetKeys(NULL); // flush
                emulatorResume();
                osk->SetEnabled(!doQuit);
                pressed = true;
            }
        }else{
            pressed = false;
        }
        // wait a frame
        VIDEO_WaitVSync();
    }
    delete menu;

    emulatorStop();
    KBD_SetWpadOrientation(WPADO_VERTICAL);

    // Remove emulator+keyboard from display
    manager->Lock();
    manager->RemoveRenderCallback(RenderEmuImage, (void *)emuImg->GetTextureBuffer());
    manager->RemoveAndDelete(emuSpr, emuImg, 20);
    delete osk;
    manager->Unlock();

    manager->SetMode(prevVideo);
    background->Show();
}

void File_SplitPath(const char *pSrcFileName, char *pDir, char *pName)
{
	char *ptr1, *ptr2;

	/* Build pathname: */
	ptr1 = strrchr(pSrcFileName, '/');
	if (ptr1)
	{
		strcpy(pName, ptr1+1);
		memmove(pDir, pSrcFileName, ptr1-pSrcFileName);
		pDir[ptr1-pSrcFileName] = 0;
	}
	else
	{
 		strcpy(pName, pSrcFileName);
		sprintf(pDir, ".%c", '/');
	}
}

int main(int argc, char **argv)
{
    // USB Gecko
#if ENABLE_GECKO
    DEBUG_Init(GDBSTUB_DEVICE_USB, 1);
    CON_EnableGecko(1, false);
#endif
    printf("Starting...\n");

    // Memory allocator instrumentation
    allocLogStart();

    // Set main thread priority
    LWP_SetThreadPriority(LWP_GetSelf(), 100);

    L2Enhance();
    __exception_setreload(8);

    // Init Wiimote
    PAD_Init();
    WPAD_Init();
    WPAD_SetDataFormat(WPAD_CHAN_0, WPAD_FMT_BTNS_ACC_IR);
    WPAD_SetDataFormat(WPAD_CHAN_1, WPAD_FMT_BTNS_ACC_IR);

    // Init Storage (keyboard layout can be saved)
    bool bSDMounted  = fatMountSimple("sd", &__io_wiisd);
    bool bUSBMounted = fatMountSimple("usb", &__io_usbstorage);

    // Init keyboard
    keyboardInit();
    KBD_SetWpadOrientation(WPADO_VERTICAL);

    // GUI init
    manager = new GuiManager();
    GuiFontInit();
    GuiImageInit();

    // Init console
#if CONSOLE_DEBUG
    console = new GuiConsole(manager, 12, 12, 640-24, 448-24);
    console->SetVisible(true);
#endif

    // Background
    background = new GuiBackground(manager);
    background->Show();

    // Init storage access
    if( !bSDMounted && !bUSBMounted ) {
        // Prepare messagebox
        GuiMessageBox *msgboxSdSetup = new GuiMessageBox(manager);
        msgboxSdSetup->Show("No Storage (USB/SD-Card) found!");
        archThreadSleep(3000);
        delete msgboxSdSetup;
    } else
    // Init storage access
    if( SetupStorage(manager, bSDMounted, bUSBMounted) ) {
        // Set current directory to the MSX-root
        archSetCurrentDirectory(GetMSXRootPath());

        char *game_dir = NULL;
        char sGamesPath[100];
        char sGamesFile[100];

        // Please wait...
        msgbox = new GuiMessageBox(manager);

        // Parse arguments. Auto boot file and video options.
		if (argc > 1)
		{
			autoboot = true;
			File_SplitPath(argv[1], sGamesPath, sGamesFile);

			if(argv[2])
			{
				if(!strcasecmp(argv[2],"-240p"))
				{
					doublestrike = true;
				}
				else if(!strcasecmp(argv[2],"-scanlines"))
				{
					scanlines = true;
				}
			}
		}
		else
		{
			sprintf(sGamesPath, "%s/Games", GetMSXRootPath());
            msgbox->Show("Please wait...");
		}

        // Init blueMSX emulator
        blueMsxInit(1);

        msgbox->Remove();

        GuiDirSelect *dirs = new GuiDirSelect(manager, sGamesPath, "dirlist.xml");

        for(;;) {

            game_dir = dirs->DoModal(autoboot);

            if( game_dir == NULL ) {
                break;
            }
            // Game menu
            GameElement *game = NULL;
            GameElement *prev;
            for(;;) {
                GuiGameSelect *menu = new GuiGameSelect(manager);
                prev = game;


                if( menu->Load(game_dir, "gamelist.xml") ) {

                    if (autoboot)
                    {
                        game = menu->DoModal(prev, sGamesFile);
                    }
                    else
                    {
                        game = menu->DoModal(prev);
                    }
                    if( prev != NULL ) {
                        delete prev;
                    }


                }else{
                    msgbox->Show("gamelist.xml not found!");
                    archThreadSleep(2000);
                    msgbox->Remove();
                }
                delete menu;

                if( game == NULL ) {
                    break;
                }else{
                    blueMsxRun(game, game_dir);
                }
            }
        }
        delete dirs;

        printf("Clean-up\n");

        // Destroy message box
        delete msgbox;

        // Destroy emulator
        emulatorExit();
        videoDestroy(video);
        propDestroy(properties);
        archSoundDestroy();
        mixerDestroy(mixer);
    }
    // Destroy background and layer manager
    delete background;
    archThreadSleep(15*20);

#if CONSOLE_DEBUG
    // Destroy console
    delete console;
#endif
    // Free GUI resources
    GuiFontClose();
    GuiImageClose();

    delete manager;

    keyboardClose();

    fatUnmount("sd");
    fatUnmount("usb");

    allocLogStop();
    printf("Leaving...\n");
    return 0;
}

