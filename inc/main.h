#ifndef MAIN_H
#define MAIN_H

#include <SDL3/SDL.h>

#define R_DEBUG
#define RDEBUG_BREAK_EXIT

#define BLACK   0x000000ff
#define WHITE   0xffffffff
#define GOLD    0xfcd303ff
#define RED     0xff0000ff
#define LGREEN  0x40d800ff
#define MGREEN  0x0d7000ff
#define DGREEN  0x003000ff

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

typedef struct context_s {
    const char*     name;
    const char*     path;
    SDL_Window*     window;
    SDL_GPUDevice*  dev;
    u32             width;
    u32             height;
} context_t;

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

typedef struct ascii2info_s {
    u32     charID;
    u32     color;
    vec2f_t pos;
} ascii2info_t;

#define ASCII2D_BUF_SIZE 1024

typedef struct asciidata_s {
    f32 r, g, b, a;
    f32 x, y;
    f32 scale;
    u32 charID;
} asciidata_t;

#define ASCII2D_OBJ_BUF_SIZE 32

typedef struct gameobj_s {
    f32          x, y;
    u64          len;
    asciidata_t* data;
} gameobj_t;

#endif