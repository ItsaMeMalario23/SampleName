#ifndef INPUT_H
#define INPUT_H

#include <main.h>

#define NUM_KEYS                    400

// mouse input modes
#define MOUSE_DISABLE_ALL           (1u << 0)
#define MOUSE_DISABLE_MOTION        (1u << 0)
#define MOUSE_DISABLE_WHEEL         (1u << 1)
#define MOUSE_DISABLE_BUTTONS       (1u << 2)
#define MOUSE_DISABLE_BUTTON_UP     (1u << 3)
#define MOUSE_SEPARATE_WHEEL        (1u << 4)

// bitmasks to check input mode
#define MMASK_DISABLE_MOTION        (MOUSE_DISABLE_ALL | MOUSE_DISABLE_MOTION)
#define MMASK_DISABLE_WHEEL         (MOUSE_DISABLE_ALL | MOUSE_DISABLE_WHEEL)
#define MMASK_DISABLE_BUTTONS       (MOUSE_DISABLE_ALL | MOUSE_DISABLE_BUTTONS)
#define MMASK_DISABLE_BUTTON_UP     (MOUSE_DISABLE_ALL | MOUSE_DISABLE_BUTTONS | MOUSE_DISABLE_BUTTON_UP)

// Function types
typedef SDL_AppResult (*kbevent_f)(u32);

typedef void (*inputmap_f)(void);

typedef SDL_AppResult (*mmotion_f)(f32, f32);
typedef SDL_AppResult (*mwheel_f)(f32);
typedef SDL_AppResult (*mbutton_f)(u8);

typedef struct instate_s {
    f32 screen_dx;
    f32 screen_dy;
    u64 dynamic;        /* to store lvl specific data or flags */
    u8 up;
    u8 down;
    u8 left;
    u8 right;
    u8 showfps;
} instate_t;

typedef struct mouseinput_s {
    u64         mode;
    mmotion_f   motion;
    mwheel_f    wheel;
    mwheel_f    wheel_i;
    mbutton_f   button;
} mouseinput_t;

instate_t*    getInputState(void);
mouseinput_t* getMouse(void);

void setStdInputMap(void);
void set2DLayerInputMap(void);
void set3DInputMap(void);

SDL_AppResult handleKBInput(SDL_Scancode key, u32 kdown);

#endif