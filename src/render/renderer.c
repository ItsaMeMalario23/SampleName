#include <stdio.h>
#include <SDL3/SDL.h>

#include <input.h>
#include <render/renderer.h>
#include <debug/rdebug.h>

static SDL_GPUGraphicsPipeline* pipeline2D;
static SDL_GPUGraphicsPipeline* pipelineLayers;
static SDL_GPUGraphicsPipeline* pipelineHitbox;

static SDL_GPUTexture* asciitex;
static SDL_GPUSampler* sampler;

static SDL_GPUTransferBuffer* asciitransferbuf;
static SDL_GPUBuffer* asciigpubuf;

static SDL_GPUTransferBuffer* boxtransferbuf;
static SDL_GPUBuffer* boxvtxbuf;

static SDL_GPUTextureFormat rfmt;

SDL_Mutex* renderBufLock = NULL;

// game objects
static gameobj_t objectbuf[ASCII2D_OBJ_BUF_SIZE];
static u32 objidx;
static u32 objfragmented;

// ascii chars
static asciidata_t charbuf[ASCII2D_BUF_SIZE];
static u32 numchars;
static const asciidata_t* charbound = charbuf + ASCII2D_BUF_SIZE;

// hitboxes
static gameobj_t* hitboxes[RENDER_HITBOX_BUF_SIZE];
static u32 numboxes;

// layers
static gameobj_t* layerobjects[RENDER_MAX_LAYERS * RENDER_OBJ_PER_LAYER];
static renderlayer_t layers[RENDER_MAX_LAYERS];
static u32 indices[RENDER_MAX_LAYERS];
static f32 scales[RENDER_MAX_LAYERS] = {0.4f, 0.6f, 0.8f, 1.0f, 1.2f, 1.2f}; 

static u32 rmode = RENDER_MODE_LAYERED;

static instate_t* input;

void renderInit(context_t* restrict con, SDL_WindowFlags flags)
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

    // init buffers
    asciitransferbuf = SDL_CreateGPUTransferBuffer(
        con->dev,
        &(SDL_GPUTransferBufferCreateInfo) {
            .usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD,
            .size = ASCII2D_BUF_SIZE * sizeof(asciidata_t)
        }
    );

    rAssert(asciitransferbuf);

    asciigpubuf = SDL_CreateGPUBuffer(
        con->dev,
        &(SDL_GPUBufferCreateInfo) {
            .usage = SDL_GPU_BUFFERUSAGE_GRAPHICS_STORAGE_READ,
            .size = ASCII2D_BUF_SIZE * sizeof(asciidata_t)
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
}

static inline void draw2D(SDL_GPURenderPass* restrict renderpass)
{
    SDL_BindGPUGraphicsPipeline(renderpass, pipeline2D);
    SDL_BindGPUVertexStorageBuffers(renderpass, 0, &asciigpubuf, 1);
    SDL_BindGPUFragmentSamplers(renderpass, 0, &(SDL_GPUTextureSamplerBinding) { .texture = asciitex, .sampler = sampler }, 1);

    SDL_DrawGPUPrimitives(renderpass, numchars * 6, 1, 0, 0);
}

static inline void drawLayers(SDL_GPURenderPass* restrict renderpass, SDL_GPUCommandBuffer* restrict cmdbuf)
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
    SDL_DrawGPUPrimitives(renderpass, (numchars - indices[5]) * 6, 1, indices[5] * 6, 0);
}

static inline void drawBoxes(SDL_GPURenderPass* restrict renderpass)
{
    SDL_BindGPUGraphicsPipeline(renderpass, pipelineHitbox);
    SDL_BindGPUVertexBuffers(renderpass, 0, &(SDL_GPUBufferBinding) { .buffer = boxvtxbuf, .offset = 0 }, 1);

    SDL_DrawGPUPrimitives(renderpass, numboxes * 6, 1, 0, 0);
}

static inline void transferAsciiBuf(SDL_GPUCopyPass* restrict copypass, SDL_GPUDevice* restrict dev)
{
    rAssert(numchars < ASCII2D_BUF_SIZE);

    asciidata_t* transfmem = SDL_MapGPUTransferBuffer(dev, asciitransferbuf, true);

    SDL_LockMutex(renderBufLock);
    memcpy(transfmem, charbuf, sizeof(asciidata_t) * numchars);
    SDL_UnlockMutex(renderBufLock);

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
            .size = numchars * sizeof(asciidata_t)
        },
        true
    );
}

static inline void transferLayerBuf(SDL_GPUCopyPass* restrict copypass, SDL_GPUDevice* restrict dev)
{
    rAssert(numchars < ASCII2D_BUF_SIZE);

    asciidata_t* transfmem = SDL_MapGPUTransferBuffer(dev, asciitransferbuf, true);

    memset(indices, 0, sizeof(indices));
    numchars = 0;

    SDL_LockMutex(renderBufLock);

    for (u32 i = 0; i < RENDER_MAX_LAYERS; i++)
    {
        indices[i] = numchars;

        for (gameobj_t** k = layers[i].objects; k < layers[i].objects + layers[i].numobjects; k++)
        {
            memcpy(transfmem + numchars, (*k)->data, sizeof(asciidata_t) * (*k)->len);
            numchars += (*k)->len;
        }
    }

    SDL_UnlockMutex(renderBufLock);

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
            .size = numchars * sizeof(asciidata_t)
        },
        true
    );
}

static inline void transferBoxVtxBuf(SDL_GPUCopyPass* restrict copypass, SDL_GPUDevice* restrict dev)
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

void renderDraw(context_t* restrict con)
{
    SDL_GPUCommandBuffer* cmdbuf = SDL_AcquireGPUCommandBuffer(con->dev);

    if (!cmdbuf) {
        SDL_Log("Failed to acquire copy cmd buf: %s", SDL_GetError());
        return;
    }

    // copy pass
    SDL_GPUCopyPass* copypass = SDL_BeginGPUCopyPass(cmdbuf);

    if (numchars && rmode == RENDER_MODE_2D)
        transferAsciiBuf(copypass, con->dev);
    else if (numchars && rmode == RENDER_MODE_LAYERED)
        transferLayerBuf(copypass, con->dev);

    if (numboxes)
        transferBoxVtxBuf(copypass, con->dev);

    SDL_EndGPUCopyPass(copypass);

    // submit buffers as early as possible
    SDL_SubmitGPUCommandBuffer(cmdbuf);

    cmdbuf = SDL_AcquireGPUCommandBuffer(con->dev);

    if (!cmdbuf) {
        SDL_Log("Failed to acquire render cmd buf: %s", SDL_GetError());
        return;
    }

    SDL_GPUTexture* swapchain;

    if (!SDL_WaitAndAcquireGPUSwapchainTexture(cmdbuf, con->window, &swapchain, NULL, NULL)) {
        SDL_Log("Failed to acquire swapchain texture: %s", SDL_GetError());
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
            .cycle = false,
            .load_op = SDL_GPU_LOADOP_CLEAR,
            .store_op = SDL_GPU_STOREOP_STORE,
            .clear_color = (SDL_FColor) { RENDER_CLEAR_COLOR_R, RENDER_CLEAR_COLOR_G, RENDER_CLEAR_COLOR_B, RENDER_CLEAR_COLOR_A }
        },
        1,
        NULL
    );

    if (numchars && rmode == RENDER_MODE_2D)
        draw2D(renderpass);
    else if (numchars && rmode == RENDER_MODE_LAYERED)
        drawLayers(renderpass, cmdbuf);

    if (numboxes)
        drawBoxes(renderpass);

    SDL_EndGPURenderPass(renderpass);

    SDL_SubmitGPUCommandBuffer(cmdbuf);
}

void renderCleanup(context_t* restrict con)
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
//  worldsim
//

void addObjectToLayer(gameobj_t* object, u32 layer)
{
    if (!object || layer >= RENDER_MAX_LAYERS || layers[layer].numobjects >= RENDER_OBJ_PER_LAYER) {
        SDL_Log("Failed to add object to layer");
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

    SDL_Log("Failed to add hitbox, buffer full");
}

void removeHitbox(gameobj_t* object)
{
    if (!numboxes) {
        SDL_Log("Failed to remove hitbox, no hitboxes in buf");
        return;
    }

    for (u32 i = 0; i < RENDER_HITBOX_BUF_SIZE; i++) {
        if (hitboxes[i] == object) {
            hitboxes[i] = NULL;
            numboxes--;
            return;
        }
    }

    SDL_Log("Failed to remove hitbox, object has no hitbox");
}

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


gameobj_t* addGameObject(const ascii2info_t* info, u32 len, f32 x, f32 y)
{
    rAssert(info);
    rAssert(len);

    if (numchars + len >= ASCII2D_BUF_SIZE) {
        SDL_Log("ERROR: Ascii buffer full");
        return NULL;
    }

    gameobj_t* obj = NULL;

    if (objfragmented)
    {
        for (u32 i = 0; i < ASCII2D_OBJ_BUF_SIZE; i++) {
            if (!objectbuf[i].data) {
                obj = objectbuf + i;
                break;
            }
        }

        if (!obj) {
            SDL_Log("ERROR: Object buffer full");
            return NULL;
        }
    }
    else if (objidx >= ASCII2D_OBJ_BUF_SIZE)
    {
        SDL_Log("ERROR: Object buffer full");
        return NULL;
    }
    else
    {
        obj = objectbuf + objidx++;
    }

    rAssert(obj);

    obj->data = charbuf + numchars;
    obj->len = len;
    obj->xscale = 0.0f;
    obj->yscale = 0.0f;
    obj->dx = 0.0f;
    obj->dy = 0.0f;
    obj->x = x;
    obj->y = y;

    numchars += len;

    for (asciidata_t* i = obj->data; i < obj->data + len; i++, info++) {
        i->r = (f32) ((u8) (info->color >> 24)) / 255.0f;
        i->g = (f32) ((u8) (info->color >> 16)) / 255.0f;
        i->b = (f32) ((u8) (info->color >> 8)) / 255.0f;
        i->a = (f32) ((u8) info->color) / 255.0f;
        i->scale = 1.0f / 80.0f;
        i->x = info->pos.x + x;
        i->y = info->pos.y + y;
        
        if (info->charID < 32)
            i->charID = 0;
        else
            i->charID = info->charID - 32;
    }

    SDL_Log("INFO: Successfully allocated ascii object of length %ld", obj->len);

    return obj;
}

gameobj_t* addGameObjectStruct(const objectinfo_t* restrict object)
{
    rAssert(object);
    rAssert(object->len);
    rAssert(object->data);

    gameobj_t* obj = addGameObject(object->data, object->len, object->x, object->y);

    if (!obj)
        return NULL;

    obj->xscale = object->xscale;
    obj->yscale = object->yscale;

    return obj;
}

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

void updateGameObjectPos(gameobj_t* restrict obj)
{
    rAssert(obj && obj->len && obj->data);

    if (obj->dx < EPSILON && obj->dx > -EPSILON && obj->dy < EPSILON && obj->dy > -EPSILON)
        return;

    for (u32 i = 0; i < obj->len; i++) {
        obj->data[i].x += obj->dx;
        obj->data[i].y += obj->dy;
    }

    obj->x += obj->dx;
    obj->y += obj->dy;
    obj->dx = 0.0f;
    obj->dy = 0.0f;
}

void removeGameObject(gameobj_t* restrict obj)
{
    if (!obj || !obj->len || obj->data)
        return;

    if (obj < objectbuf || obj >= objectbuf + ASCII2D_OBJ_BUF_SIZE) {
        SDL_Log("ERROR: could not remove game object, pointer outside of object bounds");
        return;
    }

    rAssert(obj->len <= numchars);
    rAssert(obj->data + obj->len <= charbound && obj->data >= charbuf);

    if (obj->data + obj->len == charbound) {
        objidx = obj + 1 - objectbuf;
        objfragmented = 0;
    }

    if (!objfragmented)
    {
        for (gameobj_t* i = objectbuf; i < objectbuf + objidx; i++) {
            if (i == obj)
                continue;

            if (i->data && obj->data < i->data) {
                objfragmented = 1;
                break;
            }
        }

        if (!objfragmented) {
            numchars -= obj->len;
            obj->data = NULL;
            obj->len = 0;
            objidx--;
            return;
        }
    }

    // defragment
    memmove(obj->data, obj->data + obj->len, sizeof(asciidata_t) * ((obj->data + obj->len) - charbound));

    numchars -= obj->len;
    obj->data = NULL;
    obj->len = 0;
}

void resetAllGameObjects(void)
{
    numchars = 0;
    objidx = 0;
    objfragmented = 0;

    memset(objectbuf, 0, sizeof(gameobj_t) * ASCII2D_OBJ_BUF_SIZE);
}