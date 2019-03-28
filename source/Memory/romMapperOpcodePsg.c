/*****************************************************************************
** $Source: /cvsroot/bluemsx/blueMSX/Src/Memory/romMapperOpcodePsg.c,v $
**
** $Revision: 1.1 $
**
** $Date: 2008/11/23 20:26:12 $
**
** More info: http://www.bluemsx.com
**
** Copyright (C) 2003-2006 Daniel Vik
**
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
** 
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Software
** Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
**
******************************************************************************
*/
#include "romMapperOpcodePsg.h"
#include "MediaDb.h"
#include "DeviceManager.h"
#include "DebugDeviceManager.h"
#include "SaveState.h"
#include "AY8910.h"
#include "Board.h"
#include "IoPort.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

typedef struct {
    AY8910* ay8910;
    int    deviceHandle;
    int    debugHandle;
} RomMapperOpcodePsg;

static void saveState(RomMapperOpcodePsg* rm)
{
    ay8910SaveState(rm->ay8910);
}

static void loadState(RomMapperOpcodePsg* rm)
{
    ay8910LoadState(rm->ay8910);
}

static void destroy(RomMapperOpcodePsg* rm)
{
    int i;
    deviceManagerUnregister(rm->deviceHandle);
    debugDeviceUnregister(rm->debugHandle);
    ay8910Destroy(rm->ay8910);

    for (i = 0; i < 16; i++) {
        ioPortUnregister(0x60 + i);
    }

    free(rm);
}

static UInt8 peek(RomMapperOpcodePsg* rm, UInt16 ioPort)
{
    return ay8910PeekData(rm->ay8910, ioPort);
}

static UInt8 read(RomMapperOpcodePsg* rm, UInt16 ioPort)
{
    return ay8910ReadData(rm->ay8910, ioPort);
}

static void write(RomMapperOpcodePsg* rm, UInt16 ioPort, UInt8 value)
{
    if ((ioPort & 3) == 0) {
        ay8910WriteAddress(rm->ay8910, ioPort, value);
    }
    if ((ioPort & 3) == 1) {
        ay8910WriteData(rm->ay8910, ioPort, value);
    }
}

static void reset(RomMapperOpcodePsg* rm)
{
    ay8910Reset(rm->ay8910);
}

static void getDebugInfo(RomMapperOpcodePsg* rm, DbgDevice* dbgDevice)
{
    DbgIoPorts* ioPorts;
    int i;

    ioPorts = dbgDeviceAddIoPorts(dbgDevice, "AY8910", 16);

    for (i = 0; i < 16; i += 4) {
        UInt8 ioPort = 0x60 + i;
        dbgIoPortsAddPort(ioPorts, i + 0, ioPort + 0, DBG_IO_WRITE, 0xff);
        dbgIoPortsAddPort(ioPorts, i + 1, ioPort + 1, DBG_IO_WRITE, 0xff);
        dbgIoPortsAddPort(ioPorts, i + 2, ioPort + 2, DBG_IO_READ,  peek(rm, ioPort + 2));
        dbgIoPortsAddPort(ioPorts, i + 3, ioPort + 3, DBG_IO_NONE,  0xff);
    }
}

int romMapperOpcodePsgCreate() 
{
    int i;

    DeviceCallbacks callbacks = { destroy, reset, saveState, loadState };
    DebugCallbacks dbgCallbacks = { getDebugInfo, NULL, NULL, NULL };
    RomMapperOpcodePsg* rm = malloc(sizeof(RomMapperOpcodePsg));

    rm->deviceHandle = deviceManagerRegister(ROM_OPCODEPSG, &callbacks, rm);
    rm->debugHandle = debugDeviceRegister(DBGTYPE_AUDIO, "AY8910", &dbgCallbacks, rm);

    rm->ay8910 = ay8910Create(boardGetMixer(), AY8910_MSX, PSGTYPE_AY8910);

    for (i = 0; i < 16; i += 4) {
        ioPortRegister(0x60 + i, NULL, write, rm);
        ioPortRegister(0x61 + i, NULL, write, rm);
        ioPortRegister(0x62 + i, read, NULL, rm);
    }

    reset(rm);

    return 1;
}

