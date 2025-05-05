#include <SDL3/SDL.h>

#include <input.h>

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

//
//  keyboard mapping
//

static kbevent_f kbmap[NUM_KEYS] = {up, left, down, right, up, left, down, right, quit, quit};

SDL_AppResult handleKBInput(SDL_Keycode key, u32 kdown)
{
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