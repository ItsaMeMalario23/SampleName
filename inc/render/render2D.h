#ifndef RENDER2D_H
#define RENDER2D_H

#include <main.h>

#define RENDERTYPE_UNDEF    0
#define RENDERTYPE_2D       1
#define RENDERTYPE_3D       2

#define EPSILON (0.0000001f)

typedef void (*render_f)(context_t*);

typedef struct renderer_s {
    u64      type;
    render_f init;
    render_f draw;
    render_f cleanup;
} renderer_t;

extern renderer_t r_ascii2D;

void renderInitWindow(context_t* restrict con, SDL_WindowFlags flags);
void renderInitPipeline(context_t* restrict con);
void renderDraw(context_t* restrict con);
void renderCleanupWindow(context_t* restrict con);
void renderCleanupPipeline(context_t* restrict con);
SDL_GPUShader* renderLoadShader(context_t* restrict con, const char* restrict filename, u32 samplerCnt, u32 uniBufCnt, u32 stoBufCnt, u32 stoTexCnt);
SDL_Surface* renderLoadBmp(context_t* restrict con, const char* restrict filename);

gameobj_t* addGameObject(const ascii2info_t* info, u32 len, f32 x, f32 y);
void moveGameObject(const gameobj_t* restrict obj, f32 dx, f32 dy);
void updateGameObjectPos(gameobj_t* restrict obj);
void removeGameObject(gameobj_t* restrict obj);
void resetAllGameObjects(void);

mat4_t multiplyMat4(mat4_t m1, mat4_t m2);
mat4_t rotationMatZ(f32 rad);
mat4_t translationMat(f32 dx, f32 dy, f32 dz);

vec3f_t normalizeVec3(vec3f_t v);
vec3f_t crossVec3(vec3f_t a, vec3f_t b);

f32 dotVec3(vec3f_t a, vec3f_t b);

#endif