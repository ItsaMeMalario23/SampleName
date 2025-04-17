#ifndef COM_H
#define COM_H

#include <main.h>

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
} vec3f_t;

typedef struct mat4x4_s {
    f32 m11, m12, m13, m14;
    f32 m21, m22, m23, m24;
    f32 m31, m32, m33, m34;
    f32 m41, m42, m43, m44;
} mat4x4_t;

typedef struct vertPosCol_s {
    f32 x, y, z;
    u8 r, g, b, a;
} vertPosCol_t;

bool comInit(context_t* context, SDL_WindowFlags flags);
bool comCleanup(context_t* context);

SDL_GPUShader* comLoadShader(SDL_GPUDevice* dev, const char* filename, u32 samplerCount, u32 uniformBufCount, u32 storageBufCount, u32 storageTexCount);
SDL_Surface* comLoadBmp(const char* filename);

mat4x4_t matMultiply(mat4x4_t m1, mat4x4_t m2);
mat4x4_t rotationMatZ(f32 rad);
mat4x4_t translationMat(f32 dx, f32 dy, f32 dz);
vec3f_t normalizeVec3(vec3f_t v);
vec3f_t crossVec3(vec3f_t a, vec3f_t b);
f32 dotVec3(vec3f_t a, vec3f_t b);

#endif