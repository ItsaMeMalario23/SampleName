#ifndef MAIN_H
#define MAIN_H

#include <SDL3/SDL.h>

#define R_DEBUG
#define RDEBUG_BREAK_CUSTOM

#define BLACK   0x000000ff
#define WHITE   0xffffffff
#define GOLD    0xfcd303ff
#define RED     0xff0000ff
#define LGREEN  0x40d800ff
#define MGREEN  0x0d7000ff
#define DGREEN  0x003000ff

#define EPSILON         (0.0000001f)

#define BIRD_X_SCALE    (0.1f)
#define BIRD_Y_SCALE    (0.13333333f)

#define CHERRY_X_SCALE  (0.06f)
#define CHERRY_Y_SCALE  (0.10f)

#define FLAG_Y_OFFSET   (0.006f)

typedef int8_t i8;
typedef uint8_t u8;

typedef int16_t i16;
typedef uint16_t u16;

typedef int32_t i32;
typedef uint32_t u32;

typedef int64_t i64;
typedef uint64_t u64;

typedef float f32;
typedef double f64;

typedef struct animation_s animation_t;

typedef struct vec2f_s {
    f32 x;
    f32 y;
} vec2f_t;

typedef struct vec3f_s {
    f32 x;
    f32 y;
    f32 z;
    u32 pad;    /* padding for 64-bit alignment */
} vec3f_t;

typedef struct mat4_s {
    f32 m11, m12, m13, m14;
    f32 m21, m22, m23, m24;
    f32 m31, m32, m33, m34;
    f32 m41, m42, m43, m44;
} mat4_t;

typedef struct asciidata_s {
    f32 r, g, b, a;
    f32 x, y;
    f32 scale;
    u32 charID;
} asciidata_t;

typedef struct gameobj_s {
    f32                 dx, dy;        /* only used to TEMPORARILY store translations */
    f32                 x, y;          /* global coordinates of obj origin, should NOT be modified directly */
    f32                 xscale;
    f32                 yscale;
    f32                 hitbox_dx;     /* optional hitbox offset */
    f32                 hitbox_dy;
    u32                 len;
    u16                 id;
    u16                 visible;
    asciidata_t*        data;
    animation_t*        animation;
} gameobj_t;

extern u32 f_int3;

static inline bool fnotzero(f32 f) { return f > EPSILON || f < -EPSILON; }

#endif