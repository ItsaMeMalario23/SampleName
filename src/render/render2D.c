#include <stdio.h>
#include <SDL3/SDL.h>

#include <render/render2D.h>
#include <debug/rdebug.h>

static SDL_GPUGraphicsPipeline* pipeline;
static SDL_GPUTexture* texture;
static SDL_GPUSampler* sampler;

// buffers for frag shader
static SDL_GPUTransferBuffer* fragtransferbuf;
static SDL_GPUBuffer* fragdatabuf;

// for hitboxes
static SDL_GPUGraphicsPipeline* boxpipeline;
static SDL_GPUBuffer* vtxbuf;
static SDL_GPUBuffer* idxbuf;

// character buf
static asciidata_t charbuf[ASCII2D_BUF_SIZE];
static u32 numchars;
static const asciidata_t* charbound = charbuf + ASCII2D_BUF_SIZE;

SDL_Mutex* renderBufLock = NULL;

// game object buf
static gameobj_t objbuf[ASCII2D_OBJ_BUF_SIZE];
static u32 objidx;
static u32 objfragmented;

void renderInitWindow(context_t* restrict con, SDL_WindowFlags flags)
{
    con->dev = SDL_CreateGPUDevice(SDL_GPU_SHADERFORMAT_SPIRV | SDL_GPU_SHADERFORMAT_DXIL | SDL_GPU_SHADERFORMAT_MSL, 0, NULL);

    if (!con->dev) {
        SDL_Log("Failed to create GPU device");
        return;
    }

    con->window = SDL_CreateWindow(con->name, con->width, con->height, flags);

    if (!con->window) {
        SDL_Log("Failed to create window");
        return;
    }

    if (!SDL_ClaimWindowForGPUDevice(con->dev, con->window)) {
        SDL_Log("Failed to claim window for GPU device");
        return;
    }

    renderBufLock = SDL_CreateMutex();

    if (!renderBufLock) {
        SDL_Log("Failed to create render buffer lock");
        return;
    }

    con->path = SDL_GetBasePath();
}

void renderInitPipeline(context_t* restrict con)
{
    SDL_GPUPresentMode pmode = SDL_GPU_PRESENTMODE_VSYNC;

    if (SDL_WindowSupportsGPUPresentMode(con->dev, con->window, SDL_GPU_PRESENTMODE_IMMEDIATE))
        pmode = SDL_GPU_PRESENTMODE_IMMEDIATE;
    else if (SDL_WindowSupportsGPUPresentMode(con->dev, con->window, SDL_GPU_PRESENTMODE_MAILBOX))
        pmode = SDL_GPU_PRESENTMODE_MAILBOX;

    SDL_SetGPUSwapchainParameters(con->dev, con->window, SDL_GPU_SWAPCHAINCOMPOSITION_SDR, pmode);

    SDL_GPUShader* vertshader = renderLoadShader(con, "ascii2D.vert", 0, 0, 1, 0);
    SDL_GPUShader* fragshader = renderLoadShader(con, "ascii2D.frag", 1, 0, 0, 0);

    if (!vertshader || !fragshader) {
        SDL_Log("Failed to create shaders");
        return;
    }

    pipeline = SDL_CreateGPUGraphicsPipeline(
        con->dev,
        &(SDL_GPUGraphicsPipelineCreateInfo) {
            .target_info = (SDL_GPUGraphicsPipelineTargetInfo) {
                .num_color_targets = 1,
                .color_target_descriptions = (SDL_GPUColorTargetDescription[]) {{
                    .format = SDL_GetGPUSwapchainTextureFormat(con->dev, con->window),
                    /*
                    .blend_state = {
                        .enable_blend = true,
                        .color_blend_op = SDL_GPU_BLENDOP_ADD,
                        .alpha_blend_op = SDL_GPU_BLENDOP_ADD,
                        .src_color_blendfactor = SDL_GPU_BLENDFACTOR_SRC_ALPHA,
                        .dst_color_blendfactor = SDL_GPU_BLENDFACTOR_ONE_MINUS_SRC_ALPHA,
                        .src_alpha_blendfactor = SDL_GPU_BLENDFACTOR_SRC_ALPHA,
                        .dst_alpha_blendfactor = SDL_GPU_BLENDFACTOR_ONE_MINUS_SRC_ALPHA
                    }
                        */
                }}
            },
            .primitive_type = SDL_GPU_PRIMITIVETYPE_TRIANGLELIST,
            .vertex_shader = vertshader,
            .fragment_shader = fragshader
        }
    );

    SDL_ReleaseGPUShader(con->dev, vertshader);
    SDL_ReleaseGPUShader(con->dev, fragshader);

    SDL_Surface* img = renderLoadBmp(con, "pressstart.bmp");

    if (!img) {
        SDL_Log("Failed to load image");
        return;
    }

    SDL_GPUTransferBuffer* textransferbuf = SDL_CreateGPUTransferBuffer(con->dev, &(SDL_GPUTransferBufferCreateInfo) {.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD, .size = img->w * img->h * 4});

    u8* textransferptr = SDL_MapGPUTransferBuffer(con->dev, textransferbuf, 0);

    SDL_memcpy(textransferptr, img->pixels, img->w * img->h * 4);
    
    SDL_UnmapGPUTransferBuffer(con->dev, textransferbuf);

    texture = SDL_CreateGPUTexture(
        con->dev,
        &(SDL_GPUTextureCreateInfo) {
            .type = SDL_GPU_TEXTURETYPE_2D,
            .format = SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM,
            .width = img->w,
            .height = img->h,
            .layer_count_or_depth = 1,
            .num_levels = 1,
            .usage = SDL_GPU_TEXTUREUSAGE_SAMPLER
        }
    );

    sampler = SDL_CreateGPUSampler(
        con->dev,
        &(SDL_GPUSamplerCreateInfo) {
            .min_filter = SDL_GPU_FILTER_NEAREST,
            .mag_filter = SDL_GPU_FILTER_NEAREST,
            .mipmap_mode = SDL_GPU_SAMPLERMIPMAPMODE_NEAREST,
            .address_mode_u = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE,
            .address_mode_v = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE,
            .address_mode_w = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE
        }
    );

    fragtransferbuf = SDL_CreateGPUTransferBuffer(con->dev, &(SDL_GPUTransferBufferCreateInfo) {.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD, .size = ASCII2D_BUF_SIZE * sizeof(asciidata_t)});
    fragdatabuf = SDL_CreateGPUBuffer(con->dev, &(SDL_GPUBufferCreateInfo) {.usage = SDL_GPU_BUFFERUSAGE_GRAPHICS_STORAGE_READ, .size = ASCII2D_BUF_SIZE * sizeof(asciidata_t)});

    SDL_GPUCommandBuffer* cmdbuf = SDL_AcquireGPUCommandBuffer(con->dev);
    SDL_GPUCopyPass* copypass = SDL_BeginGPUCopyPass(cmdbuf);

    SDL_UploadToGPUTexture(
        copypass,
        &(SDL_GPUTextureTransferInfo) {
            .transfer_buffer = textransferbuf,
            .offset = 0
        },
        &(SDL_GPUTextureRegion) {
            .texture = texture,
            .w = img->w,
            .h = img->h,
            .d = 1
        },
        0
    );

    SDL_EndGPUCopyPass(copypass);
    SDL_SubmitGPUCommandBuffer(cmdbuf);

    SDL_DestroySurface(img);
    SDL_ReleaseGPUTransferBuffer(con->dev, textransferbuf);

    // hitbox pipeline
    vertshader = renderLoadShader(con, "box.vert", 0, 0, 0, 0);
    fragshader = renderLoadShader(con, "box.frag", 0, 0, 0, 0);

    if (!vertshader || !fragshader) {
        SDL_Log("Failed to create hitbox shaders");
        return;
    }

    boxpipeline = SDL_CreateGPUGraphicsPipeline(
        con->dev,
        &(SDL_GPUGraphicsPipelineCreateInfo) {
            .target_info = {
                .num_color_targets = 1,
                .color_target_descriptions = (SDL_GPUColorTargetDescription[]) {{
                    .format = SDL_GetGPUSwapchainTextureFormat(con->dev, con->window)
                }}
            },
            .vertex_input_state = (SDL_GPUVertexInputState) {
                .num_vertex_buffers = 1,
                .vertex_buffer_descriptions = (SDL_GPUVertexBufferDescription[]) {{
                    .slot = 0,
                    .input_rate = SDL_GPU_VERTEXINPUTRATE_VERTEX,
                    .instance_step_rate = 0,
                    .pitch = sizeof(vec2f_t),
                }},
                .num_vertex_attributes = 1,
                .vertex_attributes = (SDL_GPUVertexAttribute[]) {{
                    .buffer_slot = 0,
                    .format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT2,
                    .location = 0,
                    .offset = 0,
                }}
            },
            .rasterizer_state.fill_mode = SDL_GPU_FILLMODE_LINE,
            .primitive_type = SDL_GPU_PRIMITIVETYPE_TRIANGLELIST,
            .vertex_shader = vertshader,
            .fragment_shader = fragshader
        }
    );

    if (!boxpipeline) {
        SDL_Log("Failed to create box pipeline");
        return;
    }

    vtxbuf = SDL_CreateGPUBuffer(con->dev, &(SDL_GPUBufferCreateInfo) {.usage = SDL_GPU_BUFFERUSAGE_VERTEX, .size = sizeof(vec2f_t) * 4});
    idxbuf = SDL_CreateGPUBuffer(con->dev, &(SDL_GPUBufferCreateInfo) {.usage = SDL_GPU_BUFFERUSAGE_INDEX, .size = sizeof(u16) * 6});

    SDL_GPUTransferBuffer* transfbuf = SDL_CreateGPUTransferBuffer(
        con->dev,
        &(SDL_GPUTransferBufferCreateInfo) {
            .usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD,
            .size = (sizeof(f32) * 8) + (sizeof(u16) * 6)
        }
    );

    vec2f_t* transfdata = SDL_MapGPUTransferBuffer(con->dev, transfbuf, false);

    transfdata[0] = (vec2f_t) {-1.0f,  1.0f};
    transfdata[1] = (vec2f_t) { 1.0f,  1.0f};
    transfdata[2] = (vec2f_t) { 1.0f, -1.0f};
    transfdata[3] = (vec2f_t) {-1.0f, -1.0f};

    u16* idxdata = (u16*) &transfdata[4];

    idxdata[0] = 0;
    idxdata[1] = 1;
    idxdata[2] = 2;
    idxdata[3] = 0;
    idxdata[4] = 2;
    idxdata[5] = 3;

    SDL_UnmapGPUTransferBuffer(con->dev, transfbuf);

    cmdbuf = SDL_AcquireGPUCommandBuffer(con->dev);
    copypass = SDL_BeginGPUCopyPass(cmdbuf);

    SDL_UploadToGPUBuffer(
        copypass,
        &(SDL_GPUTransferBufferLocation) {
            .transfer_buffer = transfbuf,
            .offset = 0
        },
        &(SDL_GPUBufferRegion) {
            .buffer = vtxbuf,
            .offset = 0,
            .size = sizeof(vec2f_t) * 4
        },
        false
    );

    SDL_UploadToGPUBuffer(
        copypass,
        &(SDL_GPUTransferBufferLocation) {
            .transfer_buffer = transfbuf,
            .offset = sizeof(vec2f_t) * 4
        },
        &(SDL_GPUBufferRegion) {
            .buffer = idxbuf,
            .offset = 0,
            .size = sizeof(u16) * 6
        },
        false
    );

    SDL_EndGPUCopyPass(copypass);
    SDL_SubmitGPUCommandBuffer(cmdbuf);
    SDL_ReleaseGPUTransferBuffer(con->dev, transfbuf);
    SDL_ReleaseGPUShader(con->dev, vertshader);
    SDL_ReleaseGPUShader(con->dev, fragshader);
}

static u32 boxmode = 1;
static f32 hitbox[4];

void setHitbox(f32 x, f32 y, f32 dx, f32 dy)
{
    hitbox[0] = x;
    hitbox[1] = y;
    hitbox[2] = dx;
    hitbox[3] = dy;
}

void renderDraw(context_t* restrict con)
{
    SDL_GPUCommandBuffer* cmdbuf = SDL_AcquireGPUCommandBuffer(con->dev);

    if (!cmdbuf) {
        SDL_Log("Failed to acquire cmd buf: %s", SDL_GetError());
        return;
    }

    SDL_GPUTexture* swapchain;

    if (!SDL_WaitAndAcquireGPUSwapchainTexture(cmdbuf, con->window, &swapchain, NULL, NULL)) {
        SDL_Log("Failed to acquire swapchain texture: %s", SDL_GetError());
        return;
    }

    if (numchars && swapchain) {
        asciidata_t* data = SDL_MapGPUTransferBuffer(con->dev, fragtransferbuf, true);

        rAssert(numchars < ASCII2D_BUF_SIZE);

        SDL_LockMutex(renderBufLock);
        memcpy(data, charbuf, sizeof(asciidata_t) * numchars);
        SDL_UnlockMutex(renderBufLock);

        SDL_UnmapGPUTransferBuffer(con->dev, fragtransferbuf);

        SDL_GPUCopyPass* copypass = SDL_BeginGPUCopyPass(cmdbuf);

        SDL_UploadToGPUBuffer(
            copypass,
            &(SDL_GPUTransferBufferLocation) {
                .transfer_buffer = fragtransferbuf,
                .offset = 0
            },
            &(SDL_GPUBufferRegion) {
                .buffer = fragdatabuf,
                .offset = 0,
                .size = numchars * sizeof(asciidata_t),
            },
            true
        );

        SDL_EndGPUCopyPass(copypass);

        SDL_GPURenderPass* renderpass = SDL_BeginGPURenderPass(
            cmdbuf,
            &(SDL_GPUColorTargetInfo) {
                .texture = swapchain,
                .cycle = false,
                .load_op = SDL_GPU_LOADOP_CLEAR,
                .store_op = SDL_GPU_STOREOP_STORE,
                .clear_color = (SDL_FColor) {0.0f, 0.0f, 0.0f, 1.0f}
            },
            1,
            NULL
        );

        SDL_BindGPUGraphicsPipeline(renderpass, pipeline);
        SDL_BindGPUVertexStorageBuffers(renderpass, 0, &fragdatabuf, 1);
        SDL_BindGPUFragmentSamplers(renderpass, 0, &(SDL_GPUTextureSamplerBinding) {.texture = texture, .sampler = sampler}, 1);

        SDL_DrawGPUPrimitives(renderpass, numchars * 6, 1, 0, 0);

        SDL_EndGPURenderPass(renderpass);
    }

    if (boxmode && swapchain) {
        SDL_GPUTransferBuffer* transfbuf = SDL_CreateGPUTransferBuffer(
            con->dev,
            &(SDL_GPUTransferBufferCreateInfo) {
                .usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD,
                .size = (sizeof(f32) * 8)
            }
        );
    
        vec2f_t* transfdata = SDL_MapGPUTransferBuffer(con->dev, transfbuf, false);
    
        transfdata[0] = (vec2f_t) {hitbox[0], hitbox[1] + hitbox[3]};
        transfdata[1] = (vec2f_t) {hitbox[0] + hitbox[2], hitbox[1] + hitbox[3]};
        transfdata[2] = (vec2f_t) {hitbox[0] + hitbox[2], hitbox[1]};
        transfdata[3] = (vec2f_t) {hitbox[0], hitbox[1]};
    
        SDL_UnmapGPUTransferBuffer(con->dev, transfbuf);

        SDL_GPUCopyPass* copypass = SDL_BeginGPUCopyPass(cmdbuf);

        SDL_UploadToGPUBuffer(
            copypass,
            &(SDL_GPUTransferBufferLocation) {
                .transfer_buffer = transfbuf,
                .offset = 0
            },
            &(SDL_GPUBufferRegion) {
                .buffer = vtxbuf,
                .offset = 0,
                .size = sizeof(vec2f_t) * 4
            },
            false
        );
    
        SDL_EndGPUCopyPass(copypass);

        SDL_GPURenderPass* renderpass = SDL_BeginGPURenderPass(
            cmdbuf,
            &(SDL_GPUColorTargetInfo) {
                .texture = swapchain,
                .cycle = false,
                .load_op = SDL_GPU_LOADOP_LOAD,
                .store_op = SDL_GPU_STOREOP_STORE,
            },
            1,
            NULL
        );

        SDL_BindGPUGraphicsPipeline(renderpass, boxpipeline);
        SDL_BindGPUVertexBuffers(renderpass, 0, &(SDL_GPUBufferBinding) {.buffer = vtxbuf, .offset = 0}, 1);
        SDL_BindGPUIndexBuffer(renderpass, &(SDL_GPUBufferBinding) {.buffer = idxbuf, .offset = 0}, SDL_GPU_INDEXELEMENTSIZE_16BIT);
    
        SDL_DrawGPUIndexedPrimitives(renderpass, 6, 1, 0, 0, 0);

        SDL_EndGPURenderPass(renderpass);
    }

    SDL_SubmitGPUCommandBuffer(cmdbuf);
}

void renderCleanupWindow(context_t* restrict con)
{
    SDL_ReleaseWindowFromGPUDevice(con->dev, con->window);
    SDL_DestroyWindow(con->window);
    SDL_DestroyGPUDevice(con->dev);
}

void renderCleanupPipeline(context_t* restrict con)
{
    SDL_ReleaseGPUGraphicsPipeline(con->dev, pipeline);
    SDL_ReleaseGPUSampler(con->dev, sampler);
    SDL_ReleaseGPUTexture(con->dev, texture);
    SDL_ReleaseGPUTransferBuffer(con->dev, fragtransferbuf);
    SDL_ReleaseGPUBuffer(con->dev, fragdatabuf);

    if (renderBufLock)
        SDL_DestroyMutex(renderBufLock);
}

SDL_GPUShader* renderLoadShader(context_t* restrict con, const char* restrict filename, u32 samplerCnt, u32 uniBufCnt, u32 stoBufCnt, u32 stoTexCnt)
{
    SDL_GPUShaderStage stage;

    if (SDL_strstr(filename, ".vert"))
        stage = SDL_GPU_SHADERSTAGE_VERTEX;
    else if (SDL_strstr(filename, ".frag"))
        stage = SDL_GPU_SHADERSTAGE_FRAGMENT;
    else
        return NULL;

    char path[256];
    const char* entry;

    SDL_GPUShaderFormat backendFmts = SDL_GetGPUShaderFormats(con->dev);
    SDL_GPUShaderFormat fmt = SDL_GPU_SHADERFORMAT_INVALID;

    if (backendFmts & SDL_GPU_SHADERFORMAT_SPIRV) {
        snprintf(path, sizeof(path), "%s..\\resources\\shaders\\comp\\SPV\\%s.spv", con->path, filename);
        fmt = SDL_GPU_SHADERFORMAT_SPIRV;
        entry = "main";
    } else if (backendFmts & SDL_GPU_SHADERFORMAT_MSL) {
        snprintf(path, sizeof(path), "%s..\\resources\\shaders\\comp\\MSL\\%s.msl", con->path, filename);
        fmt = SDL_GPU_SHADERFORMAT_MSL;
        entry = "main0";
    } else if (backendFmts & SDL_GPU_SHADERFORMAT_DXIL) {
        snprintf(path, sizeof(path), "%s..\\resources\\shaders\\comp\\DXIL\\%s.dxil", con->path, filename);
        fmt = SDL_GPU_SHADERFORMAT_SPIRV;
        entry = "main";
    } else {
        SDL_Log("Failed to recognize shader format");
        return NULL;
    }

    size_t len;
    void* code = SDL_LoadFile(path, &len);

    if (!code) {
        SDL_Log("Failed to load shader: %s", path);
        return NULL;
    }

    SDL_GPUShaderCreateInfo info = {
        .code = code,
        .code_size = len,
        .entrypoint = entry,
        .format = fmt,
        .stage = stage,
        .num_samplers = samplerCnt,
        .num_uniform_buffers = uniBufCnt,
        .num_storage_buffers = stoBufCnt,
        .num_storage_textures = stoTexCnt
    };

    SDL_GPUShader* shader = SDL_CreateGPUShader(con->dev, &info);

    if (!shader)
        SDL_Log("Failed to create shader");

    SDL_free(code);

    return shader;
}

SDL_Surface* renderLoadBmp(context_t* restrict con, const char* restrict filename)
{
    char path[256];
    SDL_Surface* surf;

    snprintf(path, sizeof(path), "%s..\\resources\\img\\%s", con->path, filename);

    surf = SDL_LoadBMP(path);

    if (!surf) {
        SDL_Log("Failed to load BMP: %s", SDL_GetError());
        return NULL;
    }

    return surf;
}

renderer_t r_ascii2D = {RENDERTYPE_2D, renderInitPipeline, renderDraw, renderCleanupPipeline};

//
//
//

void handleCollision(void)
{
    rAssert(bird);
    rAssert(fruit[0]);

    for (gameobj_t* i = fruit[0]; i < fruit[0] + 4; i++) {
        //SDL_Log("x %.4f y %.4f", bird->x, bird->y - (1.0f / 8.0f));

        if (bird->x + (1.0f / 12.0f) < i->x || bird->x > i->x + (1.0f / 14.0f))
            continue;

        if (bird->y < i->y || bird->y > i->y + (1.0f / 10.0f))
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
            if (!objbuf[i].data) {
                obj = objbuf + i;
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
        obj = objbuf + objidx++;
    }

    rAssert(obj);

    obj->data = charbuf + numchars;
    obj->len = len;
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

    if (obj < objbuf || obj >= objbuf + ASCII2D_OBJ_BUF_SIZE) {
        SDL_Log("ERROR: could not remove game object, pointer outside of object bounds");
        return;
    }

    rAssert(obj->len <= numchars);
    rAssert(obj->data + obj->len <= charbound && obj->data >= charbuf);

    if (obj->data + obj->len == charbound) {
        objidx = obj + 1 - objbuf;
        objfragmented = 0;
    }

    if (!objfragmented)
    {
        for (gameobj_t* i = objbuf; i < objbuf + objidx; i++) {
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

    memset(objbuf, 0, sizeof(gameobj_t) * ASCII2D_OBJ_BUF_SIZE);
}

//
//
//

mat4_t multiplyMat4(mat4_t m1, mat4_t m2)
{
    mat4_t res;

    res.m11 = (m1.m11 * m2.m11) + (m1.m12 * m2.m21) + (m1.m13 * m2.m31) + (m1.m14 * m2.m41);
    res.m12 = (m1.m11 * m2.m12) + (m1.m12 * m2.m22) + (m1.m13 * m2.m32) + (m1.m14 * m2.m42);
    res.m12 = (m1.m11 * m2.m13) + (m1.m12 * m2.m23) + (m1.m13 * m2.m33) + (m1.m14 * m2.m43);
    res.m12 = (m1.m11 * m2.m14) + (m1.m12 * m2.m24) + (m1.m13 * m2.m34) + (m1.m14 * m2.m44);

    res.m21 = (m1.m21 * m2.m11) + (m1.m22 * m2.m21) + (m1.m23 * m2.m31) + (m1.m24 * m2.m41);
    res.m22 = (m1.m21 * m2.m12) + (m1.m22 * m2.m22) + (m1.m23 * m2.m32) + (m1.m24 * m2.m42);
    res.m22 = (m1.m21 * m2.m13) + (m1.m22 * m2.m23) + (m1.m23 * m2.m33) + (m1.m24 * m2.m43);
    res.m22 = (m1.m21 * m2.m14) + (m1.m22 * m2.m24) + (m1.m23 * m2.m34) + (m1.m24 * m2.m44);

    res.m31 = (m1.m31 * m2.m11) + (m1.m32 * m2.m21) + (m1.m33 * m2.m31) + (m1.m34 * m2.m41);
    res.m32 = (m1.m31 * m2.m12) + (m1.m32 * m2.m22) + (m1.m33 * m2.m32) + (m1.m34 * m2.m42);
    res.m32 = (m1.m31 * m2.m13) + (m1.m32 * m2.m23) + (m1.m33 * m2.m33) + (m1.m34 * m2.m43);
    res.m32 = (m1.m31 * m2.m14) + (m1.m32 * m2.m24) + (m1.m33 * m2.m34) + (m1.m34 * m2.m44);

    res.m41 = (m1.m41 * m2.m11) + (m1.m42 * m2.m21) + (m1.m43 * m2.m31) + (m1.m44 * m2.m41);
    res.m42 = (m1.m41 * m2.m12) + (m1.m42 * m2.m22) + (m1.m43 * m2.m32) + (m1.m44 * m2.m42);
    res.m42 = (m1.m41 * m2.m13) + (m1.m42 * m2.m23) + (m1.m43 * m2.m33) + (m1.m44 * m2.m43);
    res.m42 = (m1.m41 * m2.m14) + (m1.m42 * m2.m24) + (m1.m43 * m2.m34) + (m1.m44 * m2.m44);

    return res;
}

mat4_t rotationMatZ(f32 rad)
{
    return (mat4_t) {
         SDL_cosf(rad), SDL_sinf(rad), 0, 0,
        -SDL_sinf(rad), SDL_cosf(rad), 0, 0,
         0, 0, 1, 0,
         0, 0, 0, 1
    };
}

mat4_t translationMat(f32 dx, f32 dy, f32 dz)
{
    return (mat4_t) {
         1,  0,  0,  0,
         0,  1,  0,  0,
         0,  0,  1,  0,
        dx, dy, dz,  1
    };
}

vec3f_t normalizeVec3(vec3f_t v)
{
    f32 m = SDL_sqrtf((v.x * v.x) + (v.y * v.y) * (v.z * v.z));
    return (vec3f_t) {v.x / m, v.y / m, v.z / m};
}

vec3f_t crossVec3(vec3f_t a, vec3f_t b)
{
    return (vec3f_t) {
         (a.y * b.z) - (b.y * a.z),
        -((a.x * b.z) - (b.x * a.z)),
         (a.x * b.y) - (b.x * a.y)
    };
}

f32 dotVec3(vec3f_t a, vec3f_t b)
{
    return (a.x * b.x) + (a.y * b.y) + (a.z * b.z);
}