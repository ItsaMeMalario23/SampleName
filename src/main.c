#define SDL_MAIN_USE_CALLBACKS 1

#include <math.h>
#include <stdio.h>
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3/SDL_events.h>
#include <SDL3/SDL_gpu.h>

#include <main.h>
#include <input.h>
#include <objects.h>
#include <animation.h>
#include <render/render.h>
#include <debug/rdebug.h>
#include <debug/memtrack.h>

static context_t context = { .name = "[SampleName]", .width = 1920, .height = 1080 };

context_t* getContext(void)
{
    return &context;
}

bool lifecycleWatchdog(void* userdata, SDL_Event* event)
{
    if (event->type == SDL_EVENT_DID_ENTER_BACKGROUND) {
        SDL_Event evt;
        evt.type = SDL_EVENT_USER;
        evt.user.code = 0;
        SDL_PushEvent(&evt);
    } else if (event->type == SDL_EVENT_WILL_ENTER_BACKGROUND) {
        SDL_Event evt;
        evt.type = SDL_EVENT_USER;
        evt.user.code = 1;
        SDL_PushEvent(&evt);
    }

    return 0;
}

u32 f_int3 = 0;

// interrupt execution on assert fail
void int3(void)
{
    f_int3 = 1;
    SDL_Log(" INT3");
    SDL_SetWindowRelativeMouseMode(context.window, false);
}

static instate_t* input;
static mouseinput_t* mouse;
static f64 frq;
static u64 prev;

//
//  Init
//
SDL_AppResult SDL_AppInit(void** appstate, int argc, char** argv)
{
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        SDL_Log("[ERROR] Failed to initialized SDL: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    rSetDebugBreak(int3);

    renderInit(SDL_WINDOW_BORDERLESS, RENDER_MODE_3D);

    if (renderGetMode() == RENDER_MODE_UNDEF) {
        SDL_Log("[ERROR] Failed to initialize renderer");
        return SDL_APP_FAILURE;
    }

    initAnimationThread();

    input = getInputState();
    mouse = getMouse();

    if (startLevel(LVL_ID_ODYSSEY)) {
        SDL_Log("[ERROR] Failed to start level");
        return SDL_APP_FAILURE;
    }

    SDL_AddEventWatch(lifecycleWatchdog, NULL);

    frq = (f64) SDL_GetPerformanceFrequency();
    prev = SDL_GetPerformanceCounter();

    return SDL_APP_CONTINUE;
}

//
//  Event
//
SDL_AppResult SDL_AppEvent(void* appstate, SDL_Event* event)
{
    switch (event->type) {
    case SDL_EVENT_QUIT:
        return SDL_APP_SUCCESS;

    // keyboard
    case SDL_EVENT_KEY_DOWN:
        return handleKBInput(event->key.scancode, 1);

    case SDL_EVENT_KEY_UP:
        return handleKBInput(event->key.scancode, 0);

    // mouse
    case SDL_EVENT_MOUSE_MOTION:
        if (!(mouse->mode & MMASK_DISABLE_MOTION) && mouse->motion)
            return mouse->motion(event->motion.xrel, event->motion.yrel);
        else
            return SDL_APP_CONTINUE;

    case SDL_EVENT_MOUSE_WHEEL:
        if (!(mouse->mode & MMASK_DISABLE_WHEEL) && mouse->wheel) {
            if (!(mouse->mode & MOUSE_SEPARATE_WHEEL) || event->wheel.y < 0.0f)
                return mouse->wheel(event->wheel.y);
            else
                return mouse->wheel_i(event->wheel.y);
            }
        else
            return SDL_APP_CONTINUE;

    case SDL_EVENT_MOUSE_BUTTON_DOWN:
        if (!(mouse->mode & MMASK_DISABLE_BUTTONS) && mouse->button)
            return mouse->button(event->button.button);
        else
            return SDL_APP_CONTINUE;

    case SDL_EVENT_MOUSE_BUTTON_UP:
        if (!(mouse->mode & MMASK_DISABLE_BUTTON_UP) && mouse->button)
            return mouse->button(event->button.button);
        else
            return SDL_APP_CONTINUE;

    default:
        return SDL_APP_CONTINUE;
    }

    return SDL_APP_CONTINUE;
}

//
//  Update
//
SDL_AppResult SDL_AppIterate(void* appstate)
{
    static u32 counter;

    if (f_int3) {
        return SDL_APP_CONTINUE;
    }

    u64 t = SDL_GetPerformanceCounter();
    f64 dt = ((f64) t - (f64) prev) / (f64) frq;

    if (input->showfps && ++counter >= 400) {
        SDL_Log("FPS %.2f", 1.0 / dt);
        counter = 0;
    }

    if (gameUpdate(dt))
        return SDL_APP_SUCCESS;

    renderDraw(&context);

    prev = t;

    return SDL_APP_CONTINUE;
}

//
//  Exit
//
void SDL_AppQuit(void* appstate, SDL_AppResult result)
{
    renderCleanup();
    cleanupAnimationThread();
}