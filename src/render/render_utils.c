#include <stdio.h>
#include <SDL3/SDL.h>

#include <render/render.h>
#include <debug/rdebug.h>

SDL_GPUShader* renderLoadShader(context_t* restrict con, const char* filename, u32 samplerCnt, u32 uniBufCnt, u32 stoBufCnt, u32 stoTexCnt)
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
        SDL_Log("ERROR: Failed to recognize shader format");
        return NULL;
    }

    size_t len;
    void* code = SDL_LoadFile(path, &len);

    if (!code) {
        SDL_Log("[ERROR] Failed to load shader: %s", path);
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
        SDL_Log("[ERROR] Failed to create shader");

    SDL_free(code);

    return shader;
}

SDL_Surface* renderLoadBmp(context_t* restrict con, const char* filename)
{
    char path[256];
    SDL_Surface* surf;

    snprintf(path, sizeof(path), "%s..\\resources\\img\\%s", con->path, filename);

    surf = SDL_LoadBMP(path);

    if (!surf) {
        SDL_Log("[ERROR] Failed to load BMP: %s", SDL_GetError());
        return NULL;
    }

    return surf;
}

SDL_GPUTextureFormat renderInitWindow(context_t* restrict con, SDL_WindowFlags flags)
{
    con->dev = SDL_CreateGPUDevice(SDL_GPU_SHADERFORMAT_SPIRV | SDL_GPU_SHADERFORMAT_DXIL | SDL_GPU_SHADERFORMAT_MSL, 0, NULL);

    if (!con->dev) {
        SDL_Log("ERROR: Failed to create GPU device");
        return SDL_GPU_TEXTUREFORMAT_INVALID;
    }

    con->window = SDL_CreateWindow(con->name, con->width, con->height, flags);

    if (!con->window) {
        SDL_Log("[ERROR] Failed to create window");
        return SDL_GPU_TEXTUREFORMAT_INVALID;
    }

    if (!SDL_ClaimWindowForGPUDevice(con->dev, con->window)) {
        SDL_Log("[ERROR] Failed to claim window for GPU device");
        return SDL_GPU_TEXTUREFORMAT_INVALID;
    }

    renderBufLock = SDL_CreateMutex();

    if (!renderBufLock) {
        SDL_Log("[ERROR] Failed to create render buf lock");
        return SDL_GPU_TEXTUREFORMAT_INVALID;
    }

    con->path = SDL_GetBasePath();

    SDL_GPUPresentMode mode = SDL_GPU_PRESENTMODE_VSYNC;

    if (SDL_WindowSupportsGPUPresentMode(con->dev, con->window, SDL_GPU_PRESENTMODE_IMMEDIATE))
        mode = SDL_GPU_PRESENTMODE_IMMEDIATE;
    else if (SDL_WindowSupportsGPUPresentMode(con->dev, con->window, SDL_GPU_PRESENTMODE_MAILBOX))
        mode = SDL_GPU_PRESENTMODE_MAILBOX;

    SDL_SetGPUSwapchainParameters(con->dev, con->window, SDL_GPU_SWAPCHAINCOMPOSITION_SDR, mode);

    return SDL_GetGPUSwapchainTextureFormat(con->dev, con->window);
}

SDL_GPUGraphicsPipeline* renderInit2DPipeline(SDL_GPUDevice* dev, SDL_GPUShader* restrict vert, SDL_GPUShader* restrict frag, SDL_GPUTextureFormat fmt)
{
    if (!dev || !vert || !frag) {
        SDL_Log("[ERROR] Init 2D pipeline null ref");
        return NULL;
    }

    return SDL_CreateGPUGraphicsPipeline(
        dev,
        &(SDL_GPUGraphicsPipelineCreateInfo) {
            .target_info = (SDL_GPUGraphicsPipelineTargetInfo) {
                .num_color_targets = 1,
                .color_target_descriptions = (SDL_GPUColorTargetDescription[]) {{
                    .format = fmt,
                /*  .blend_state = {
                        .enable_blend = true,
                        .color_blend_op = SDL_GPU_BLENDOP_ADD,
                        .alpha_blend_op = SDL_GPU_BLENDOP_ADD,
                        .src_color_blendfactor = SDL_GPU_BLENDFACTOR_SRC_ALPHA,
                        .dst_color_blendfactor = SDL_GPU_BLENDFACTOR_ONE_MINUS_SRC_ALPHA,
                        .src_alpha_blendfactor = SDL_GPU_BLENDFACTOR_SRC_ALPHA,
                        .dst_alpha_blendfactor = SDL_GPU_BLENDFACTOR_ONE_MINUS_SRC_ALPHA
                    }   */
                }}
            },
            .primitive_type = SDL_GPU_PRIMITIVETYPE_TRIANGLELIST,
            .vertex_shader = vert,
            .fragment_shader = frag
        }
    );
}

SDL_GPUGraphicsPipeline* renderInitBoxPipeline(SDL_GPUDevice* dev, SDL_GPUShader* restrict vert, SDL_GPUShader* restrict frag, SDL_GPUTextureFormat fmt)
{
    if (!dev || !vert || !frag) {
        SDL_Log("[ERROR] Init box pipeline null ref");
        return NULL;
    }

    return SDL_CreateGPUGraphicsPipeline(
        dev,
        &(SDL_GPUGraphicsPipelineCreateInfo) {
            .target_info = {
                .num_color_targets = 1,
                .color_target_descriptions = (SDL_GPUColorTargetDescription[]) {{
                    .format = fmt
                }}
            },
            .vertex_input_state = (SDL_GPUVertexInputState) {
                .num_vertex_buffers = 1,
                .vertex_buffer_descriptions = (SDL_GPUVertexBufferDescription[]) {{
                    .slot = 0,
                    .input_rate = SDL_GPU_VERTEXINPUTRATE_VERTEX,
                    .instance_step_rate = 0,
                    .pitch = sizeof(vec2f_t)
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
            .vertex_shader = vert,
            .fragment_shader = frag
        }
    );
}

SDL_GPUGraphicsPipeline* renderInit3DPipeline(context_t* restrict con, SDL_GPUShader* restrict vert, SDL_GPUShader* restrict frag, SDL_GPUTexture** depthtex, SDL_GPUTextureFormat fmt)
{
    if (!con->dev || !vert || !frag || !depthtex) {
        SDL_Log("[ERROR] Init 3D pipeline null ref");
        return NULL;
    }

    *depthtex = SDL_CreateGPUTexture(
        con->dev,
        &(SDL_GPUTextureCreateInfo) {
            .format = SDL_GPU_TEXTUREFORMAT_D16_UNORM,
            .usage = SDL_GPU_TEXTUREUSAGE_DEPTH_STENCIL_TARGET,
            .width = con->width,
            .height = con->height,
            .layer_count_or_depth = 1,
            .num_levels = 1
        }
    );

    return SDL_CreateGPUGraphicsPipeline(
        con->dev,
        &(SDL_GPUGraphicsPipelineCreateInfo) {
            .target_info = {
                .num_color_targets = 1,
                .color_target_descriptions = (SDL_GPUColorTargetDescription[]) {{
                    .format = fmt
                }},
                .has_depth_stencil_target = true,
                .depth_stencil_format = SDL_GPU_TEXTUREFORMAT_D16_UNORM
            },
            .depth_stencil_state = (SDL_GPUDepthStencilState) {
                .enable_depth_test = true,
                .enable_depth_write = true,
                .compare_op = SDL_GPU_COMPAREOP_LESS
            },
            .vertex_input_state = (SDL_GPUVertexInputState) {
                .num_vertex_buffers = 1,
                .vertex_buffer_descriptions = (SDL_GPUVertexBufferDescription[]) {{
                    .slot = 0,
                    .instance_step_rate = 0,
                    .input_rate = SDL_GPU_VERTEXINPUTRATE_VERTEX,
                    .pitch = sizeof(vec3f_t)
                }},
                .num_vertex_attributes = 2,
                .vertex_attributes = (SDL_GPUVertexAttribute[]) {{
                    .buffer_slot = 0,
                    .location = 0,
                    .offset = 0,
                    .format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3
                }, {
                    .buffer_slot = 0,
                    .location = 1,
                    .offset = sizeof(f32) * 3,
                    .format = SDL_GPU_VERTEXELEMENTFORMAT_UINT
                }}
            },
            .rasterizer_state = (SDL_GPURasterizerState) {
                #ifdef DEBUG_WIREFRAMES
                    .fill_mode = SDL_GPU_FILLMODE_LINE,
                #else
                    .fill_mode = SDL_GPU_FILLMODE_FILL,
                #endif
                .cull_mode = SDL_GPU_CULLMODE_BACK,
            },
            .primitive_type = SDL_GPU_PRIMITIVETYPE_TRIANGLELIST,
            .vertex_shader = vert,
            .fragment_shader = frag
        }
    );
}

SDL_GPUGraphicsPipeline* renderInitOdysseyPipeline(SDL_GPUDevice* dev, SDL_GPUShader* restrict vert, SDL_GPUShader* restrict frag, SDL_GPUTextureFormat fmt)
{
    if (!dev || !vert || !frag) {
        SDL_Log("[ERROR] Init odyssey pipeline null ref");
        return NULL;
    }

    return SDL_CreateGPUGraphicsPipeline(
        dev,
        &(SDL_GPUGraphicsPipelineCreateInfo) {
            .target_info = {
                .num_color_targets = 1,
                .color_target_descriptions = (SDL_GPUColorTargetDescription[]) {{
                    .format = fmt
                }},
                //.has_depth_stencil_target = true,
                //.depth_stencil_format = SDL_GPU_TEXTUREFORMAT_D16_UNORM
            },
            /*.depth_stencil_state = (SDL_GPUDepthStencilState) {
                .enable_depth_test = true,
                .enable_depth_write = true,
                .compare_op = SDL_GPU_COMPAREOP_LESS
            },*/
            .vertex_input_state = (SDL_GPUVertexInputState) {
                .num_vertex_buffers = 1,
                .vertex_buffer_descriptions = (SDL_GPUVertexBufferDescription[]) {{
                    .slot = 0,
                    .instance_step_rate = 0,
                    .input_rate = SDL_GPU_VERTEXINPUTRATE_VERTEX,
                    .pitch = sizeof(f32) * 5
                }},
                .num_vertex_attributes = 2,
                .vertex_attributes = (SDL_GPUVertexAttribute[]) {{
                    .buffer_slot = 0,
                    .location = 0,
                    .offset = 0,
                    .format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3
                }, {
                    .buffer_slot = 0,
                    .location = 1,
                    .offset = sizeof(f32) * 3,
                    .format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT2
                }}
            },
            .rasterizer_state.fill_mode = SDL_GPU_FILLMODE_FILL,
            .primitive_type = SDL_GPU_PRIMITIVETYPE_TRIANGLELIST,
            .vertex_shader = vert,
            .fragment_shader = frag
        }
    );
}

SDL_GPUGraphicsPipeline* renderInit3DDebugPipeline(context_t* restrict con, SDL_GPUShader* restrict vert, SDL_GPUShader* restrict frag, SDL_GPUTextureFormat fmt)
{
    if (!con->dev || !vert || !frag) {
        SDL_Log("[ERROR] Init 3D pipeline null ref");
        return NULL;
    }

    return SDL_CreateGPUGraphicsPipeline(
        con->dev,
        &(SDL_GPUGraphicsPipelineCreateInfo) {
            .target_info = {
                .num_color_targets = 1,
                .color_target_descriptions = (SDL_GPUColorTargetDescription[]) {{
                    .format = fmt
                }}
            },
            .rasterizer_state.fill_mode = SDL_GPU_FILLMODE_FILL,
            .primitive_type = SDL_GPU_PRIMITIVETYPE_LINELIST,
            .vertex_shader = vert,
            .fragment_shader = frag
        }
    );
}

SDL_GPUTexture* renderInitAsciiTexture(context_t* restrict con, const char* filename, SDL_GPUSampler** sampler)
{
    rAssert(con);
    rAssert(con->dev);
    rAssert(filename);
    rWarning(!(*sampler));

    SDL_Surface* img = renderLoadBmp(con, filename);

    if (!img) {
        SDL_Log("[ERROR] Failed to load image");
        return NULL;
    }

    SDL_GPUTransferBuffer* transfbuf = SDL_CreateGPUTransferBuffer(
        con->dev,
        &(SDL_GPUTransferBufferCreateInfo) {
            .usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD,
            .size = img->w * img->h * 4
        }
    );

    u8* transfmem = SDL_MapGPUTransferBuffer(con->dev, transfbuf, false);

    memcpy(transfmem, img->pixels, img->w * img->h * 4);

    SDL_UnmapGPUTransferBuffer(con->dev, transfbuf);

    SDL_GPUTexture* tex = SDL_CreateGPUTexture(
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

    if (!tex) {
        SDL_Log("[ERROR] Failed to create GPU texture");
        return NULL;
    }

    SDL_GPUSampler* smp = SDL_CreateGPUSampler(
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

    if (!smp) {
        SDL_Log("[ERROR] Failed to create GPU sampler");
        return NULL;
    }

    SDL_GPUCommandBuffer* cmdbuf = SDL_AcquireGPUCommandBuffer(con->dev);
    SDL_GPUCopyPass* copypass = SDL_BeginGPUCopyPass(cmdbuf);

    SDL_UploadToGPUTexture(
        copypass,
        &(SDL_GPUTextureTransferInfo) {
            .transfer_buffer = transfbuf,
            .offset = 0
        },
        &(SDL_GPUTextureRegion) {
            .texture = tex,
            .w = img->w,
            .h = img->h,
            .d = 1
        },
        false
    );

    SDL_EndGPUCopyPass(copypass);
    SDL_SubmitGPUCommandBuffer(cmdbuf);

    SDL_DestroySurface(img);
    SDL_ReleaseGPUTransferBuffer(con->dev, transfbuf);

    *sampler = smp;

    return tex;
}

SDL_GPUTexture* renderInitOdysseyTexture(context_t* restrict con, SDL_GPUSampler** sampler, SDL_GPUTextureFormat rfmt, u32 w, u32 h)
{
    rAssert(con);
    rAssert(con->dev);
    rWarning(!(*sampler));

    SDL_GPUTransferBuffer* transfbuf = SDL_CreateGPUTransferBuffer(
        con->dev,
        &(SDL_GPUTransferBufferCreateInfo) {
            .usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD,
            .size = h * w * 4
        }
    );

    void* transfmem = SDL_MapGPUTransferBuffer(con->dev, transfbuf, false);

    rAssert(transfmem);

    memset(transfmem, 0, h * w * 4);

    SDL_UnmapGPUTransferBuffer(con->dev, transfbuf);

    SDL_GPUTexture* tex = SDL_CreateGPUTexture(
        con->dev,
        &(SDL_GPUTextureCreateInfo) {
            .type = SDL_GPU_TEXTURETYPE_2D,
            .format = rfmt,
            .width = w,
            .height = h,
            .layer_count_or_depth = 1,
            .num_levels = 1,
            .usage = SDL_GPU_TEXTUREUSAGE_SAMPLER | SDL_GPU_TEXTUREUSAGE_COLOR_TARGET
        }
    );

    if (!tex) {
        SDL_Log("[ERROR] Failed to create GPU texture");
        return NULL;
    }

    SDL_GPUSampler* smp = SDL_CreateGPUSampler(
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

    if (!smp) {
        SDL_Log("[ERROR] Failed to create GPU sampler");
        return NULL;
    }

    SDL_GPUCommandBuffer* cmdbuf = SDL_AcquireGPUCommandBuffer(con->dev);
    SDL_GPUCopyPass* copypass = SDL_BeginGPUCopyPass(cmdbuf);

    SDL_UploadToGPUTexture(
        copypass,
        &(SDL_GPUTextureTransferInfo) {
            .transfer_buffer = transfbuf,
            .offset = 0
        },
        &(SDL_GPUTextureRegion) {
            .texture = tex,
            .w = w,
            .h = h,
            .d = 1
        },
        false
    );

    SDL_EndGPUCopyPass(copypass);
    
    SDL_SubmitGPUCommandBuffer(cmdbuf);

    SDL_ReleaseGPUTransferBuffer(con->dev, transfbuf);

    *sampler = smp;

    return tex;
}

void renderCleanupWindow(context_t* restrict con)
{
    SDL_ReleaseWindowFromGPUDevice(con->dev, con->window);
    SDL_DestroyWindow(con->window);
    SDL_DestroyGPUDevice(con->dev);
}