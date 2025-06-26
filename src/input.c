#include <SDL3/SDL.h>

#include <input.h>
#include <animation.h>
#include <render/camera.h>
#include <render/render.h>
#include <debug/rdebug.h>

//
//  Input state
//
static instate_t state;
static mouseinput_t mouse;

instate_t* getInputState(void)
{
    return &state;
}

mouseinput_t* getMouse(void)
{
    return &mouse;
}

//
//  Input callbacks
//
static SDL_AppResult nop(u32 kdown)
{
    return SDL_APP_CONTINUE;
}

static SDL_AppResult quit(u32 kdown)
{
    return SDL_APP_SUCCESS;
}

static SDL_AppResult up(u32 kdown)
{
    state.up = kdown;
    return SDL_APP_CONTINUE;
}

static SDL_AppResult down(u32 kdown)
{
    state.down = kdown;
    return SDL_APP_CONTINUE;
}

static SDL_AppResult left(u32 kdown)
{
    state.left = kdown;
    return SDL_APP_CONTINUE;
}

static SDL_AppResult right(u32 kdown)
{
    state.right = kdown;
    return SDL_APP_CONTINUE;
}

static SDL_AppResult dbreak(u32 kdown)
{
    if (kdown)
        rAssert(0);

    return SDL_APP_CONTINUE;
}

static SDL_AppResult hitboxes(u32 kdown)
{
    if (kdown)
        renderToggleHitboxes();

    return SDL_APP_CONTINUE;
}

static SDL_AppResult fps(u32 kdown)
{
    if (kdown)
        state.showfps = !state.showfps;

    return SDL_APP_CONTINUE;
}

static SDL_AppResult lvlexit(u32 kdown)
{
    if (kdown)
        state.dynamic = 1;

    return SDL_APP_CONTINUE;
}

static SDL_AppResult odysseymode(u32 kdown)
{
    if (kdown)
        state.dynamic = 2;

    return SDL_APP_CONTINUE;
}

static SDL_AppResult thirdperson(u32 kdown)
{
    if (kdown)
        state.dynamic = 3;

    return SDL_APP_CONTINUE;
}

//
//  Layer mode
//
static SDL_AppResult scrleft(u32 kdown)
{
    if (kdown)
        state.screen_dx -= 0.02f;

    return SDL_APP_CONTINUE;
}

static SDL_AppResult scrright(u32 kdown)
{
    if (kdown)
        state.screen_dx += 0.02f;

    return SDL_APP_CONTINUE;
}

static SDL_AppResult scrup(u32 kdown)
{
    if (kdown)
        state.screen_dy += 0.02f;

    return SDL_APP_CONTINUE;
}

static SDL_AppResult scrdown(u32 kdown)
{
    if (kdown)
        state.screen_dy -= 0.02f;
    
    return SDL_APP_CONTINUE;
}

//
//  Keyboard mappings
//
static kbevent_f kbmap[NUM_KEYS];

void setStdInputMap(void)
{
    kbmap[SDL_SCANCODE_ESCAPE] = quit;
    kbmap[SDL_SCANCODE_Q] = quit;

    kbmap[SDL_SCANCODE_0] = dbreak;
    kbmap[SDL_SCANCODE_H] = hitboxes;
    kbmap[SDL_SCANCODE_F] = fps;
    kbmap[SDL_SCANCODE_X] = lvlexit;

    kbmap[SDL_SCANCODE_UP] = up;
    kbmap[SDL_SCANCODE_W] = up;

    kbmap[SDL_SCANCODE_LEFT] = left;
    kbmap[SDL_SCANCODE_A] = left;
    
    kbmap[SDL_SCANCODE_DOWN] = down;
    kbmap[SDL_SCANCODE_S] = down;

    kbmap[SDL_SCANCODE_RIGHT] = right;
    kbmap[SDL_SCANCODE_D] = right;
}

void set2DLayerInputMap(void)
{
    kbmap[SDL_SCANCODE_ESCAPE] = quit;
    kbmap[SDL_SCANCODE_Q] = quit;

    kbmap[SDL_SCANCODE_0] = dbreak;
    kbmap[SDL_SCANCODE_F] = fps;
    kbmap[SDL_SCANCODE_X] = lvlexit;

    kbmap[SDL_SCANCODE_LEFT] = scrright;
    kbmap[SDL_SCANCODE_RIGHT] = scrleft;
    kbmap[SDL_SCANCODE_DOWN] = scrup;
    kbmap[SDL_SCANCODE_UP] = scrdown;

    kbmap[SDL_SCANCODE_W] = up;
    kbmap[SDL_SCANCODE_A] = left;
    kbmap[SDL_SCANCODE_S] = down;
    kbmap[SDL_SCANCODE_D] = right;
}

void set3DInputMap(void)
{
    kbmap[SDL_SCANCODE_ESCAPE] = quit;
    kbmap[SDL_SCANCODE_Q] = quit;

    kbmap[SDL_SCANCODE_0] = dbreak;
    kbmap[SDL_SCANCODE_F] = fps;
    kbmap[SDL_SCANCODE_X] = lvlexit;

    kbmap[SDL_SCANCODE_W] = up;
    kbmap[SDL_SCANCODE_A] = left;
    kbmap[SDL_SCANCODE_S] = down;
    kbmap[SDL_SCANCODE_D] = right;

    kbmap[SDL_SCANCODE_M] = odysseymode;
    kbmap[SDL_SCANCODE_T] = thirdperson;
}

void set3DAirInputMap(void)
{
    kbmap[SDL_SCANCODE_ESCAPE] = quit;
    kbmap[SDL_SCANCODE_Q] = quit;

    kbmap[SDL_SCANCODE_0] = dbreak;
    kbmap[SDL_SCANCODE_F] = fps;
    kbmap[SDL_SCANCODE_X] = lvlexit;

    kbmap[SDL_SCANCODE_W] = up;
    kbmap[SDL_SCANCODE_A] = left;
    kbmap[SDL_SCANCODE_S] = down;
    kbmap[SDL_SCANCODE_D] = right;

    kbmap[SDL_SCANCODE_T] = thirdperson;
}

SDL_AppResult handleKBInput(SDL_Scancode key, u32 kdown)
{
    // handle interrupt
    if (f_int3) {
        if (key == SDL_SCANCODE_RETURN) {
            f_int3 = 0;
            SDL_Log(" UNPAUSE");
            return SDL_APP_CONTINUE;
        } else if (key == SDL_SCANCODE_ESCAPE || key == SDL_SCANCODE_Q) {
            return SDL_APP_SUCCESS;
        }
    }

    if (key < NUM_KEYS && kbmap[key])
        return kbmap[key](kdown);

    return SDL_APP_CONTINUE;
}

// for MSAA
/* static SDL_AppResult incsamples(u32 kdown)
{
    if (kdown && renderSamples < 3) {
        renderSamples++;
        SDL_Log("samples: %d", renderSamples);
    }

    return SDL_APP_CONTINUE;
}

static SDL_AppResult decsamples(u32 kdown)
{
    if (kdown && renderSamples) {
        renderSamples--;
        SDL_Log("samples: %d", renderSamples);
    }

    return SDL_APP_CONTINUE;
} */