#ifndef RENDER2D_H
#define RENDER2D_H

#include <main.h>

#define RENDERTYPE_UNDEF    0
#define RENDERTYPE_2D       1
#define RENDERTYPE_3D       2

#define HITBOX_BUF_SIZE     8

#define CLEAR_COLOR_R   ( 24.0f / 255.0f)
#define CLEAR_COLOR_G   ( 32.0f / 255.0f)
#define CLEAR_COLOR_B   ( 84.0f / 255.0f)
#define CLEAR_COLOR_A   (1.0f)

typedef struct hitbox_s {
    const gameobj_t* object;
    f32 dx;
    f32 dy;
} hitbox_t;

extern renderer_t r_ascii2D;
extern SDL_Mutex* renderBufLock;
extern u32 renderSamples;

void renderInitWindow(context_t* restrict con, SDL_WindowFlags flags);
void renderInitPipelines(context_t* restrict con);
void renderDraw(context_t* restrict con);
void renderCleanupWindow(context_t* restrict con);
void renderCleanupPipelines(context_t* restrict con);

SDL_GPUShader* renderLoadShader(context_t* restrict con, const char* restrict filename, u32 samplerCnt, u32 uniBufCnt, u32 stoBufCnt, u32 stoTexCnt);
SDL_Surface* renderLoadBmp(context_t* restrict con, const char* restrict filename);

void addHitbox(const gameobj_t* restrict object, f32 dx, f32 dy);
void removeHitbox(const gameobj_t* restrict object);

gameobj_t* addGameObject(const ascii2info_t* info, u32 len, f32 x, f32 y);
gameobj_t* addGameObjectStruct(const objectinfo_t* restrict object);
void moveGameObject(const gameobj_t* restrict obj, f32 dx, f32 dy);
void updateGameObjectPos(gameobj_t* restrict obj);
void removeGameObject(gameobj_t* restrict obj);
void resetAllGameObjects(void);
void handleCollision(void);

mat4_t multiplyMat4(mat4_t m1, mat4_t m2);
mat4_t rotationMatZ(f32 rad);
mat4_t translationMat(f32 dx, f32 dy, f32 dz);

vec3f_t normalizeVec3(vec3f_t v);
vec3f_t crossVec3(vec3f_t a, vec3f_t b);

f32 dotVec3(vec3f_t a, vec3f_t b);

#endif