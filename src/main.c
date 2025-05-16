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
#include <render/renderer.h>
#include <debug/rdebug.h>
#include <debug/memtrack.h>

static context_t context = { .name = "[SampleName]", .width = 1920, .height = 1080 };

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
}

static instate_t* inputstate;
static f64 frq;
static u64 prev;

SDL_AppResult SDL_AppInit(void** appstate, int argc, char** argv)
{
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        SDL_Log("Failed to initialized SDL: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    rSetDebugBreak(int3);

    renderInit(&context, SDL_WINDOW_BORDERLESS, RENDER_MODE_LAYERED);

    if (getRenderMode() == RENDER_MODE_UNDEF) {
        SDL_Log("Failed to initialize renderer");
        return SDL_APP_FAILURE;
    }

    initAnimationThread();

    loadScene(&scene1);

    inputstate = getInputState();
    inputstate->screen_dx = 0.0f;
    inputstate->screen_dy = -0.14f;

    SDL_AddEventWatch(lifecycleWatchdog, NULL);

    frq = (f64) SDL_GetPerformanceFrequency();
    prev = SDL_GetPerformanceCounter();

    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void* appstate, SDL_Event* event)
{
    if (event->type == SDL_EVENT_QUIT)
    {
        return SDL_APP_SUCCESS;
    }
    else if (event->type == SDL_EVENT_KEY_DOWN)
    {
        return handleKBInput(event->key.scancode, 1);
    }
    else if (event->type == SDL_EVENT_KEY_UP)
    {
        return handleKBInput(event->key.scancode, 0);
    }

    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppIterate(void* appstate)
{
    static u32 counter;

    if (f_int3) {
        return SDL_APP_CONTINUE;
    }

    u64 t = SDL_GetPerformanceCounter();
    f64 dt = ((f64) t - (f64) prev) / (f64) frq;

    if (inputstate->showfps && ++counter >= 400) {
        SDL_Log("FPS %.2f", 1.0 / dt);
        counter = 0;
    }

    movePlayer(dt);

    renderDraw(&context);

    prev = t;

    return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void* appstate, SDL_AppResult result)
{
    renderCleanup(&context);
    cleanupAnimationThread();
}