#include <SDL3/SDL.h>

#include <input.h>
#include <animation.h>
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
    
    if (kdown)
        birdanimation->numcycles = ANIM_REPEAT;
    else
        birdanimation->numcycles = ANIM_SUSPENDED;

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