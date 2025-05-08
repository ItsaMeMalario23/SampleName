#ifndef INPUT_H
#define INPUT_H

#include <main.h>

#define NUM_KEYS    400

typedef struct instate_s {
    u8 up;
    u8 down;
    u8 left;
    u8 right;
} instate_t;

typedef SDL_AppResult (*kbevent_f)(u32);

instate_t* getInputState(void);

void setStdKBMapping(void);

SDL_AppResult handleKBInput(SDL_Scancode key, u32 kdown);

#endif