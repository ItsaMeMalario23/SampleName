#include <SDL3/SDL.h>

#include <input.h>
#include <animation.h>
#include <render/render2D.h>
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

    if (show) {
        addHitbox(bird, 0.0f, -BIRD_Y_SCALE);
        addHitbox(fruit[0], 0.0f, 0.0f);
        addHitbox(fruit[1], 0.0f, 0.0f);
        addHitbox(fruit[2], 0.0f, 0.0f);
        addHitbox(fruit[3], 0.0f, 0.0f);
    } else {
        removeHitbox(bird);
        removeHitbox(fruit[0]);
        removeHitbox(fruit[1]);
        removeHitbox(fruit[2]);
        removeHitbox(fruit[3]);
    }

    return SDL_APP_CONTINUE;
}

static SDL_AppResult fps(u32 kdown)
{
    if (kdown)
        state.showfps = !state.showfps;

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