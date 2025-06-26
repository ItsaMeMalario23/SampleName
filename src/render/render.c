#include <stdio.h>
#include <SDL3/SDL.h>

#include <input.h>
#include <worldsim.h>
#include <objects.h>
#include <render/camera.h>
#include <render/render.h>
#include <debug/rdebug.h>

// Render mode
static u32 rmode = RENDER_MODE_UNDEF;

// Render pipelines
static SDL_GPUGraphicsPipeline* pipeline2D;
static SDL_GPUGraphicsPipeline* pipeline3D;
static SDL_GPUGraphicsPipeline* pipelineOdyssey;
static SDL_GPUGraphicsPipeline* pipelineLayers;
static SDL_GPUGraphicsPipeline* pipelineHitbox;

// 3D / Odyssey
static SDL_GPUTexture* depthtexture;
static SDL_GPUTexture* odysseytexture;
static SDL_GPUSampler* odysseysampler;

static camera_t camera;
static context_t* context;

static const obj3D_t* objbuf3D[RENDER_OBJ3D_BUF_SIZE];

// Ascii texture
static SDL_GPUTexture* asciitex;
static SDL_GPUSampler* sampler;

// Buffers
static SDL_GPUTransferBuffer* asciitransferbuf;
static SDL_GPUBuffer* asciigpubuf;
static SDL_GPUTransferBuffer* boxtransferbuf;
static SDL_GPUBuffer* boxvtxbuf;
static SDL_GPUTransferBuffer* transferbuf3D;
static SDL_GPUBuffer* vtxbuf3D;
static SDL_GPUTransferBuffer* uitransferbuf_stat;
static SDL_GPUBuffer* uibuf_stat;
static SDL_GPUTransferBuffer* uitransferbuf_dyn;
static SDL_GPUBuffer* uibuf_dyn;
static SDL_GPUBuffer* odysseybuf;

static SDL_GPUTextureFormat rfmt;

SDL_Mutex* renderBufLock = NULL;

// Objects
static gameobj_t* objectbuf;

static u32 num2Dchars;
static u32 numlayerchars;
static u32 numobjects;
static u32 num3Dobjects;
static u32 numstaticuichars;
static u32 numdynuichars;

static u8 renderbuf[ASCII_OBJ_BUF_SIZE];

// Layers
static u8 layerobjects[RENDER_MAX_LAYERS * RENDER_OBJ_PER_LAYER];
static renderlayer_t layers[RENDER_MAX_LAYERS];

static u32 indices[RENDER_MAX_LAYERS];
static f32 scales[RENDER_MAX_LAYERS] = {0.4f, 0.6f, 0.8f, 1.0f, 1.2f, 1.2f};

// Hitboxes
static gameobj_t* hitboxes[RENDER_HITBOX_BUF_SIZE];
static u32 numboxes;

static instate_t* input;    // TODO remove this

void renderInit(SDL_WindowFlags flags, u32 mode)
{
    context = getContext();
    input = getInputState();
    objectbuf = getObjectBuf();

    // init window
    rfmt = renderInitWindow(context, flags);

    // init render pipelines
    SDL_GPUShader* vert = renderLoadShader(context, "ascii2D.vert", 0, 0, 1, 0);
    SDL_GPUShader* frag = renderLoadShader(context, "ascii2D.frag", 1, 0, 0, 0);

    pipeline2D = renderInit2DPipeline(context->dev, vert, frag, rfmt);

    SDL_ReleaseGPUShader(context->dev, vert);
    SDL_ReleaseGPUShader(context->dev, frag);

    asciitex = renderInitAsciiTexture(context, "pressstart.bmp", &sampler);

    vert = renderLoadShader(context, "box.vert", 0, 0, 0, 0);
    frag = renderLoadShader(context, "box.frag", 0, 0, 0, 0);

    pipelineHitbox = renderInitBoxPipeline(context->dev, vert, frag, rfmt);

    SDL_ReleaseGPUShader(context->dev, vert);
    SDL_ReleaseGPUShader(context->dev, frag);

    vert = renderLoadShader(context, "asciiLayers.vert", 0, 1, 1, 0);
    frag = renderLoadShader(context, "ascii2D.frag", 1, 0, 0, 0);

    pipelineLayers = renderInit2DPipeline(context->dev, vert, frag, rfmt);

    SDL_ReleaseGPUShader(context->dev, vert);
    SDL_ReleaseGPUShader(context->dev, frag);

    if (!pipeline2D || !pipelineHitbox || !pipelineLayers || !asciitex)
        return;

    // init buffers
    asciitransferbuf = SDL_CreateGPUTransferBuffer(
        context->dev,
        &(SDL_GPUTransferBufferCreateInfo) {
            .usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD,
            .size = ASCII_CHAR_BUF_SIZE * sizeof(asciidata_t)
        }
    );

    rAssert(asciitransferbuf);

    asciigpubuf = SDL_CreateGPUBuffer(
        context->dev,
        &(SDL_GPUBufferCreateInfo) {
            .usage = SDL_GPU_BUFFERUSAGE_GRAPHICS_STORAGE_READ,
            .size = ASCII_CHAR_BUF_SIZE * sizeof(asciidata_t)
        }
    );

    rAssert(asciigpubuf);

    boxtransferbuf = SDL_CreateGPUTransferBuffer(
        context->dev,
        &(SDL_GPUTransferBufferCreateInfo) {
            .usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD,
            .size = sizeof(vec2f_t) * 6 * RENDER_HITBOX_BUF_SIZE
        }
    );

    rAssert(boxtransferbuf);

    boxvtxbuf = SDL_CreateGPUBuffer(
        context->dev,
        &(SDL_GPUBufferCreateInfo) {
            .usage = SDL_GPU_BUFFERUSAGE_VERTEX,
            .size = sizeof(vec2f_t) * 6 * RENDER_HITBOX_BUF_SIZE
        }
    );

    rAssert(boxvtxbuf);

    uitransferbuf_dyn = SDL_CreateGPUTransferBuffer(
        context->dev,
        &(SDL_GPUTransferBufferCreateInfo) {
            .usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD,
            .size = RENDER_DYNUI_BUF_SIZE * sizeof(asciidata_t)
        }
    );

    rAssert(uitransferbuf_dyn);

    uibuf_dyn = SDL_CreateGPUBuffer(
        context->dev,
        &(SDL_GPUBufferCreateInfo) {
            .usage = SDL_GPU_BUFFERUSAGE_GRAPHICS_STORAGE_READ,
            .size = RENDER_DYNUI_BUF_SIZE * sizeof(asciidata_t)
        }
    );

    rAssert(uibuf_dyn);

    for (u32 i = 0; i < RENDER_MAX_LAYERS; i++) {
        layers[i].numobjects = 0;
        layers[i].objects = layerobjects + (i * RENDER_OBJ_PER_LAYER);
    }

    vert = renderLoadShader(context, "test3D.vert", 0, 2, 0, 0);
    frag = renderLoadShader(context, "test3D.frag", 0, 0, 0, 0);

    pipeline3D = renderInit3DPipeline(context, vert, frag, &depthtexture, rfmt);

    if (!pipeline3D || !depthtexture)
        return;

    SDL_ReleaseGPUShader(context->dev, vert);
    SDL_ReleaseGPUShader(context->dev, frag);

    vert = renderLoadShader(context, "odyssey.vert", 0, 2, 0, 0);
    frag = renderLoadShader(context, "odyssey.frag", 1, 0, 0, 0);

    pipelineOdyssey = renderInitOdysseyPipeline(context->dev, vert, frag, rfmt);

    if (!pipelineOdyssey)
        return;

    SDL_ReleaseGPUShader(context->dev, vert);
    SDL_ReleaseGPUShader(context->dev, frag);

    SDL_GPUTransferBuffer* tmp = SDL_CreateGPUTransferBuffer(
        context->dev,
        &(SDL_GPUTransferBufferCreateInfo) {
            .usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD,
            .size = sizeof(f32) * 30
        }
    );

    rAssert(tmp);

    odysseybuf = SDL_CreateGPUBuffer(
        context->dev,
        &(SDL_GPUBufferCreateInfo) {
            .usage = SDL_GPU_BUFFERUSAGE_VERTEX,
            .size = sizeof(f32) * 30
        }
    );

    rAssert(odysseybuf);

    void* transfmem = SDL_MapGPUTransferBuffer(context->dev, tmp, false);

    memcpy(transfmem, wall_vtx_uv, sizeof(f32) * 30);

    SDL_UnmapGPUTransferBuffer(context->dev, tmp);

    SDL_GPUCommandBuffer* cmdbuf = SDL_AcquireGPUCommandBuffer(context->dev);
    SDL_GPUCopyPass* copypass = SDL_BeginGPUCopyPass(cmdbuf);

    SDL_UploadToGPUBuffer(
        copypass,
        &(SDL_GPUTransferBufferLocation) {
            .transfer_buffer = tmp,
            .offset = 0
        },
        &(SDL_GPUBufferRegion) {
            .buffer = odysseybuf,
            .offset = 0,
            .size = sizeof(f32) * 30
        },
        false
    );

    SDL_EndGPUCopyPass(copypass);
    SDL_SubmitGPUCommandBuffer(cmdbuf);

    SDL_ReleaseGPUTransferBuffer(context->dev, tmp);

    rmode = mode;
}

static inline void transferAsciiBuf(SDL_GPUCopyPass* const copypass, SDL_GPUDevice* const dev)
{
    asciidata_t* transfmem = SDL_MapGPUTransferBuffer(context->dev, asciitransferbuf, true);

    rAssert(asciitransferbuf);
    rAssert(transfmem);
    rAssert(numobjects);

    num2Dchars = 0;

    SDL_LockMutex(renderBufLock);

    for (u32 i = 0; i < numobjects; i++) {
        if (!objectbuf[renderbuf[i]].visible)
            continue;

        memcpy(transfmem + num2Dchars, objectbuf[renderbuf[i]].data, sizeof(asciidata_t) * objectbuf[renderbuf[i]].len);

        num2Dchars += objectbuf[renderbuf[i]].len;
    }

    SDL_UnlockMutex(renderBufLock);

    rAssert(num2Dchars < ASCII_CHAR_BUF_SIZE);

    SDL_UnmapGPUTransferBuffer(context->dev, asciitransferbuf);

    SDL_UploadToGPUBuffer(
        copypass,
        &(SDL_GPUTransferBufferLocation) {
            .transfer_buffer = asciitransferbuf,
            .offset = 0
        },
        &(SDL_GPUBufferRegion) {
            .buffer = asciigpubuf,
            .offset = 0,
            .size = num2Dchars * sizeof(asciidata_t)
        },
        true
    );
}

static inline void transferDynUIBuf(SDL_GPUCopyPass* const copypass, SDL_GPUDevice* const dev)
{
    SDL_UploadToGPUBuffer(
        copypass,
        &(SDL_GPUTransferBufferLocation) {
            .transfer_buffer = uitransferbuf_dyn,
            .offset = 0
        },
        &(SDL_GPUBufferRegion) {
            .buffer = uibuf_dyn,
            .offset = 0,
            .size = numdynuichars * sizeof(asciidata_t)
        },
        true
    );
}

static inline void transferLayerBuf(SDL_GPUCopyPass* const copypass, SDL_GPUDevice* const dev)
{
    asciidata_t* transfmem = SDL_MapGPUTransferBuffer(dev, asciitransferbuf, true);

    memset(indices, 0, sizeof(indices));
    numlayerchars = 0;

    SDL_LockMutex(renderBufLock);

    for (u32 i = 0; i < RENDER_MAX_LAYERS; i++)
    {
        indices[i] = numlayerchars;

        for (u32 k = 0; k < layers[i].numobjects; k++)
        {
            if (!objectbuf[layers[i].objects[k]].visible)
                continue;

            memcpy(transfmem + numlayerchars, objectbuf[layers[i].objects[k]].data, sizeof(asciidata_t) * objectbuf[layers[i].objects[k]].len);
            
            numlayerchars += objectbuf[layers[i].objects[k]].len;
        }
    }

    SDL_UnlockMutex(renderBufLock);

    rAssert(numlayerchars < ASCII_CHAR_BUF_SIZE);

    SDL_UnmapGPUTransferBuffer(dev, asciitransferbuf);

    SDL_UploadToGPUBuffer(
        copypass,
        &(SDL_GPUTransferBufferLocation) {
            .transfer_buffer = asciitransferbuf,
            .offset = 0
        },
        &(SDL_GPUBufferRegion) {
            .buffer = asciigpubuf,
            .offset = 0,
            .size = numlayerchars * sizeof(asciidata_t)
        },
        true
    );
}

static inline void transferBoxVtxBuf(SDL_GPUCopyPass* const copypass, SDL_GPUDevice* const dev)
{
    rAssert(numboxes <= 8);

    vec2f_t* transfmem = SDL_MapGPUTransferBuffer(dev, boxtransferbuf, true);

    u32 k = 0;
    for (gameobj_t** i = hitboxes; i < hitboxes + RENDER_HITBOX_BUF_SIZE; i++) {
        if (!(*i))
            continue;

        transfmem[k]     = (vec2f_t) { (*i)->x + (*i)->hitbox_dx,                (*i)->y + (*i)->hitbox_dy + (*i)->yscale };
        transfmem[k + 1] = (vec2f_t) { (*i)->x + (*i)->hitbox_dx + (*i)->xscale, (*i)->y + (*i)->hitbox_dy + (*i)->yscale };
        transfmem[k + 2] = (vec2f_t) { (*i)->x + (*i)->hitbox_dx,                (*i)->y + (*i)->hitbox_dy                };
        transfmem[k + 3] = (vec2f_t) { (*i)->x + (*i)->hitbox_dx + (*i)->xscale, (*i)->y + (*i)->hitbox_dy                };
        transfmem[k + 4] = (vec2f_t) { (*i)->x + (*i)->hitbox_dx,                (*i)->y + (*i)->hitbox_dy                };
        transfmem[k + 5] = (vec2f_t) { (*i)->x + (*i)->hitbox_dx + (*i)->xscale, (*i)->y + (*i)->hitbox_dy + (*i)->yscale };

        k += 6;
    }

    SDL_UnmapGPUTransferBuffer(dev, boxtransferbuf);

    SDL_UploadToGPUBuffer(
        copypass,
        &(SDL_GPUTransferBufferLocation) {
            .transfer_buffer = boxtransferbuf,
            .offset = 0
        },
        &(SDL_GPUBufferRegion) {
            .buffer = boxvtxbuf,
            .offset = 0,
            .size = sizeof(vec2f_t) * 6 * numboxes
        },
        true
    );
}

static inline void draw2D(SDL_GPURenderPass* const renderpass)
{
    SDL_BindGPUGraphicsPipeline(renderpass, pipeline2D);
    SDL_BindGPUVertexStorageBuffers(renderpass, 0, &asciigpubuf, 1);
    SDL_BindGPUFragmentSamplers(renderpass, 0, &(SDL_GPUTextureSamplerBinding) { .texture = asciitex, .sampler = sampler }, 1);

    SDL_DrawGPUPrimitives(renderpass, num2Dchars * 6, 1, 0, 0);
}

static inline void drawStaticUI(SDL_GPURenderPass* const renderpass)
{
    SDL_BindGPUGraphicsPipeline(renderpass, pipeline2D);
    SDL_BindGPUVertexStorageBuffers(renderpass, 0, &uibuf_stat, 1);
    SDL_BindGPUFragmentSamplers(renderpass, 0, &(SDL_GPUTextureSamplerBinding) { .texture = asciitex, .sampler = sampler }, 1);

    SDL_DrawGPUPrimitives(renderpass, numstaticuichars * 6, 1, 0, 0);
}

static inline void drawDynUI(SDL_GPURenderPass* const renderpass)
{
    rReleaseAssert(numdynuichars <= 256);

    SDL_BindGPUGraphicsPipeline(renderpass, pipeline2D);
    SDL_BindGPUVertexStorageBuffers(renderpass, 0, &uibuf_dyn, 1);
    SDL_BindGPUFragmentSamplers(renderpass, 0, &(SDL_GPUTextureSamplerBinding) { .texture = asciitex, .sampler = sampler }, 1);

    SDL_DrawGPUPrimitives(renderpass, numdynuichars * 6, 1, 0, 0);
}

static inline void drawLayers(SDL_GPURenderPass* const renderpass, SDL_GPUCommandBuffer* const cmdbuf)
{
    SDL_BindGPUGraphicsPipeline(renderpass, pipelineLayers);
    SDL_BindGPUVertexStorageBuffers(renderpass, 0, &asciigpubuf, 1);
    SDL_BindGPUFragmentSamplers(renderpass, 0, &(SDL_GPUTextureSamplerBinding) { .texture = asciitex, .sampler = sampler }, 1);

    f32 data[4];

    data[0] = input->screen_dx * scales[0];
    data[1] = input->screen_dy * scales[0];
    data[2] = scales[0];
    SDL_PushGPUVertexUniformData(cmdbuf, 0, data, 32);
    SDL_DrawGPUPrimitives(renderpass, indices[1] * 6, 1, 0, 0);

    data[0] = input->screen_dx * scales[1];
    data[1] = input->screen_dy * scales[1];
    data[2] = scales[1];
    SDL_PushGPUVertexUniformData(cmdbuf, 0, data, 32);
    SDL_DrawGPUPrimitives(renderpass, (indices[2] - indices[1]) * 6, 1, indices[1] * 6, 0);

    data[0] = input->screen_dx * scales[2];
    data[1] = input->screen_dy * scales[2];
    data[2] = scales[2];
    SDL_PushGPUVertexUniformData(cmdbuf, 0, data, 32);
    SDL_DrawGPUPrimitives(renderpass, (indices[3] - indices[2]) * 6, 1, indices[2] * 6, 0);

    data[0] = input->screen_dx * scales[3];
    data[1] = input->screen_dy * scales[3];
    data[2] = scales[3];
    SDL_PushGPUVertexUniformData(cmdbuf, 0, data, 32);
    SDL_DrawGPUPrimitives(renderpass, (indices[4] - indices[3]) * 6, 1, indices[3] * 6, 0);

    data[0] = input->screen_dx * scales[4];
    data[1] = input->screen_dy * scales[4];
    data[2] = scales[4];
    SDL_PushGPUVertexUniformData(cmdbuf, 0, data, 32);
    SDL_DrawGPUPrimitives(renderpass, (indices[5] - indices[4]) * 6, 1, indices[4] * 6, 0);

    data[0] = input->screen_dx * scales[5];
    data[1] = input->screen_dy * scales[5];
    data[2] = scales[5];
    SDL_PushGPUVertexUniformData(cmdbuf, 0, data, 32);
    SDL_DrawGPUPrimitives(renderpass, (numlayerchars - indices[5]) * 6, 1, indices[5] * 6, 0);
}

static inline void draw3D(SDL_GPURenderPass* const renderpass, SDL_GPUCommandBuffer* const cmdbuf)
{
    SDL_BindGPUGraphicsPipeline(renderpass, pipeline3D);
    SDL_BindGPUVertexBuffers(renderpass, 0, &(SDL_GPUBufferBinding) { .buffer = vtxbuf3D, .offset = 0 }, 1);
    SDL_PushGPUVertexUniformData(cmdbuf, 0, &camera.rendermat, sizeof(mat4_t));

    const obj3D_t** bound = objbuf3D + num3Dobjects;
    for (const obj3D_t** i = objbuf3D; i < bound; i++) {
        if (!((*i)->flags & OBJECT_VISIBLE))
            continue;

        if ((*i)->flags & OBJECT_NEED_REBUILD)
            objectUpdate((obj3D_t*) (*i));

        SDL_PushGPUVertexUniformData(cmdbuf, 1, &(*i)->transform, sizeof(mat4_t));
        SDL_DrawGPUPrimitives(renderpass, (*i)->numvtx, 1, (*i)->vtxbufoffset, 0);
    }
}

static inline void drawOdyssey2D(SDL_GPURenderPass* renderpass, SDL_GPUCommandBuffer* cmdbuf)
{
    SDL_BindGPUGraphicsPipeline(renderpass, pipelineOdyssey);

    //SDL_BindGPUVertexBuffers(renderpass, 0, &(SDL_GPUBufferBinding) { .buffer = vtxbuf3D, .offset = 0 }, 1);
    SDL_BindGPUVertexBuffers(renderpass, 0, &(SDL_GPUBufferBinding) { .buffer = odysseybuf, .offset = 0 }, 1);
    SDL_BindGPUFragmentSamplers(renderpass, 0, &(SDL_GPUTextureSamplerBinding) { .texture = odysseytexture, .sampler = odysseysampler }, 1);

    SDL_PushGPUVertexUniformData(cmdbuf, 0, &camera.rendermat, sizeof(mat4_t));
    SDL_PushGPUVertexUniformData(cmdbuf, 1, &objects3D[1].transform, sizeof(mat4_t));

    bool fucked = 0;

    if (!objects3D[1].vtxbufoffset)     // TODO why tf is this happening?
        fucked = 1;

    SDL_DrawGPUPrimitives(renderpass, 6, 1, 0, 0);
}

static inline void drawBoxes(SDL_GPURenderPass* const renderpass)
{
    SDL_BindGPUGraphicsPipeline(renderpass, pipelineHitbox);
    SDL_BindGPUVertexBuffers(renderpass, 0, &(SDL_GPUBufferBinding) { .buffer = boxvtxbuf, .offset = 0 }, 1);

    SDL_DrawGPUPrimitives(renderpass, numboxes * 6, 1, 0, 0);
}

static inline void drawOdyssey(SDL_GPUCommandBuffer* cmdbuf, SDL_GPUDevice* dev)
{
    // 2D mario scene render pass
    SDL_GPURenderPass* renderpass = SDL_BeginGPURenderPass(
        cmdbuf,
        &(SDL_GPUColorTargetInfo) {
            .texture = odysseytexture,
            .cycle = true,
            .load_op = SDL_GPU_LOADOP_CLEAR,
            .store_op = SDL_GPU_STOREOP_STORE,
            .clear_color = (SDL_FColor) {
                0.10196f, 0.10196f, 0.10196f, 1.0f
            }
        },
        1,
        NULL
    );

    draw2D(renderpass);

    SDL_EndGPURenderPass(renderpass);

    SDL_SubmitGPUCommandBuffer(cmdbuf);

    /*
    SDL_GPUFence* f = SDL_SubmitGPUCommandBufferAndAcquireFence(cmdbuf);

    if (!f)
        SDL_Log("[ERROR] Failed to acquire fence: %s", SDL_GetError());

    if (!SDL_WaitForGPUFences(context->dev, true, &f, 1))
        SDL_Log("[ERROR] Failed to wait for fence: %s", SDL_GetError());

    SDL_ReleaseGPUFence(context->dev, f);*/

    cmdbuf = SDL_AcquireGPUCommandBuffer(context->dev);

    SDL_GPUTexture* swapchain;

    if (!SDL_WaitAndAcquireGPUSwapchainTexture(cmdbuf, context->window, &swapchain, NULL, NULL)) {
        SDL_Log("[ERROR] Failed to acquire swapchain texture O: %s", SDL_GetError());
        return;
    }

    if (!swapchain) {
        SDL_SubmitGPUCommandBuffer(cmdbuf);
        return;
    }

    // 3D scene render pass
    renderpass = SDL_BeginGPURenderPass(
        cmdbuf,
        &(SDL_GPUColorTargetInfo) {
            .texture = swapchain,
            .cycle = true,
            .load_op = SDL_GPU_LOADOP_CLEAR,
            .store_op = SDL_GPU_STOREOP_STORE,
            .clear_color = (SDL_FColor) {
                RENDER_CLEAR_COLOR_R,
                RENDER_CLEAR_COLOR_G,
                RENDER_CLEAR_COLOR_B,
                RENDER_CLEAR_COLOR_A
            }
        },
        1,
        &(SDL_GPUDepthStencilTargetInfo) {
            .texture = depthtexture,
            .cycle = true,
            .load_op = SDL_GPU_LOADOP_CLEAR,
            .store_op = SDL_GPU_STOREOP_DONT_CARE,
            .clear_depth = 10.0f
        }
    );

    draw3D(renderpass, cmdbuf);

    SDL_EndGPURenderPass(renderpass);

    // need to start new render pass, otherwise crash
    renderpass = SDL_BeginGPURenderPass(
        cmdbuf,
        &(SDL_GPUColorTargetInfo) {
            .texture = swapchain,
            .cycle = true,
            .load_op = SDL_GPU_LOADOP_LOAD,
            .store_op = SDL_GPU_STOREOP_STORE,
        },
        1,
        NULL
    );

    drawOdyssey2D(renderpass, cmdbuf);

    SDL_EndGPURenderPass(renderpass);
    SDL_SubmitGPUCommandBuffer(cmdbuf);
}

void renderDraw(context_t* restrict con)
{
    SDL_GPUCommandBuffer* cmdbuf = SDL_AcquireGPUCommandBuffer(context->dev);

    rAssert(cmdbuf);

    u32 m = rmode;

    if (m & RMODE_REQUIRE_COPYPASS) {
        if (!cmdbuf) {
            SDL_Log("[ERROR] Failed to acquire copy cmd buf: %s", SDL_GetError());
            return;
        }

        // copy pass
        SDL_GPUCopyPass* copypass = SDL_BeginGPUCopyPass(cmdbuf);

        rAssert(copypass);

        if (m & RENDER_MODE_2D || m & RENDER_MODE_ODYSSEY)
            transferAsciiBuf(copypass, context->dev);
        
        if (m & RENDER_MODE_LAYERED)
            transferLayerBuf(copypass, context->dev);

        if (m & RENDER_MODE_HITBOXES && numboxes)
            transferBoxVtxBuf(copypass, context->dev);

        if (m & RENDER_MODE_UI_DYNAMIC && numdynuichars & (1u << 31)) {
            numdynuichars << 1 >> 1;    // clear transfer flag

            rAssert(numdynuichars < 256);

            transferDynUIBuf(copypass, context->dev);
        }

        SDL_EndGPUCopyPass(copypass);

        // submit buffers as early as possible
        SDL_SubmitGPUCommandBuffer(cmdbuf);

        cmdbuf = SDL_AcquireGPUCommandBuffer(context->dev);
    }

    if (!cmdbuf) {
        SDL_Log("[ERROR] Failed to acquire render cmd buf: %s", SDL_GetError());
        return;
    }

    if (m & RENDER_MODE_ODYSSEY) {
        drawOdyssey(cmdbuf, context->dev);
        return;
    }

    SDL_GPUTexture* swapchain;

    if (!SDL_WaitAndAcquireGPUSwapchainTexture(cmdbuf, context->window, &swapchain, NULL, NULL)) {
        SDL_Log("[ERROR] Failed to acquire swapchain texture: %s", SDL_GetError());
        return;
    }

    if (!swapchain) {
        SDL_SubmitGPUCommandBuffer(cmdbuf);
        return;
    }

    // render pass
    SDL_GPURenderPass* renderpass = SDL_BeginGPURenderPass(
        cmdbuf,
        &(SDL_GPUColorTargetInfo) {
            .texture = swapchain,
            .cycle = true,
            .load_op = SDL_GPU_LOADOP_CLEAR,
            .store_op = SDL_GPU_STOREOP_STORE,
            .clear_color = (SDL_FColor) { RENDER_CLEAR_COLOR_R, RENDER_CLEAR_COLOR_G, RENDER_CLEAR_COLOR_B, RENDER_CLEAR_COLOR_A }
        },
        1,
        m & RENDER_MODE_3D ?
        &(SDL_GPUDepthStencilTargetInfo) {
            .texture = depthtexture,
            .cycle = true,
            .load_op = SDL_GPU_LOADOP_CLEAR,
            .store_op = SDL_GPU_STOREOP_DONT_CARE,
            .clear_depth = 10.0f
        }
        : NULL
    );

    if (m & RENDER_MODE_2D)
        draw2D(renderpass);
    
    if (m & RENDER_MODE_LAYERED)
        drawLayers(renderpass, cmdbuf);

    if (m & RENDER_MODE_HITBOXES && numboxes)
        drawBoxes(renderpass);

    if (m & RENDER_MODE_3D && num3Dobjects)
        draw3D(renderpass, cmdbuf);

    if (m & RENDER_MODE_UI_STATIC && numstaticuichars)
        drawStaticUI(renderpass);

    if (m & RENDER_MODE_UI_DYNAMIC && numdynuichars)
        drawDynUI(renderpass);

    SDL_EndGPURenderPass(renderpass);
    SDL_SubmitGPUCommandBuffer(cmdbuf);
}

u32 renderGetMode(void)
{
    return rmode;
}

camera_t* renderGetCamera(void)
{
    return &camera;
}

// implicitly resets render buffers
void renderMode(u32 mode)
{
    if (mode == RENDER_MODE_ODYSSEY)
        rAssert(odysseytexture);

    rmode = mode;
    renderResetBuffers();
}

void renderSetupOdyssey(u32 w, u32 h)
{
    rAssert(w);
    rAssert(h);

    if (odysseysampler)
        SDL_ReleaseGPUSampler(context->dev, odysseysampler);

    if (odysseytexture)
        SDL_ReleaseGPUTexture(context->dev, odysseytexture);

    odysseysampler = NULL;
    odysseytexture = NULL;

    odysseytexture = renderInitOdysseyTexture(context, &odysseysampler, rfmt, w, h);
}

void renderToggleHitboxes(void)
{
    rmode ^= RENDER_MODE_HITBOXES;
}

void renderResetBuffers(void)
{
    num2Dchars = 0;
    numlayerchars = 0;
    numobjects = 0;
    num3Dobjects = 0;
    numboxes = 0;

    memset(hitboxes, 0, sizeof(hitboxes));

    for (renderlayer_t* i = layers; i < layers + RENDER_MAX_LAYERS; i++)
        i->numobjects = 0;
}

void renderCleanup(void)
{
    if (!context) {
        SDL_Log("[ERROR] Render cleanup failed: not initialized");
        return;
    }

    SDL_ReleaseGPUGraphicsPipeline(context->dev, pipeline2D);
    SDL_ReleaseGPUGraphicsPipeline(context->dev, pipeline3D);
    SDL_ReleaseGPUGraphicsPipeline(context->dev, pipelineOdyssey);
    SDL_ReleaseGPUGraphicsPipeline(context->dev, pipelineLayers);
    SDL_ReleaseGPUGraphicsPipeline(context->dev, pipelineHitbox);
    SDL_ReleaseGPUTransferBuffer(context->dev, asciitransferbuf);
    SDL_ReleaseGPUTransferBuffer(context->dev, boxtransferbuf);
    SDL_ReleaseGPUBuffer(context->dev, asciigpubuf);
    SDL_ReleaseGPUBuffer(context->dev, boxvtxbuf);
    SDL_ReleaseGPUTexture(context->dev, asciitex);
    SDL_ReleaseGPUSampler(context->dev, sampler);

    if (odysseysampler) SDL_ReleaseGPUSampler(context->dev, odysseysampler);

    if (odysseytexture) SDL_ReleaseGPUTexture(context->dev, odysseytexture);

    if (transferbuf3D) SDL_ReleaseGPUTransferBuffer(context->dev, transferbuf3D);

    if (vtxbuf3D) SDL_ReleaseGPUBuffer(context->dev, vtxbuf3D);

    if (renderBufLock) SDL_DestroyMutex(renderBufLock);

    SDL_ReleaseGPUTransferBuffer(context->dev, uitransferbuf_stat);
    SDL_ReleaseGPUTransferBuffer(context->dev, uitransferbuf_dyn);
    SDL_ReleaseGPUBuffer(context->dev, uibuf_stat);
    SDL_ReleaseGPUBuffer(context->dev, uibuf_dyn);

    renderCleanupWindow(context);

    context = NULL;
}

//
//  Objects
//
void rSetup3DVtxBuf(obj3D_t* objects, u32 numobjs)
{
    if (!objects) {
        SDL_Log("[ERROR] setup3DVtxBuf null ref");
        return;
    }

    if (!numobjs || numobjs > RENDER_OBJ3D_BUF_SIZE) {
        SDL_Log("[ERROR] Unable to create %ld 3D elements, max %ld", numobjs, RENDER_OBJ3D_BUF_SIZE);
        return;
    }

    u32 len = 0;
    const obj3D_t* bound = objects + numobjs;

    for (obj3D_t* i = objects; i->vtxbuf && i < bound; i->vtxbufoffset = len, len += i->numvtx, i++);

    rAssert(len < 16384);

    if (transferbuf3D)
        SDL_ReleaseGPUTransferBuffer(context->dev, transferbuf3D);

    if (vtxbuf3D)
        SDL_ReleaseGPUBuffer(context->dev, vtxbuf3D);

    transferbuf3D = SDL_CreateGPUTransferBuffer(
        context->dev,
        &(SDL_GPUTransferBufferCreateInfo) {
            .usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD,
            .size = sizeof(vec3f_t) * len
        }
    );

    vtxbuf3D = SDL_CreateGPUBuffer(
        context->dev,
        &(SDL_GPUBufferCreateInfo) {
            .usage = SDL_GPU_BUFFERUSAGE_VERTEX,
            .size = sizeof(vec3f_t) * len
        }
    );

    rAssert(transferbuf3D);
    rAssert(vtxbuf3D);

    vec3f_t* transfmem = SDL_MapGPUTransferBuffer(context->dev, transferbuf3D, false);

    for (const obj3D_t* i = objects; i < bound; transfmem += i->numvtx, i++) {
        memcpy(transfmem, i->vtxbuf, sizeof(vec3f_t) * i->numvtx);

        if (i->flags & OBJECT_RENDER)
            rAdd3DObjectToRenderBuf(i);
    }

    SDL_UnmapGPUTransferBuffer(context->dev, transferbuf3D);

    SDL_GPUCommandBuffer* cmdbuf = SDL_AcquireGPUCommandBuffer(context->dev);
    SDL_GPUCopyPass* copypass = SDL_BeginGPUCopyPass(cmdbuf);

    SDL_UploadToGPUBuffer(
        copypass,
        &(SDL_GPUTransferBufferLocation) {
            .transfer_buffer = transferbuf3D,
            .offset = 0
        },
        &(SDL_GPUBufferRegion) {
            .buffer = vtxbuf3D,
            .offset = 0,
            .size = sizeof(vec3f_t) * len
        },
        false
    );

    SDL_EndGPUCopyPass(copypass);
    SDL_SubmitGPUCommandBuffer(cmdbuf);
}

void rSetupStaticUIBuf(uiobject_t* objects, u32 numobjs)
{
    rAssert(objects);
    rAssert(numobjs);

    const uiobject_t* bound = objects + numobjs;

    for (uiobject_t* i = objects; i->data && i < bound; i->bufoffset = numstaticuichars, numstaticuichars += i->len, i++);

    rAssert(numstaticuichars < 16384);

    if (uitransferbuf_stat)
        SDL_ReleaseGPUTransferBuffer(context->dev, uitransferbuf_stat);

    if (uibuf_stat)
        SDL_ReleaseGPUBuffer(context->dev, uibuf_stat);

    uitransferbuf_stat = SDL_CreateGPUTransferBuffer(
        context->dev,
        &(SDL_GPUTransferBufferCreateInfo) {
            .usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD,
            .size = numstaticuichars * sizeof(asciidata_t)
        }
    );

    uibuf_stat = SDL_CreateGPUBuffer(
        context->dev,
        &(SDL_GPUBufferCreateInfo) {
            .usage = SDL_GPU_BUFFERUSAGE_GRAPHICS_STORAGE_READ,
            .size = numstaticuichars * sizeof(asciidata_t)
        }
    );

    rAssert(uitransferbuf_stat);
    rAssert(uibuf_stat);

    asciidata_t* mem = SDL_MapGPUTransferBuffer(context->dev, uitransferbuf_stat, true);

    for (const uiobject_t* i = objects; i < bound; mem += i->len, i++)
        memcpy(mem, i->data, i->len * sizeof(asciidata_t));

    SDL_UnmapGPUTransferBuffer(context->dev, uitransferbuf_stat);

    SDL_GPUCommandBuffer* cmdbuf = SDL_AcquireGPUCommandBuffer(context->dev);
    SDL_GPUCopyPass* copypass = SDL_BeginGPUCopyPass(cmdbuf);

    SDL_UploadToGPUBuffer(
        copypass,
        &(SDL_GPUTransferBufferLocation) {
            .transfer_buffer = uitransferbuf_stat,
            .offset = 0
        },
        &(SDL_GPUBufferRegion) {
            .buffer = uibuf_stat,
            .offset = 0,
            .size = numstaticuichars * sizeof(asciidata_t)
        },
        true
    );

    SDL_EndGPUCopyPass(copypass);

    SDL_SubmitGPUCommandBuffer(cmdbuf);
}

void rTransferDynUIBuf(const asciidata_t* data, u32 len)
{
    if (!data || !len || len > RENDER_DYNUI_BUF_SIZE)
        return;

    void* mem = SDL_MapGPUTransferBuffer(context->dev, uitransferbuf_dyn, true);

    memcpy(mem, data, len * sizeof(asciidata_t));

    SDL_UnmapGPUTransferBuffer(context->dev, uitransferbuf_dyn);

    numdynuichars = len | (1u << 31);   // set transfer flag
}

void rAdd3DObjectToRenderBuf(const obj3D_t* restrict object)
{
    rAssert(object);

    if (num3Dobjects >= RENDER_OBJ3D_BUF_SIZE) {
        SDL_Log("[ERROR] Failed to add 3D object to render buf, buffer size %ld full", RENDER_OBJ3D_BUF_SIZE);
        return;
    }

    objbuf3D[num3Dobjects++] = object;
}

void rAddObjectToRenderBuf(u32 object)
{
    if (object >= ASCII_OBJ_BUF_SIZE || numobjects >= ASCII_OBJ_BUF_SIZE) {
        SDL_Log("[ERROR] Failed to add object to render buf");
        return;
    }

    renderbuf[numobjects++] = object;
}

void rAddObjectToLayer(u32 object, u32 layer)
{
    if (object >= ASCII_OBJ_BUF_SIZE || layer >= RENDER_MAX_LAYERS || layers[layer].numobjects >= RENDER_OBJ_PER_LAYER) {
        SDL_Log("[ERROR] Failed to add object to layer");
        return;
    }

    layers[layer].objects[layers[layer].numobjects++] = object;
}

void rAddHitbox(gameobj_t* object)
{
    for (u32 i = 0; i < RENDER_HITBOX_BUF_SIZE; i++) {
        if (!hitboxes[i]) {
            hitboxes[i] = object;
            numboxes++;
            return;
        }
    }

    SDL_Log("[ERROR] Failed to add hitbox, buffer full");
}

void rRemoveHitbox(gameobj_t* object)
{
    if (!numboxes) {
        SDL_Log("[ERROR] Failed to remove hitbox, no hitboxes in buf");
        return;
    }

    for (u32 i = 0; i < RENDER_HITBOX_BUF_SIZE; i++) {
        if (hitboxes[i] == object) {
            hitboxes[i] = NULL;
            numboxes--;
            return;
        }
    }

    SDL_Log("[ERROR] Failed to remove hitbox, object has no hitbox");
}

//
//
//
/*
void handleCollision(void)
{
    rAssert(bird);
    rAssert(fruit[0]);

    for (gameobj_t* i = fruit[0]; i < fruit[0] + 4; i++) {
        if (bird->x + bird->xscale < i->x || bird->x > i->x + i->xscale)
            continue;

        if (bird->y < i->y || bird->y - bird->yscale > i->y + i->yscale)
            continue;

        for (asciidata_t* k = i->data; k < i->data + i->len; k++) {
            k->a = 0.0f;
            k->charID = 0;
        }
    }
}
*/

void moveGameObject(const gameobj_t* restrict obj, f32 dx, f32 dy)
{
    rAssert(obj && obj->len && obj->data);

    if (!obj || !obj->len || !obj->data)
        return;

    for (u32 i = 0; i < obj->len; i++) {
        obj->data[i].x += dx;
        obj->data[i].y += dy;
    }
}