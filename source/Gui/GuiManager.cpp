
#include <stdlib.h>
#include <wiiuse/wpad.h>
#include "GuiManager.h"

#define GUI_DISP_STACK_SIZE 64*1024

void *GuiManager::DisplayThreadWrapper(void *arg)
{
    GuiManager *my = (GuiManager *)arg;
    my->DisplayThread();
    return NULL;
}

void GuiManager::DisplayThread(void)
{
    do {
        LWP_MutexLock(mutex);

        // Call registered callbacks
        GuiManagerCallback *p = render;
        while( p ) {
            p->callback(p->context);
            p = p->next;
        }

        // Handle fade-out
        for(int i = 0; i < GUI_MAX_LAYERS; i++) {
            FrameRemove *remove = &remove_list[i];
            if( remove->layer ) {
                if( remove->delay ) {
                    remove->delay--;
                }else
                if( remove->count ) {
                    remove->count--;
                    remove->layer->SetTransparency((remove->count * remove->alpha) / remove->fade);
                }else{
                    if( remove->needdelete ) {
                        RemoveAndDelete(remove->layer, remove->image);
                    }else{
                        Remove(remove->layer);
                        remove->layer->SetTransparency(remove->alpha);
                    }
                    remove->layer = NULL;
                }

            }
        }
        // Handle fade-in
        for(int i = 0; i < GUI_MAX_LAYERS; i++) {
            FrameAdd *add = &add_list[i];
            if( add->layer ) {
                if( add->delay ) {
                    add->delay--;
                }else
                if( add->count ) {
                    add->count--;
                    if( add->layer->GetTransparency() != add->curalpha ) {
                        // someone has changed the transparency, change as new setpoint for fade
                        add->alpha = add->layer->GetTransparency();
                    }
                    add->curalpha = ((add->fade - add->count) * add->alpha) / add->fade;
                    add->layer->SetTransparency(add->curalpha);
                }else{
                    add->layer = NULL;
                }

            }
        }

        manager->Draw();
        LWP_MutexUnlock(mutex);
        gwd.Flush();
    }while( !quit_thread );
}

void GuiManager::AddRenderCallback(void (*callback)(void*), void *context)
{
    GuiManagerCallback *p, *newrender = new GuiManagerCallback;
    newrender->callback = callback;
    newrender->context = context;
    newrender->next = NULL;
    if( render ) {
        p = render;
        while( p->next ) {
            p = p->next;
        }
        p->next = newrender;
    }else{
        render = newrender;
    }
}

void GuiManager::RemoveRenderCallback(void (*callback)(void*), void *context)
{
    GuiManagerCallback *cur = render;
    GuiManagerCallback *prev = NULL;
    while( cur ) {
        if( cur->callback == callback && cur->context == context ) {
            LWP_MutexLock(mutex);
            if( prev ) {
                prev->next = cur->next;
            }else{
                render = cur->next;
            }
            delete cur;
            LWP_MutexUnlock(mutex);
            return;
        }
        prev = cur;
        cur = cur->next;
    }
}

void GuiManager::Lock(void)
{
    LWP_MutexLock(mutex);
}

void GuiManager::Unlock(void)
{
    LWP_MutexUnlock(mutex);
}

void GuiManager::AddIndex(int index, Layer *layer, bool fix, int fade, int delay)
{
    FrameAdd *add = NULL;
    int i;
    if( fade ) {
        // find free spot in add-list
        for(i = 0; i < GUI_MAX_LAYERS; i++) {
            if( add_list[i].layer == NULL ) break;
        }
        if( i == GUI_MAX_LAYERS ) return;
        add = &add_list[i];
        // set parameters for adding it
        add->count = add->fade = fade;
        add->delay = delay;
        add->alpha = layer->GetTransparency();
        add->curalpha = 0;
        // start at 'fully transparent'
        layer->SetTransparency(add->curalpha);
    }
    if( index < fixed_layers ) {
        manager->Insert(layer, index);
        fixed_layers++;
    }else
    if( fix ) {
        manager->Insert(layer, 0);
        fixed_layers++;
    }else{
        if( index == GUI_MAX_LAYERS ) {
            manager->Append(layer);
        }else{
            manager->Insert(layer, index);
        }
    }
    if( fade ) {
        // start fading-in
        add->layer = layer;
    }
}

void GuiManager::AddTop(Layer *layer, int fade, int delay)
{
    AddIndex(fixed_layers, layer, false, fade, delay);
}

void GuiManager::AddTopFixed(Layer *layer, int fade, int delay)
{
    AddIndex(fixed_layers, layer, true, fade, delay);
}

void GuiManager::AddOnTopOf(Layer *ontopof, Layer *layer, int fade, int delay)
{
    int index = manager->GetIndex(ontopof);
    if( index >= 0 ) {
        AddIndex(index, layer, false, fade, delay);
    }
}

void GuiManager::AddBehind(Layer *behind, Layer *layer, int fade, int delay)
{
    int index = manager->GetIndex(behind);
    if( index >= 0 ) {
        AddIndex(index+1, layer, false, fade, delay);
    }
}

void GuiManager::AddBottom(Layer *layer, int fade, int delay)
{
    AddIndex(GUI_MAX_LAYERS, layer, false, fade, delay);
}

void GuiManager::RegisterRemove(Layer *layer, bool needdelete, int fade, int delay, Image *image)
{
    int i;
    // remove from add-list (still busy fading-in)
    // remove from remove-list to prevent deleting twice
    for(i = 0; i < GUI_MAX_LAYERS; i++) {
        if( add_list[i].layer == layer ) {
            add_list[i].layer = NULL;
        }
        if( remove_list[i].layer == layer ) {
            remove_list[i].layer = NULL;
        }
    }
    // find free spot in remove-list
    for(i = 0; i < GUI_MAX_LAYERS; i++) {
        if( remove_list[i].layer == NULL ) break;
    }
    if( i == GUI_MAX_LAYERS ) return;
    FrameRemove *remove = &remove_list[i];
    // Add entry to list
    remove->needdelete = needdelete;
    remove->count = remove->fade = fade;
    remove->delay = delay;
    remove->alpha = layer->GetTransparency();
    remove->image = image;
    remove->layer = layer;
}

void GuiManager::Remove(Layer *layer, int fade, int delay)
{
    if( fade == 0 ) {
        int idx = manager->GetIndex(layer);
        if( idx < 0 ) return;

        manager->Remove(layer);

        if( idx < fixed_layers ) {
            fixed_layers--;
        }
    }else{
        RegisterRemove(layer, false, fade, delay, NULL);
    }
}

void GuiManager::RemoveAndDelete(Layer *layer, Image *image, int fade, int delay)
{
    if( layer != NULL ) {
        if( fade == 0 ) {
            Remove(layer);
            if( image ) {
                delete image;
            }
            delete layer;
        }else{
            RegisterRemove(layer, true, fade, delay, image);
        }
    }
}

bool GuiManager::GetWiiMoteIR(int *x, int *y, int *angle)
{
    ir_t ir;
    WPAD_IR(WPAD_CHAN_0, &ir);
    if( !ir.state || !ir.smooth_valid ) {
        WPAD_IR(WPAD_CHAN_1, &ir);
    }
    if( !ir.state || !ir.smooth_valid ) {
        return false;
    }
    int sx = (int)(((ir.sx - (500-200)) * gwd.GetWidth()) / 400);
    int sy = (int)(((ir.sy - (500-150)) * gwd.GetHeight()) / 300);
    if( x ) *x = sx;
    if( y ) *y = sy;
    if( angle ) *angle = ir.angle;

    return true;
}

GW_VIDEO_MODE GuiManager::GetMode(void)
{
    return gwd.GetMode();
}

u32 GuiManager::GetWidth(void)
{
    return gwd.GetWidth();
}

u32 GuiManager::GetHeight(void)
{
    return gwd.GetHeight();
}

void GuiManager::SetMode(GW_VIDEO_MODE mode)
{
    gwd.SetMode(mode);
}

void GuiManager::Set240p()
{
    gwd.Set240p();
}

GuiManager::GuiManager()
{
    fixed_layers = 0;
    memset(remove_list, 0, sizeof(remove_list));
    memset(add_list, 0, sizeof(add_list));

    // Initialize GameWindow
    gwd.InitVideo();
    gwd.SetBackground((GXColor){ 0, 0, 0, 255 });

    // Initialize manager
    manager = new LayerManager(GUI_MAX_LAYERS);
    render = NULL;

    // Start displaying
    quit_thread = false;
    LWP_MutexInit(&mutex, 1);
    thread_stack = malloc(GUI_DISP_STACK_SIZE);
    LWP_CreateThread(&thread, DisplayThreadWrapper, this,
                     thread_stack, GUI_DISP_STACK_SIZE, 90);
}

GuiManager::~GuiManager()
{
    // Handle pending delete requests
    LWP_MutexLock(mutex);
    for(int i = 0; i < GUI_MAX_LAYERS; i++) {
        if( remove_list[i].layer != NULL &&
            remove_list[i].needdelete ) {
            RemoveAndDelete(remove_list[i].layer, remove_list[i].image);
        }
    }

    // Stop display thread
    quit_thread = true;
    LWP_JoinThread(thread, NULL);
    LWP_MutexDestroy(mutex);
    free(thread_stack);

    // Clean-up
    GuiManagerCallback *p = render;
    while( p ) {
        GuiManagerCallback *cb = p;
        p = p->next;
        delete cb;
    }

    delete manager;
}

