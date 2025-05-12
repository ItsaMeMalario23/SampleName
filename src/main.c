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
#include <render/render2D.h>
#include <debug/rdebug.h>
#include <debug/memtrack.h>

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

// static bird animation
static vec2f_t frm1[2] = {{0.0f, -0.006f}, {0.0f, -0.003f}};
static vec2f_t frm2[2] = {{0.0f, 0.006f}, {0.0f, 0.003f}};
static vec2f_t frm3[2] = {{0.0f, 0.0f}, {0.0f, 0.0f}};
static animframe_t animation[4] = {{frm1, 4, 2}, {frm2, 8, 2}, {frm3, 16, 2}};

// callback bird animation
static void animateBird(gameobj_t* restrict bird)
{
    static u32 bcounter;

    if (!bird) {
        bcounter = 0;
        return;
    }

    if (bcounter == 0) {
        bird->data[0].y -= 0.006f;
        bird->data[1].y -= 0.003f;
        bcounter++;
        return;
    }

    if (bcounter == 3) {
        bird->data[0].y += 0.006f;
        bird->data[1].y += 0.003f;
        bcounter++;
        return;
    }

    if (++bcounter > 8)
        bcounter = 0;
}

// callback flag animation
static void animateFlag(gameobj_t* restrict flg)
{
    if (!flg)
        return;

    u64 t = SDL_GetTicks();
    u32 k = 0;

    for (asciidata_t* i = flg->data; i < flg->data + flg->len; i++, k++)
        i->y = flagdata[k].pos.y + 1.95f + (sinf(((f32) t / 780.0f) + (i->x * 40.0f)) * FLAG_Y_OFFSET);
}

static animation_t* birdanimation;
gameobj_t* bird;
gameobj_t* fruit[4];

SDL_AppResult SDL_AppInit(void** appstate, int argc, char** argv)
{
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        SDL_Log("Failed to initialized SDL: %s", SDL_GetError());
        return -1;
    }

    rSetDebugBreak(int3);

    renderInitWindow(&context, 0);
    renderer->init(&context);

    // add game objects
    bird = addGameObjectStruct(&birdobj);

    gameobj_t* flag = addGameObjectStruct(&flagobj);

    for (u32 i = 0; i < 4; i++)
       fruit[i] = addGameObjectStruct(cherryobj + i);

    // init keyboard mapping
    setStdKBMapping();
    inputstate = getInputState();

    // init animations
    initAnimationThread();

    birdanimation = addCallbackAnimation(bird, animateBird, 12, ANIM_RESET_POS | ANIM_RESET_CB | ANIM_KEEPALIVE);
    queueAnimation(birdanimation, 0, 0);

    animation_t* flaganimation = addCallbackAnimation(flag, animateFlag, 0, 0);
    queueAnimation(flaganimation, ANIM_REPEAT, 0);

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

    if (bird && inputstate->up)
        bird->dy += 0.192f * dt;

    if (bird && inputstate->left)
        bird->dx -= 0.108f * dt;

    if (bird && inputstate->down)
        bird->dy -= 0.192f * dt;

    if (bird && inputstate->right)
        bird->dx += 0.108f * dt;

    if (fnotzero(bird->dx) || fnotzero(bird->dy))
        birdanimation->numcycles = ANIM_REPEAT;
    else
        birdanimation->numcycles = ANIM_TERMINATE;

    updateGameObjectPos(bird);

    handleCollision();

    renderer->draw(&context);

    prev = t;

    return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void* appstate, SDL_AppResult result)
{
    renderer->cleanup(&context);
    renderCleanupWindow(&context);
    cleanupAnimationThread();
}