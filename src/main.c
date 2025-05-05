#define SDL_MAIN_USE_CALLBACKS 1

#include <stdio.h>
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3/SDL_events.h>
#include <SDL3/SDL_gpu.h>

#include <main.h>
#include <input.h>
#include <render/render2D.h>
#include <debug/rdebug.h>
#include <debug/memtrack.h>

const ascii2info_t bird[17] = {
    {'\\', WHITE, {  2, 33}}, {'-', WHITE, {17, -5}}, {'-', WHITE, {31, -5}}, {'\\', WHITE, {47,  2}},
    { '(', WHITE, {-12,  9}}, {'@',  GOLD, { 3, 10}}, {'O', WHITE, {32,  6}}, { '>',   RED, {55, 18}},
    {'\'', BLACK, { 33, 10}}, {'_', WHITE, {18, 32}}, {'_', WHITE, {33, 32}}, { '/', WHITE, {49, 32}},
    { '>',   RED, { 46, 18}}, {')', WHITE, {17, 11}}, {'B',  GOLD, {17, 28}}, { 'D',  GOLD, {34, 28}},
    { '.', WHITE, {  9, -9}},
};

static context_t context = { .name = "[SampleName]", .width = 1920, .height = 1080 };
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

static gameobj_t* obj;
static instate_t* inputstate;

static u64 frq;
static u64 prev;

SDL_AppResult SDL_AppInit(void** appstate, int argc, char** argv)
{
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        SDL_Log("Failed to initialized SDL: %s", SDL_GetError());
        return -1;
    }

    renderInitWindow(&context, 0);

    renderer->init(&context);

    obj = addGameObject(bird, sizeof(bird) / sizeof(ascii2info_t), 0.25f, 0.25f);

    frq = SDL_GetPerformanceFrequency();
    prev = SDL_GetPerformanceCounter();

    inputstate = getInputState();

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
        return handleKBInput(event->key.key, 1);
    }
    else if (event->type == SDL_EVENT_KEY_UP)
    {
        return handleKBInput(event->key.key, 0);
    }

    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppIterate(void* appstate)
{
    u64 t = SDL_GetPerformanceCounter();
    f64 dt = ((f64) t - (f64) prev) / (f64) frq;

    if (obj && inputstate->up)
        obj->y += 0.192f * dt;

    if (obj && inputstate->left)
        obj->x -= 0.108f * dt;

    if (obj && inputstate->down)
        obj->y -= 0.192f * dt;

    if (obj && inputstate->right)
        obj->x += 0.108f * dt;

    updateGameObjectPos(obj);

    renderer->draw(&context);

    prev = t;

    return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void* appstate, SDL_AppResult result)
{
    renderer->cleanup(&context);
    renderCleanupWindow(&context);
}