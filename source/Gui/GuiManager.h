#ifndef _GUI_MANAGER_H
#define _GUI_MANAGER_H

#include <gccore.h>
#include <wiisprite.h>

#define GUI_MAX_LAYERS 128

using namespace wsp;

typedef struct _GuiManagerCallback {
    void (*callback)(void*);
    void *context;
    _GuiManagerCallback *next;
} GuiManagerCallback;

typedef struct _FrameRemove {
    Layer *layer;
    Image *image;
    bool needdelete;
    int fade;
    int delay;
    int count;
    int alpha;
} FrameRemove;

typedef struct _FrameAdd {
    Layer *layer;
    int fade;
    int delay;
    int count;
    int alpha;
    int curalpha;
} FrameAdd;

class GuiManager {
public:
    GuiManager();
    virtual ~GuiManager();
    void SetMode(GW_VIDEO_MODE mode);
    void Set240p();
    GW_VIDEO_MODE GetMode(void);
    u32 GetWidth(void);
    u32 GetHeight(void);
    void AddRenderCallback(void (*callback)(void*), void *context);
    void RemoveRenderCallback(void (*callback)(void*), void *context);
    static void *DisplayThreadWrapper(void *arg);
    void DisplayThread(void);
    void Lock(void);
    void Unlock(void);
    void AddIndex(int index, Layer *layer, bool fix, int fade, int delay);
    void AddTop(Layer *layer, int fade = 0, int delay = 0);
    void AddTopFixed(Layer *layer, int fade = 0, int delay = 0);
    void AddOnTopOf(Layer *ontopof, Layer *layer, int fade = 0, int delay = 0);
    void AddBehind(Layer *behind, Layer *layer, int fade = 0, int delay = 0);
    void AddBottom(Layer *layer, int fade = 0, int delay = 0);
    void Remove(Layer *layer, int fade = 0, int delay = 0);
    void RemoveAndDelete(Layer *layer, Image *image = NULL, int fade = 0, int delay = 0);
    bool GetWiiMoteIR(int *x, int *y, int *angle);

private:
    void RegisterRemove(Layer *layer, bool needdelete, int fade, int delay, Image *image);

    GameWindow gwd;
    LayerManager *manager;
    mutex_t mutex;
    lwp_t thread;
    void *thread_stack;
    bool quit_thread;
    int fixed_layers;
    GuiManagerCallback *render;
    FrameRemove remove_list[GUI_MAX_LAYERS];
    FrameAdd add_list[GUI_MAX_LAYERS];
};

#endif

