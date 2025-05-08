#define SDL_MAIN_USE_CALLBACKS 1

#include <stdio.h>
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3/SDL_events.h>
#include <SDL3/SDL_gpu.h>

#include <main.h>
#include <input.h>
#include <animation.h>
#include <render/render2D.h>
#include <debug/rdebug.h>
#include <debug/memtrack.h>

const ascii2info_t bird2[16] = {
    {'(', 0xffffffff, {-0.0187500, -0.0694444}}, {'@', 0xfcd303ff, {0.0046875, -0.0722222}}, {'-', 0xffffffff, {0.0484375, -0.0305556}}, {'\\', 0xffffffff, {0.0734375, -0.0500000}},
    {'\\', 0xffffffff, {0.0031250, -0.1361111}}, {')', 0xffffffff, {0.0265625, -0.0750000}}, {'O', 0xffffffff, {0.0500000, -0.0611111}}, {'>', 0xff0000ff, {0.0859375, -0.0944444}},
    {'\'', 0xff, {0.0515625, -0.0722222}}, {'_', 0xffffffff, {0.0281250, -0.1333333}}, {'_', 0xffffffff, {0.0515625, -0.1333333}}, {'/', 0xffffffff, {0.0765625, -0.1333333}},
    {'>', 0xff0000ff, {0.0718750, -0.0944444}}, {'-', 0xffffffff, {0.0218750, -0.0305556}}, {'B', 0xfcd303ff, {0.0265625, -0.1222222}}, {'D', 0xfcd303ff, {0.0531250, -0.1222222}},
};

const ascii2info_t cherry[6] = {
    {'@', 0xff0000ff, {0.0484375f, -0.1305556f}}, {')', 0xff0000ff, {0.0656250f, -0.1333333f}}, {'(', 0xff0000ff, {0.0296875f, -0.1333333f}}, {'-', 0xff0000ff, {0.0468750f, -0.1583333f}},
    {',', 0x40d800ff, {0.0531250f, -0.0833333f}}, {'.', 0x40d800ff, {0.0562500f, -0.0694444f}},
};

const ascii2info_t cherry2[6] = {
    {'O', 0xff0000ff, {0.0218750f, 0.0166667f}}, {'-', 0xff0000ff, {0.0203125f, -0.0166667f}}, {')', 0xff0000ff, {0.0421875f, 0.0083333f}}, {'(', 0xff0000ff, {0.0000000f, 0.0083333f}},
    {',', 0x40d800ff, {0.0265625f, 0.0694444f}}, {'.', 0x40d800ff, {0.0250000f, 0.0888889f}},
};

static context_t context = { .name = "[SampleName]", .width = 1280, .height = 720 };
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

static u64 frq;
static u64 prev;

static vec2f_t frm1[2] = {{0.0f, -0.006f}, {0.0f, -0.003f}};
static vec2f_t frm2[2] = {{0.0f, 0.006f}, {0.0f, 0.003f}};
static vec2f_t frm3[2] = {{0.0f, 0.0f}, {0.0f, 0.0f}};
static animframe_t animation[4] = {{frm1, 4, 2}, {frm2, 6, 2}, {frm3, 20, 2}};

animation_t* birdanimation;
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

    bird = addGameObject(bird2, sizeof(bird2) / sizeof(ascii2info_t), 0.25f, 0.25f);
    
    //fruit[0] = addGameObject(cherry2, 6, 0.0f, 0.0f);
    fruit[0] = addGameObject(cherry2, 6, 0.8f, 0.3f);
    fruit[1] = addGameObject(cherry2, 6, 1.5f, 1.7f);
    fruit[2] = addGameObject(cherry2, 6, 1.0f, 1.8f);
    fruit[3] = addGameObject(cherry2, 6, 1.2f, 1.0f);
    
    frq = SDL_GetPerformanceFrequency();
    prev = SDL_GetPerformanceCounter();

    setStdKBMapping();
    inputstate = getInputState();

    initAnimationThread();
    birdanimation = addAnimation(bird, animation, 3, 1, ANIM_SUSPENDED);
    birdanimation->keep = 1;

    if (!birdanimation)
        SDL_Log("ERROR: Bird animation null ref");

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
        return handleKBInput(event->key.scancode, 1);
    }
    else if (event->type == SDL_EVENT_KEY_UP)
    {
        return handleKBInput(event->key.scancode, 0);
    }

    return SDL_APP_CONTINUE;
}

static u32 counter;

SDL_AppResult SDL_AppIterate(void* appstate)
{
    if (f_int3) {
        return SDL_APP_CONTINUE;
    }

    u64 t = SDL_GetPerformanceCounter();
    f64 dt = ((f64) t - (f64) prev) / (f64) frq;

    if (++counter >= 400) {
        //SDL_Log("FPS %.2f", 1.0 / dt);
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

    updateGameObjectPos(bird);

    setHitbox(bird->x - 1.0f, bird->y - 1.0f, (1.0f / 12.0f), (1.0f / 8.0f));

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