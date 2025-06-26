#ifndef RENDER_H
#define RENDER_H

#include <main.h>
#include <render/camera.h>

// render modes
#define RENDER_MODE_UNDEF           (0u)
#define RENDER_MODE_2D              (1u << 0)
#define RENDER_MODE_3D              (1u << 1)
#define RENDER_MODE_LAYERED         (1u << 2)
#define RENDER_MODE_ODYSSEY         (1u << 3)
#define RENDER_MODE_2D_LIGHT        (1u << 4)
#define RENDER_MODE_UI_STATIC       (1u << 5)
#define RENDER_MODE_UI_DYNAMIC      (1u << 6)
#define RENDER_MODE_HITBOXES        (1u << 7)

#define RENDER_MODE_2D_HYBRID       (RENDER_MODE_2D | RENDER_MODE_LAYERED)
#define RENDER_MODE_3D_HYBRID       (RENDER_MODE_2D | RENDER_MODE_3D)

#define RMODE_REQUIRE_COPYPASS      (RENDER_MODE_2D | RENDER_MODE_LAYERED | RENDER_MODE_HITBOXES | RENDER_MODE_ODYSSEY | RENDER_MODE_UI_DYNAMIC)

// max hitbox elements
#define RENDER_HITBOX_BUF_SIZE      8

// max renderable 3D objects
#define RENDER_OBJ3D_BUF_SIZE       32

#define RENDER_MAX_LAYERS           6
#define RENDER_OBJ_PER_LAYER        8

// max ascii chars in ui
#define RENDER_DYNUI_BUF_SIZE       256

// std background color as FColor
#define RENDER_CLEAR_COLOR_R        ( 24.0f / 255.0f)
#define RENDER_CLEAR_COLOR_G        ( 32.0f / 255.0f)
#define RENDER_CLEAR_COLOR_B        ( 84.0f / 255.0f)
#define RENDER_CLEAR_COLOR_A        (  1.0f)

//
//  Typedefs
//
typedef struct renderlayer_s {
    u8*         objects;
    u32         numobjects;
    u32         depth;
} renderlayer_t;

typedef struct uiobject_s {
    asciidata_t* data;
    u32          len;
    u32          bufoffset;
    u64          flags;
} uiobject_t;

extern SDL_Mutex* renderBufLock;

//
//  Render utils
//
SDL_GPUShader* renderLoadShader(context_t* restrict con, const char* filename, u32 samplerCnt, u32 uniBufCnt, u32 stoBufCnt, u32 stoTexCnt);
SDL_Surface* renderLoadBmp(context_t* restrict con, const char* filename);

SDL_GPUTextureFormat renderInitWindow(context_t* restrict con, SDL_WindowFlags flags);
SDL_GPUGraphicsPipeline* renderInit2DPipeline(SDL_GPUDevice* dev, SDL_GPUShader* restrict vert, SDL_GPUShader* restrict frag, SDL_GPUTextureFormat fmt);
SDL_GPUGraphicsPipeline* renderInitBoxPipeline(SDL_GPUDevice* dev, SDL_GPUShader* restrict vert, SDL_GPUShader* restrict frag, SDL_GPUTextureFormat fmt);
SDL_GPUGraphicsPipeline* renderInit3DPipeline(context_t* restrict con, SDL_GPUShader* restrict vert, SDL_GPUShader* restrict frag, SDL_GPUTexture** depthtex, SDL_GPUTextureFormat fmt);
SDL_GPUGraphicsPipeline* renderInitOdysseyPipeline(SDL_GPUDevice* dev, SDL_GPUShader* restrict vert, SDL_GPUShader* restrict frag, SDL_GPUTextureFormat fmt);
SDL_GPUTexture* renderInitAsciiTexture(context_t* restrict con, const char* filename, SDL_GPUSampler** sampler);
SDL_GPUTexture* renderInitOdysseyTexture(context_t* restrict con, SDL_GPUSampler** sampler, SDL_GPUTextureFormat rfmt, u32 w, u32 h);

void renderCleanupWindow(context_t* restrict con);

//
//  Render
//
u32       renderGetMode(void);
camera_t* renderGetCamera(void);

void renderInit(SDL_WindowFlags flags, u32 mode);
void renderDraw(context_t* restrict con);
void renderMode(u32 mode);
void renderSetupOdyssey(u32 w, u32 h);
void renderToggleHitboxes(void);
void renderResetBuffers(void);
void renderCleanup(void);

//  Objects
void rSetup3DVtxBuf(obj3D_t* objects, u32 numobjs);
void rSetupStaticUIBuf(uiobject_t* objects, u32 numobjs);
void rTransferDynUIBuf(const asciidata_t* data, u32 len);
void rAdd3DObjectToRenderBuf(const obj3D_t* restrict object);
void rAddObjectToRenderBuf(u32 object);
void rAddObjectToLayer(u32 object, u32 layer);
void rAddHitbox(gameobj_t* object);
void rRemoveHitbox(gameobj_t* object);

#endif