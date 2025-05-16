#ifndef INPUT_H
#define INPUT_H

#include <main.h>

#define NUM_KEYS    400

typedef SDL_AppResult (*kbevent_f)(u32);

typedef void (*inputmap_f)(void);

typedef struct instate_s {
    f32 screen_dx;
    f32 screen_dy;
    u8 up;
    u8 down;
    u8 left;
    u8 right;
    u8 showfps;
} instate_t;

instate_t* getInputState(void);

void setStdKBMapping(void);

SDL_AppResult handleKBInput(SDL_Scancode key, u32 kdown);

#endif