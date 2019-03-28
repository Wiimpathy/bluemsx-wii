/*****************************************************************************
** $Source: StorageSetup.h,v $
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

#include "GuiManager.h"

#define SD_ROOT_DIR  "sd:/"
#define USB_ROOT_DIR "usb:/"
#define MSX_DIR      "/MSX"

extern bool SetupStorage(GuiManager *manager, bool bSDMounted, bool bUSBMounted);
extern char *GetMSXRootPath(void);

#ifdef __cplusplus
    extern "C" {
#endif
extern char *GetStorageRootPath(void);
#ifdef __cplusplus
}
#endif

