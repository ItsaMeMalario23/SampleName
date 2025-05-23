#include <SDL3/SDL.h>

#include <input.h>
#include <animation.h>
#include <render/renderer.h>
#include <debug/rdebug.h>

//
//  input state
//
static instate_t state;

instate_t* getInputState(void)
{
    return &state;
}

//
//  input callbacks
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
    static u32 show;

    if (!kdown)
        return SDL_APP_CONTINUE;

    show = !show;

    /*
    if (show) {
        addHitbox(bird);
        addHitbox(fruit[0]);
        addHitbox(fruit[1]);
        addHitbox(fruit[2]);
        addHitbox(fruit[3]);
    } else {
        removeHitbox(bird);
        removeHitbox(fruit[0]);
        removeHitbox(fruit[1]);
        removeHitbox(fruit[2]);
        removeHitbox(fruit[3]);
    }
    */

    return SDL_APP_CONTINUE;
}

static SDL_AppResult fps(u32 kdown)
{
    if (kdown)
        state.showfps = !state.showfps;

    return SDL_APP_CONTINUE;
}

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
//  keyboard mapping
//
static kbevent_f kbmap[NUM_KEYS];

void setStdKBMapping(void)
{
    kbmap[SDL_SCANCODE_UP] = up;
    kbmap[SDL_SCANCODE_W] = up;

    kbmap[SDL_SCANCODE_LEFT] = left;
    kbmap[SDL_SCANCODE_A] = left;
    
    kbmap[SDL_SCANCODE_DOWN] = down;
    kbmap[SDL_SCANCODE_S] = down;

    kbmap[SDL_SCANCODE_RIGHT] = right;
    kbmap[SDL_SCANCODE_D] = right;

    kbmap[SDL_SCANCODE_ESCAPE] = quit;
    kbmap[SDL_SCANCODE_Q] = quit;

    kbmap[SDL_SCANCODE_0] = dbreak;
    kbmap[SDL_SCANCODE_H] = hitboxes;
    kbmap[SDL_SCANCODE_F] = fps;

    kbmap[SDL_SCANCODE_U] = scrleft;
    kbmap[SDL_SCANCODE_I] = scrright;
    kbmap[SDL_SCANCODE_J] = scrdown;
    kbmap[SDL_SCANCODE_K] = scrup;
}

SDL_AppResult handleKBInput(SDL_Scancode key, u32 kdown)
{
    // handle interrupt
    if (f_int3) {
        if (key == SDL_SCANCODE_RETURN) {
            f_int3 = 0;
            SDL_Log(" UNPAUSE");
        }

        return SDL_APP_CONTINUE;
    }

    if (key < NUM_KEYS && kbmap[key])
        return kbmap[key](kdown);

    return SDL_APP_CONTINUE;
}