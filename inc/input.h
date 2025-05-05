#ifndef INPUT_H
#define INPUT_H

#include <main.h>

#define KEY_UP      0
#define KEY_LEFT    1
#define KEY_DOWN    2
#define KEY_RIGHT   3
#define KEY_W       4
#define KEY_A       5
#define KEY_S       6
#define KEY_D       7
#define KEY_Q       8
#define KEY_ESCAPE  9

#define NUM_KEYS    10

typedef SDL_AppResult (*kbevent_f)(u32);

typedef struct instate_s {
    u8 up;
    u8 down;
    u8 left;
    u8 right;
} instate_t;

instate_t* getInputState(void);

SDL_AppResult handleKBInput(SDL_Keycode key, u32 kdown);

#endif