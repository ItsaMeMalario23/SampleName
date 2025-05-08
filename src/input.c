#include <SDL3/SDL.h>

#include <input.h>
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

//
//  keyboard mapping
//

static kbevent_f kbmap[NUM_KEYS] = {up, left, down, right, up, left, down, right, quit, quit};

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

/*
SDL_AppResult handleKBInput(SDL_Scancode key, u16 kdown, u16 int3)
{
    if (key >= 400)
        return SDL_APP_CONTINUE;

    SDL_Log("Key ID %ld", key);

    switch (key) {
    case SDLK_UP:
        return kbmap[KEY_UP](kdown);

    case SDLK_LEFT:
        return kbmap[KEY_LEFT](kdown);

    case SDLK_DOWN:
        return kbmap[KEY_DOWN](kdown);

    case SDLK_RIGHT:
        return kbmap[KEY_RIGHT](kdown);

    case SDLK_W:
        return kbmap[KEY_W](kdown);

    case SDLK_A:
        return kbmap[KEY_A](kdown);

    case SDLK_S:
        return kbmap[KEY_S](kdown);

    case SDLK_D:
        return kbmap[KEY_D](kdown);

    case SDLK_Q:
        return kbmap[KEY_Q](kdown);

    case SDLK_ESCAPE:
        return kbmap[KEY_ESCAPE](kdown);
    }

    return SDL_APP_CONTINUE;
}
*/