#define SDL_MAIN_USE_CALLBACKS 1

#include <stdio.h>
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3/SDL_events.h>
#include <SDL3/SDL_gpu.h>

#include <main.h>
//#include <com.h>
//#include <render.h>
//#include <basicvertex.h>
#include <render/rendercom.h>
#include <debug/rdebug.h>
#include <debug/memtrack.h>

static context_t context = {0};

static renderer_t* renderer = &r_ascii2D;

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

SDL_AppResult SDL_AppInit(void** appstate, int argc, char** argv)
{
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        SDL_Log("Failed to initialized SDL: %s", SDL_GetError());
        return -1;
    }

    renderInit(&context, 0);

    renderer->init(&context);

    SDL_AddEventWatch(lifecycleWatchdog, NULL);

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
        if (event->key.key == SDLK_W)
        {
            renderer->update(0);
        }
        else if (event->key.key == SDLK_S)
        {
            renderer->update(1);
        }
        else if (event->key.key == SDLK_R)
        {
            renderer->update(2);
        }
        else if (event->key.key == SDLK_ESCAPE || event->key.key == SDLK_Q)
        {
            return SDL_APP_SUCCESS;
        }
    }

    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppIterate(void* appstate)
{
    renderer->draw(&context);

    return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void* appstate, SDL_AppResult result)
{
    renderCleanup(&context);
    renderer->cleanup(&context);
}