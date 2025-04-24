#ifndef RENDERCOM_H
#define RENDERCOM_H

#include <main.h>

#define RENDERTYPE_UNDEF    0
#define RENDERTYPE_2D       1
#define RENDERTYPE_3D       2

typedef struct context_s {
    const char*     name;
    const char*     path;
    SDL_Window*     window;
    SDL_GPUDevice*  dev;
    f32             dt;
} context_t;

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

typedef struct vertex_s {
    f32 x, y, z;
    u8  r, g, b, a;
} vertex_t;

typedef struct texvertex_s {
    f32 x, y, z;
    f32 u, v;
} texvertex_t;

typedef struct renderer_s {
    u64 type;
    void (*init)(context_t* con);
    void (*update)(u32 mode);
    void (*draw)(context_t* con);
    void (*cleanup)(context_t* con);
} renderer_t;

void renderInit(context_t* con, SDL_WindowFlags flags);
void renderCleanup(context_t* con);

SDL_GPUShader* renderLoadShader(SDL_GPUDevice* dev, const char* filename, u32 samplerCnt, u32 uniBufCnt, u32 stoBufCnt, u32 stoTexCnt);
SDL_Surface* renderLoadBmp(const char* filename);

mat4_t multiplyMat4(mat4_t m1, mat4_t m2);
mat4_t rotationMatZ(f32 rad);
mat4_t translationMat(f32 dx, f32 dy, f32 dz);

vec3f_t normalizeVec3(vec3f_t v);
vec3f_t crossVec3(vec3f_t a, vec3f_t b);

f32 dotVec3(vec3f_t a, vec3f_t b);

extern renderer_t r_test;
extern renderer_t r_ascii2D;
extern renderer_t r_textest;
extern renderer_t r_instanced;

#endif