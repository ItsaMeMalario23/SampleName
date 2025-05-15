#ifndef RENDERER_H
#define RENDERER_H

#include <main.h>

// render modes
#define RENDER_MODE_UNDEF           (0)
#define RENDER_MODE_2D              (1u << 0)
#define RENDER_MODE_LAYERED         (1u << 1)
#define RENDER_MODE_HITBOXES        (1u << 2)

#define RENDER_MODE_2D_HYBRID       (RENDER_MODE_2D | RENDER_MODE_LAYERED)

// max hitbox elements
#define RENDER_HITBOX_BUF_SIZE      8

#define RENDER_MAX_LAYERS           6
#define RENDER_OBJ_PER_LAYER        8

// std background color as FColor
#define RENDER_CLEAR_COLOR_R        ( 24.0f / 255.0f)
#define RENDER_CLEAR_COLOR_G        ( 32.0f / 255.0f)
#define RENDER_CLEAR_COLOR_B        ( 84.0f / 255.0f)
#define RENDER_CLEAR_COLOR_A        (  1.0f)

//
//  Typedefs
//
typedef struct context_s {
    const char*     name;
    const char*     path;
    SDL_Window*     window;
    SDL_GPUDevice*  dev;
    u32             width;
    u32             height;
} context_t;

typedef struct renderlayer_s {
    gameobj_t** objects;
    u32         numobjects;
    u32         depth;
} renderlayer_t;

extern SDL_Mutex* renderBufLock;

//
//  Render utils
//
SDL_GPUShader* renderLoadShader(context_t* restrict con, const char* restrict filename, u32 samplerCnt, u32 uniBufCnt, u32 stoBufCnt, u32 stoTexCnt);
SDL_Surface* renderLoadBmp(context_t* restrict con, const char* restrict filename);

SDL_GPUTextureFormat renderInitWindow(context_t* restrict con, SDL_WindowFlags flags);
SDL_GPUGraphicsPipeline* renderInit2DPipeline(SDL_GPUDevice* restrict dev, SDL_GPUShader* restrict vert, SDL_GPUShader* restrict frag, SDL_GPUTextureFormat fmt);
SDL_GPUGraphicsPipeline* renderInitBoxPipeline(SDL_GPUDevice* restrict dev, SDL_GPUShader* restrict vert, SDL_GPUShader* restrict frag, SDL_GPUTextureFormat fmt);
SDL_GPUTexture* renderInitAsciiTexture(context_t* restrict con, const char* restrict filename, SDL_GPUSampler** restrict sampler);

void renderCleanupWindow(context_t* restrict con);

//
//  Render
//
void renderInit(context_t* restrict con, SDL_WindowFlags flags);
void renderDraw(context_t* restrict con);
void renderCleanup(context_t* restrict con);

//
//  Worldsim
//
void addObjectToLayer(gameobj_t* object, u32 layer);
void addHitbox(gameobj_t* object);
void removeHitbox(gameobj_t* object);
void handleCollision(void);
gameobj_t* addGameObject(const ascii2info_t* info, u32 len, f32 x, f32 y);
gameobj_t* addGameObjectStruct(const objectinfo_t* restrict object);
void moveGameObject(const gameobj_t* restrict obj, f32 dx, f32 dy);
void updateGameObjectPos(gameobj_t* restrict obj);
void removeGameObject(gameobj_t* restrict obj);
void resetAllGameObjects(void);

#endif