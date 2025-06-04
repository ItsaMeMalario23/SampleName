#include <stdio.h>
#include <SDL3/SDL.h>

#include <input.h>
#include <worldsim.h>
#include <objects.h>
#include <render/camera.h>
#include <render/render.h>
#include <debug/rdebug.h>

static SDL_GPUGraphicsPipeline* pipeline2D;
static SDL_GPUGraphicsPipeline* pipelineLayers;
static SDL_GPUGraphicsPipeline* pipelineHitbox;

//
static SDL_GPUGraphicsPipeline* pipeline3D;
static SDL_GPUGraphicsPipeline* pipelineOdyssey;
static SDL_GPUTexture* odysseytexture;
static SDL_GPUSampler* odysseysampler;
static SDL_GPUTransferBuffer* transferbuf3D;
static SDL_GPUBuffer* vtxbuf3D;
static camera_t camera;
static context_t* context;
static const obj3D_t* objbuf3D[RENDER_OBJ3D_BUF_SIZE];
static u32 num3Dobjects;
//

static SDL_GPUTexture* asciitex;
static SDL_GPUSampler* sampler;

static SDL_GPUTransferBuffer* asciitransferbuf;
static SDL_GPUBuffer* asciigpubuf;

static SDL_GPUTransferBuffer* boxtransferbuf;
static SDL_GPUBuffer* boxvtxbuf;

static SDL_GPUTextureFormat rfmt;

SDL_Mutex* renderBufLock = NULL;

static gameobj_t* objectbuf;

static u32 num2Dchars;
static u32 numlayerchars;
static u32 numobjects;

static u8 renderbuf[ASCII_OBJ_BUF_SIZE];

// layers
static u8 layerobjects[RENDER_MAX_LAYERS * RENDER_OBJ_PER_LAYER];
static renderlayer_t layers[RENDER_MAX_LAYERS];

static u32 indices[RENDER_MAX_LAYERS];
static f32 scales[RENDER_MAX_LAYERS] = {0.4f, 0.6f, 0.8f, 1.0f, 1.2f, 1.2f};

// hitboxes
static gameobj_t* hitboxes[RENDER_HITBOX_BUF_SIZE];
static u32 numboxes;

static u32 rmode = RENDER_MODE_UNDEF;

static instate_t* input;

void renderInit(context_t* const con, SDL_WindowFlags flags, u32 mode)
{
    // init window
    rfmt = renderInitWindow(con, flags);

    // init render pipelines
    SDL_GPUShader* vert = renderLoadShader(con, "ascii2D.vert", 0, 0, 1, 0);
    SDL_GPUShader* frag = renderLoadShader(con, "ascii2D.frag", 1, 0, 0, 0);

    pipeline2D = renderInit2DPipeline(con->dev, vert, frag, rfmt);

    SDL_ReleaseGPUShader(con->dev, vert);
    SDL_ReleaseGPUShader(con->dev, frag);

    asciitex = renderInitAsciiTexture(con, "pressstart.bmp", &sampler);

    vert = renderLoadShader(con, "box.vert", 0, 0, 0, 0);
    frag = renderLoadShader(con, "box.frag", 0, 0, 0, 0);

    pipelineHitbox = renderInitBoxPipeline(con->dev, vert, frag, rfmt);

    SDL_ReleaseGPUShader(con->dev, vert);
    SDL_ReleaseGPUShader(con->dev, frag);

    vert = renderLoadShader(con, "asciiLayers.vert", 0, 1, 1, 0);
    frag = renderLoadShader(con, "ascii2D.frag", 1, 0, 0, 0);

    pipelineLayers = renderInit2DPipeline(con->dev, vert, frag, rfmt);

    SDL_ReleaseGPUShader(con->dev, vert);
    SDL_ReleaseGPUShader(con->dev, frag);

    if (!pipeline2D || !pipelineHitbox || !pipelineLayers || !asciitex)
        return;

    // init buffers
    asciitransferbuf = SDL_CreateGPUTransferBuffer(
        con->dev,
        &(SDL_GPUTransferBufferCreateInfo) {
            .usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD,
            .size = ASCII_CHAR_BUF_SIZE * sizeof(asciidata_t)
        }
    );

    rAssert(asciitransferbuf);

    asciigpubuf = SDL_CreateGPUBuffer(
        con->dev,
        &(SDL_GPUBufferCreateInfo) {
            .usage = SDL_GPU_BUFFERUSAGE_GRAPHICS_STORAGE_READ,
            .size = ASCII_CHAR_BUF_SIZE * sizeof(asciidata_t)
        }
    );

    rAssert(asciigpubuf);

    boxtransferbuf = SDL_CreateGPUTransferBuffer(
        con->dev,
        &(SDL_GPUTransferBufferCreateInfo) {
            .usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD,
            .size = sizeof(vec2f_t) * 6 * RENDER_HITBOX_BUF_SIZE
        }
    );

    rAssert(boxtransferbuf);

    boxvtxbuf = SDL_CreateGPUBuffer(
        con->dev,
        &(SDL_GPUBufferCreateInfo) {
            .usage = SDL_GPU_BUFFERUSAGE_VERTEX,
            .size = sizeof(vec2f_t) * 6 * RENDER_HITBOX_BUF_SIZE
        }
    );

    rAssert(boxvtxbuf);

    for (u32 i = 0; i < RENDER_MAX_LAYERS; i++) {
        layers[i].numobjects = 0;
        layers[i].objects = layerobjects + (i * RENDER_OBJ_PER_LAYER);
    }

    input = getInputState();
    objectbuf = getObjectBuf();

    rmode = mode;

    vert = renderLoadShader(con, "test3D.vert", 0, 2, 0, 0);
    frag = renderLoadShader(con, "test3D.frag", 0, 0, 0, 0);

    pipeline3D = renderInit3DPipeline(con->dev, vert, frag, rfmt);

    if (!pipeline3D)
        return;

    SDL_ReleaseGPUShader(con->dev, vert);
    SDL_ReleaseGPUShader(con->dev, frag);

    odysseytexture = renderInitOdysseyTexture(con, &odysseysampler, rfmt);

    rAssert(odysseytexture);
    rAssert(odysseysampler);

    vert = renderLoadShader(con, "odyssey.vert", 0, 2, 0, 0);
    frag = renderLoadShader(con, "odyssey.frag", 1, 0, 0, 0);

    pipelineOdyssey = renderInitOdysseyPipeline(con->dev, vert, frag, rfmt);

    if (!pipelineOdyssey)
        return;

    SDL_ReleaseGPUShader(con->dev, vert);
    SDL_ReleaseGPUShader(con->dev, frag);

    context = getContext();
}

static inline void draw2D(SDL_GPURenderPass* const renderpass)
{
    SDL_BindGPUGraphicsPipeline(renderpass, pipeline2D);
    SDL_BindGPUVertexStorageBuffers(renderpass, 0, &asciigpubuf, 1);
    SDL_BindGPUFragmentSamplers(renderpass, 0, &(SDL_GPUTextureSamplerBinding) { .texture = asciitex, .sampler = sampler }, 1);

    SDL_DrawGPUPrimitives(renderpass, num2Dchars * 6, 1, 0, 0);
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

    SDL_BindGPUVertexBuffers(renderpass, 0, &(SDL_GPUBufferBinding) { .buffer = vtxbuf3D, .offset = 0 }, 1);
    SDL_BindGPUFragmentSamplers(renderpass, 0, &(SDL_GPUTextureSamplerBinding) { .texture = odysseytexture, .sampler = odysseysampler }, 1);

    SDL_PushGPUVertexUniformData(cmdbuf, 0, &camera.rendermat, sizeof(mat4_t));
    SDL_PushGPUVertexUniformData(cmdbuf, 1, &wall_3D->transform, sizeof(mat4_t));

    bool fucked = 0;

    if (!wall_3D->vtxbufoffset)     // TODO why tf is this happening?
        fucked = 1;

    SDL_DrawGPUPrimitives(renderpass, wall_3D->numvtx, 1, fucked ? 24 : wall_3D->vtxbufoffset, 0);
}

static inline void drawBoxes(SDL_GPURenderPass* const renderpass)
{
    SDL_BindGPUGraphicsPipeline(renderpass, pipelineHitbox);
    SDL_BindGPUVertexBuffers(renderpass, 0, &(SDL_GPUBufferBinding) { .buffer = boxvtxbuf, .offset = 0 }, 1);

    SDL_DrawGPUPrimitives(renderpass, numboxes * 6, 1, 0, 0);
}

static inline void transferAsciiBuf(SDL_GPUCopyPass* const copypass, SDL_GPUDevice* const dev)
{
    asciidata_t* transfmem = SDL_MapGPUTransferBuffer(dev, asciitransferbuf, true);

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
            .size = num2Dchars * sizeof(asciidata_t)
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

    vec2f_t* transfmem = SDL_MapGPUTransferBuffer(dev, boxtransferbuf, false);

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
        false
    );
}

static inline void drawOdyssey(SDL_GPUCommandBuffer* cmdbuf, SDL_GPUTexture* swapchain, SDL_GPUDevice* dev)
{
    // 2D mario scene render pass
    SDL_GPURenderPass* renderpass = SDL_BeginGPURenderPass(
        cmdbuf,
        &(SDL_GPUColorTargetInfo) {
            .texture = odysseytexture,
            .cycle = false,
            .load_op = SDL_GPU_LOADOP_CLEAR,
            .store_op = SDL_GPU_STOREOP_STORE,
            .clear_color = (SDL_FColor) { 0, 0, 0, 1 }
        },
        1,
        NULL
    );

    draw2D(renderpass);

    SDL_EndGPURenderPass(renderpass);

    SDL_GPUFence* f = SDL_SubmitGPUCommandBufferAndAcquireFence(cmdbuf);

    cmdbuf = SDL_AcquireGPUCommandBuffer(dev);

    rAssert(cmdbuf);

    // 3D scene render pass
    renderpass = SDL_BeginGPURenderPass(
        cmdbuf,
        &(SDL_GPUColorTargetInfo) {
            .texture = swapchain,
            .cycle = false,
            .load_op = SDL_GPU_LOADOP_CLEAR,
            .store_op = SDL_GPU_STOREOP_STORE,
            .clear_color = (SDL_FColor) { RENDER_CLEAR_COLOR_R, RENDER_CLEAR_COLOR_G, RENDER_CLEAR_COLOR_B, RENDER_CLEAR_COLOR_A }
        },
        1,
        NULL
    );

    draw3D(renderpass, cmdbuf);

    SDL_WaitForGPUFences(dev, false, &f, 1);

    drawOdyssey2D(renderpass, cmdbuf);

    SDL_EndGPURenderPass(renderpass);
    SDL_SubmitGPUCommandBuffer(cmdbuf);
}

void renderDraw(context_t* const con)
{
    SDL_GPUCommandBuffer* cmdbuf = SDL_AcquireGPUCommandBuffer(con->dev);

    if (rmode & RENDER_MODE_2D || rmode & RENDER_MODE_LAYERED || rmode & RENDER_MODE_HITBOXES || rmode & RENDER_MODE_ODYSSEY) {
        if (!cmdbuf) {
            SDL_Log("[ERROR] Failed to acquire copy cmd buf: %s", SDL_GetError());
            return;
        }

        // copy pass
        SDL_GPUCopyPass* copypass = SDL_BeginGPUCopyPass(cmdbuf);

        if (rmode & RENDER_MODE_2D || rmode & RENDER_MODE_ODYSSEY)
            transferAsciiBuf(copypass, con->dev);
        
        if (rmode & RENDER_MODE_LAYERED)
            transferLayerBuf(copypass, con->dev);

        if (rmode & RENDER_MODE_HITBOXES && numboxes)
            transferBoxVtxBuf(copypass, con->dev);

        SDL_EndGPUCopyPass(copypass);

        // submit buffers as early as possible
        SDL_SubmitGPUCommandBuffer(cmdbuf);

        cmdbuf = SDL_AcquireGPUCommandBuffer(con->dev);
    }

    if (!cmdbuf) {
        SDL_Log("[ERROR] Failed to acquire render cmd buf: %s", SDL_GetError());
        return;
    }

    SDL_GPUTexture* swapchain;

    if (!SDL_WaitAndAcquireGPUSwapchainTexture(cmdbuf, con->window, &swapchain, NULL, NULL)) {
        SDL_Log("[ERROR] Failed to acquire swapchain texture: %s", SDL_GetError());
        return;
    }

    if (!swapchain) {
        SDL_SubmitGPUCommandBuffer(cmdbuf);
        return;
    }

    if (rmode & RENDER_MODE_ODYSSEY) {
        drawOdyssey(cmdbuf, swapchain, con->dev);
        return;
    }

    // render pass
    SDL_GPURenderPass* renderpass = SDL_BeginGPURenderPass(
        cmdbuf,
        &(SDL_GPUColorTargetInfo) {
            .texture = swapchain,
            .cycle = false,
            .load_op = SDL_GPU_LOADOP_CLEAR,
            .store_op = SDL_GPU_STOREOP_STORE,
            .clear_color = (SDL_FColor) { RENDER_CLEAR_COLOR_R, RENDER_CLEAR_COLOR_G, RENDER_CLEAR_COLOR_B, RENDER_CLEAR_COLOR_A }
        },
        1,
        NULL
    );

    if (rmode & RENDER_MODE_2D)
        draw2D(renderpass);
    
    if (rmode & RENDER_MODE_LAYERED)
        drawLayers(renderpass, cmdbuf);

    if (rmode & RENDER_MODE_HITBOXES && numboxes)
        drawBoxes(renderpass);

    if (rmode & RENDER_MODE_3D && num3Dobjects)
        draw3D(renderpass, cmdbuf);

    SDL_EndGPURenderPass(renderpass);

    SDL_SubmitGPUCommandBuffer(cmdbuf);
}

u32 getRenderMode(void)
{
    return rmode;
}

camera_t* getCamera(void)
{
    return &camera;
}

// implicitly resets render buffers
void renderMode(u32 mode)
{
    rmode = mode;
    resetRenderBuffers();
}

void renderToggleHitboxes(void)
{
    rmode ^= RENDER_MODE_HITBOXES;
}

void renderCleanup(context_t* const con)
{
    SDL_ReleaseGPUGraphicsPipeline(con->dev, pipeline2D);
    SDL_ReleaseGPUGraphicsPipeline(con->dev, pipelineLayers);
    SDL_ReleaseGPUGraphicsPipeline(con->dev, pipelineHitbox);
    SDL_ReleaseGPUTransferBuffer(con->dev, asciitransferbuf);
    SDL_ReleaseGPUTransferBuffer(con->dev, boxtransferbuf);
    SDL_ReleaseGPUBuffer(con->dev, asciigpubuf);
    SDL_ReleaseGPUBuffer(con->dev, boxvtxbuf);
    SDL_ReleaseGPUTexture(con->dev, asciitex);
    SDL_ReleaseGPUSampler(con->dev, sampler);

    if (renderBufLock)
        SDL_DestroyMutex(renderBufLock);

    renderCleanupWindow(con);
}

//
//  Objects
//
void setup3DVtxBuf(obj3D_t* objects, u32 numobjects)
{
    if (!objects) {
        SDL_Log("[ERROR] setup3DVtxBuf null ref");
        return;
    }

    if (!numobjects || numobjects > RENDER_OBJ3D_BUF_SIZE) {
        SDL_Log("[ERROR] Unable to create %ld 3D elements, max %ld", numobjects, RENDER_OBJ3D_BUF_SIZE);
        return;
    }

    u32 len = 0;
    const obj3D_t* bound = objects + numobjects;

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
            add3DObjectToRenderBuf(i);
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

void add3DObjectToRenderBuf(const obj3D_t* restrict object)
{
    rAssert(object);

    if (num3Dobjects >= RENDER_OBJ3D_BUF_SIZE) {
        SDL_Log("[ERROR] Failed to add 3D object to render buf, buffer size %ld full", RENDER_OBJ3D_BUF_SIZE);
        return;
    }

    objbuf3D[num3Dobjects++] = object;
}

void addObjectToRenderBuf(u32 object)
{
    if (object >= ASCII_OBJ_BUF_SIZE || numobjects >= ASCII_OBJ_BUF_SIZE) {
        SDL_Log("[ERROR] Failed to add object to render buf");
        return;
    }

    renderbuf[numobjects++] = object;
}

void addObjectToLayer(u32 object, u32 layer)
{
    if (object >= ASCII_OBJ_BUF_SIZE || layer >= RENDER_MAX_LAYERS || layers[layer].numobjects >= RENDER_OBJ_PER_LAYER) {
        SDL_Log("[ERROR] Failed to add object to layer");
        return;
    }

    layers[layer].objects[layers[layer].numobjects++] = object;
}

void addHitbox(gameobj_t* object)
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

void removeHitbox(gameobj_t* object)
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

void resetRenderBuffers(void)
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