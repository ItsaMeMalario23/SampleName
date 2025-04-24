#include <SDL3/SDL.h>

#include <render/rendercom.h>

static SDL_GPUGraphicsPipeline* pipeline;
static SDL_GPUBuffer* vertexbuf;
static SDL_GPUBuffer* indexbuf;
static SDL_GPUTexture* texture;
static SDL_GPUSampler* sampler;

// buffers for frag shader
static SDL_GPUTransferBuffer* fragtransferbuf;
static SDL_GPUBuffer* fragdatabuf;

typedef struct asciidata_s {
    f32 x, y;
} asciidata_t;

asciidata_t dat[16] = {0};

static void init(context_t* con)
{
    dat[0] = (asciidata_t) {-1.0f, 0.0f};
    dat[1] = (asciidata_t) {-0.9f, 0.0f};
    dat[2] = (asciidata_t) {-0.8f, 0.0f};
    dat[3] = (asciidata_t) {-0.7f, 0.0f};
    dat[4] = (asciidata_t) {-0.6f, 0.0f};
    dat[5] = (asciidata_t) {-0.5f, 0.0f};
    dat[6] = (asciidata_t) {-0.4f, 0.0f};
    dat[7] = (asciidata_t) {-0.3f, 0.0f};

    SDL_GPUShader* vertshader = renderLoadShader(con->dev, "TexturedQuadInstanced.vert", 0, 0, 1, 0);
    SDL_GPUShader* fragshader = renderLoadShader(con->dev, "TexturedQuad.frag", 1, 0, 0, 0);

    if (!vertshader || !fragshader) {
        SDL_Log("Failed to create vertex shader");
        return;
    }

    SDL_Surface* imgdata = renderLoadBmp("test1.bmp");

    if (!imgdata) {
        SDL_Log("Failed to load image data");
        return;
    }

    // Create the pipeline
	SDL_GPUGraphicsPipelineCreateInfo pinfo = {
		.target_info = {
			.num_color_targets = 1,
			.color_target_descriptions = (SDL_GPUColorTargetDescription[]) {{
				.format = SDL_GetGPUSwapchainTextureFormat(con->dev, con->window)
			}},
		},
		.vertex_input_state = (SDL_GPUVertexInputState){
			.num_vertex_buffers = 1,
			.vertex_buffer_descriptions = (SDL_GPUVertexBufferDescription[]) {{
				.slot = 0,
				.input_rate = SDL_GPU_VERTEXINPUTRATE_VERTEX,
				.instance_step_rate = 0,
				.pitch = sizeof(texvertex_t)
			}},
			.num_vertex_attributes = 2,
			.vertex_attributes = (SDL_GPUVertexAttribute[]) {{
				.buffer_slot = 0,
				.format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3,
				.location = 0,
				.offset = 0
			}, {
				.buffer_slot = 0,
				.format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT2,
				.location = 1,
				.offset = sizeof(float) * 3
			}}
		},
		.primitive_type = SDL_GPU_PRIMITIVETYPE_TRIANGLELIST,
		.vertex_shader = vertshader,
		.fragment_shader = fragshader
	};

    pipeline = SDL_CreateGPUGraphicsPipeline(con->dev, &pinfo);

    if (!pipeline) {
        SDL_Log("Failed to create pipeline");
        return;
    }

    SDL_ReleaseGPUShader(con->dev, vertshader);
    SDL_ReleaseGPUShader(con->dev, fragshader);

	sampler = SDL_CreateGPUSampler(con->dev, &(SDL_GPUSamplerCreateInfo){
		.min_filter = SDL_GPU_FILTER_NEAREST,
		.mag_filter = SDL_GPU_FILTER_NEAREST,
		.mipmap_mode = SDL_GPU_SAMPLERMIPMAPMODE_NEAREST,
		.address_mode_u = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE,
		.address_mode_v = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE,
		.address_mode_w = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE,
	});

    vertexbuf = SDL_CreateGPUBuffer(con->dev, &(SDL_GPUBufferCreateInfo) {.usage = SDL_GPU_BUFFERUSAGE_VERTEX, .size = sizeof(texvertex_t) * 4});

    indexbuf = SDL_CreateGPUBuffer(con->dev, &(SDL_GPUBufferCreateInfo) {.usage = SDL_GPU_BUFFERUSAGE_INDEX, .size = sizeof(u16) * 6});

    texture = SDL_CreateGPUTexture(con->dev, &(SDL_GPUTextureCreateInfo) {
        .type = SDL_GPU_TEXTURETYPE_2D,
        .format = SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM,
        .width = imgdata->w,
        .height = imgdata->h,
        .layer_count_or_depth = 1,
        .num_levels = 1,
        .usage = SDL_GPU_TEXTUREUSAGE_SAMPLER
    });

    SDL_GPUTransferBuffer* transferbuf = SDL_CreateGPUTransferBuffer(con->dev, &(SDL_GPUTransferBufferCreateInfo) {.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD, .size = (sizeof(texvertex_t) * 4) + (sizeof(u16) * 6)});

    texvertex_t* transferdata = SDL_MapGPUTransferBuffer(con->dev, transferbuf, 0);

	transferdata[0] = (texvertex_t) { -1,  1, 0, 0, 0 };
	transferdata[1] = (texvertex_t) {  1,  1, 0, 1, 0 };
	transferdata[2] = (texvertex_t) {  1, -1, 0, 1, 1 };
	transferdata[3] = (texvertex_t) { -1, -1, 0, 0, 1 };

    u16* idxdata = (u16*) &transferdata[4];

    idxdata[0] = 0;
    idxdata[1] = 1;
    idxdata[2] = 2;
    idxdata[3] = 0;
    idxdata[4] = 2;
    idxdata[5] = 3;

    SDL_UnmapGPUTransferBuffer(con->dev, transferbuf);

    // texture
    SDL_GPUTransferBuffer* textransferbuf = SDL_CreateGPUTransferBuffer(con->dev, &(SDL_GPUTransferBufferCreateInfo) {.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD, .size = imgdata->w * imgdata->h * 4});

    u8* textransferptr = SDL_MapGPUTransferBuffer(con->dev, textransferbuf, 0);

    SDL_memcpy(textransferptr, imgdata->pixels, imgdata->w * imgdata->h * 4);

    SDL_UnmapGPUTransferBuffer(con->dev, textransferbuf);

    // data buffers
    fragtransferbuf = SDL_CreateGPUTransferBuffer(
        con->dev,
        &(SDL_GPUTransferBufferCreateInfo) {
            .usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD,
            .size = 16 * sizeof(asciidata_t)
    });

    fragdatabuf = SDL_CreateGPUBuffer(
        con->dev,
        &(SDL_GPUBufferCreateInfo) {
            .usage = SDL_GPU_BUFFERUSAGE_GRAPHICS_STORAGE_READ,
            .size = 16 * sizeof(asciidata_t)
        }
    );

    SDL_GPUCommandBuffer* cmdbuf = SDL_AcquireGPUCommandBuffer(con->dev);
    SDL_GPUCopyPass* copypass = SDL_BeginGPUCopyPass(cmdbuf);

    SDL_UploadToGPUBuffer(
        copypass,
        &(SDL_GPUTransferBufferLocation) {
            .transfer_buffer = transferbuf,
            .offset = 0
        },
        &(SDL_GPUBufferRegion) {
            .buffer = vertexbuf,
            .offset = 0,
            .size = sizeof(texvertex_t) * 4
        },
        0
    );

    SDL_UploadToGPUBuffer(
        copypass,
        &(SDL_GPUTransferBufferLocation) {
            .transfer_buffer = transferbuf,
            .offset = sizeof(texvertex_t) * 4
        },
        &(SDL_GPUBufferRegion) {
            .buffer = indexbuf,
            .offset = 0,
            .size = sizeof(u16) * 6
        },
        0
    );

    SDL_UploadToGPUTexture(
        copypass,
        &(SDL_GPUTextureTransferInfo) {
            .transfer_buffer = textransferbuf,
            .offset = 0
        },
        &(SDL_GPUTextureRegion) {
            .texture = texture,
            .w = imgdata->w,
            .h = imgdata->h,
            .d = 1
        },
        0
    );

    SDL_EndGPUCopyPass(copypass);
    SDL_SubmitGPUCommandBuffer(cmdbuf);

    SDL_DestroySurface(imgdata);
    SDL_ReleaseGPUTransferBuffer(con->dev, transferbuf);
    SDL_ReleaseGPUTransferBuffer(con->dev, textransferbuf);
}

static void draw(context_t* con)
{
    SDL_GPUCommandBuffer* cmdbuf = SDL_AcquireGPUCommandBuffer(con->dev);

    if (!cmdbuf) {
        SDL_Log("Failed to acquire command buffer: %s", SDL_GetError());
        return;
    }

    SDL_GPUTexture* swapchain;

    if (!SDL_WaitAndAcquireGPUSwapchainTexture(cmdbuf, con->window, &swapchain, NULL, NULL)) {
        SDL_Log("Failed to acquire swapchain texture: %s", SDL_GetError());
        return;
    }

    if (swapchain) {
        // buffer transfer
        asciidata_t* ptr = SDL_MapGPUTransferBuffer(con->dev, fragtransferbuf, 1);

        ptr[0].x = dat[0].x;
        ptr[0].y = dat[0].y;

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
                .size = 16 * sizeof(asciidata_t)
            },
            1
        );

        SDL_EndGPUCopyPass(copypass);

        SDL_GPUColorTargetInfo info = {0};

        info.texture = swapchain;
        info.cycle = 0;
        info.clear_color = (SDL_FColor) {0.0f, 0.0f, 0.0f, 1.0f};
        info.load_op = SDL_GPU_LOADOP_CLEAR;
        info.store_op = SDL_GPU_STOREOP_STORE;

        SDL_GPURenderPass* renderpass = SDL_BeginGPURenderPass(cmdbuf, &info, 1, NULL);

        SDL_BindGPUGraphicsPipeline(renderpass, pipeline);

        SDL_BindGPUVertexBuffers(renderpass, 0, &(SDL_GPUBufferBinding) {.buffer = vertexbuf, .offset = 0}, 1);
        SDL_BindGPUIndexBuffer(renderpass, &(SDL_GPUBufferBinding) {.buffer = indexbuf, .offset = 0}, SDL_GPU_INDEXELEMENTSIZE_16BIT);
        
        SDL_BindGPUVertexStorageBuffers(renderpass, 0, &fragdatabuf, 1);
        
        SDL_BindGPUFragmentSamplers(renderpass, 0, &(SDL_GPUTextureSamplerBinding) {.texture = texture, .sampler = sampler}, 1);

        SDL_DrawGPUIndexedPrimitives(renderpass, 6, 8, 0, 0, 0);

        SDL_EndGPURenderPass(renderpass);
    }

    SDL_SubmitGPUCommandBuffer(cmdbuf);
}

static void update(u32 mode)
{
    if (mode == 0)
        dat[0].y += 0.1f;
    else if (mode == 1)
        dat[0].y -= 0.1f;
}

static void cleanup(context_t* con)
{
    SDL_ReleaseGPUGraphicsPipeline(con->dev, pipeline);
    SDL_ReleaseGPUBuffer(con->dev, vertexbuf);
    SDL_ReleaseGPUBuffer(con->dev, indexbuf);
    SDL_ReleaseGPUTexture(con->dev, texture);
    SDL_ReleaseGPUSampler(con->dev, sampler);

    SDL_ReleaseGPUTransferBuffer(con->dev, fragtransferbuf);
    SDL_ReleaseGPUBuffer(con->dev, fragdatabuf);
}

renderer_t r_ascii2D = {RENDERTYPE_2D, init, update, draw, cleanup};

/*
#include <SDL3/SDL.h>

#include <render/rendercom.h>

static SDL_GPUGraphicsPipeline* pipeline;
static SDL_GPUBuffer* vertexbuf;
static SDL_GPUBuffer* indexbuf;
static SDL_GPUTexture* texture;
static SDL_GPUSampler* sampler;

static void init(context_t* con)
{
    SDL_GPUShader* vertshader = renderLoadShader(con->dev, "TexturedQuad.vert", 0, 0, 0, 0);

    if (!vertshader) {
        SDL_Log("Failed to create vertex shader");
        return;
    }

    SDL_GPUShader* fragshader = renderLoadShader(con->dev, "TexturedQuadArray.frag", 1, 0, 0, 0);

    if (!fragshader) {
        SDL_Log("Failed to create fragment shader");
        return;
    }

    SDL_GPUGraphicsPipelineCreateInfo pipelineinfo = {
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
                .pitch = sizeof(texvertex_t),
            }},
            .num_vertex_attributes = 2,
            .vertex_attributes = (SDL_GPUVertexAttribute[]) {{
                .buffer_slot = 0,
                .format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3,
                .location = 0,
                .offset = 0
            }, {
                .buffer_slot = 0,
                .format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT2,
                .location = 1,
                .offset = sizeof(f32) * 3
            }}
        },
        .primitive_type = SDL_GPU_PRIMITIVETYPE_TRIANGLELIST,
        .vertex_shader = vertshader,
        .fragment_shader = fragshader
    };

    pipeline = SDL_CreateGPUGraphicsPipeline(con->dev, &pipelineinfo);

    if (!pipeline) {
        SDL_Log("Failed to create render pipeline");
        return;
    }

    SDL_ReleaseGPUShader(con->dev, vertshader);
    SDL_ReleaseGPUShader(con->dev, fragshader);

    SDL_Surface* img1 = renderLoadBmp("test1.bmp");
    SDL_Surface* img2 = renderLoadBmp("test2.bmp");

    if (!img1 || !img2) {
        SDL_Log("Failed to load image");
        return;
    }

    SDL_assert(img1->w == img2->w);
    SDL_assert(img1->h == img2->h);

    vertexbuf = SDL_CreateGPUBuffer(con->dev, &(SDL_GPUBufferCreateInfo) {.usage = SDL_GPU_BUFFERUSAGE_VERTEX, .size = sizeof(texvertex_t) * 4});

    indexbuf = SDL_CreateGPUBuffer(con->dev, &(SDL_GPUBufferCreateInfo) {.usage = SDL_GPU_BUFFERUSAGE_INDEX, .size = sizeof(u16) * 6});

    texture = SDL_CreateGPUTexture
    (
        con->dev,
        &(SDL_GPUTextureCreateInfo) {
            .type = SDL_GPU_TEXTURETYPE_2D_ARRAY,
            .format = SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM,
            .width = img1->w,
            .height = img1->h,
            .layer_count_or_depth = 2,
            .num_levels = 1,
            .usage = SDL_GPU_TEXTUREUSAGE_SAMPLER
        }
    );

    sampler = SDL_CreateGPUSampler
    (
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

    SDL_GPUTransferBuffer* transferbuf = SDL_CreateGPUTransferBuffer(con->dev, &(SDL_GPUTransferBufferCreateInfo) {.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD, .size = (sizeof(texvertex_t) * 4) + (sizeof(u16) * 6)});

    texvertex_t* transferdata = (texvertex_t*) SDL_MapGPUTransferBuffer(con->dev, transferbuf, 0);

    transferdata[0] = (texvertex_t) {-1,  1,  0,  0,  0};
    transferdata[1] = (texvertex_t) { 1,  1,  0,  1,  0};
    transferdata[2] = (texvertex_t) { 1, -1,  0,  1,  1};
    transferdata[3] = (texvertex_t) {-1, -1,  0,  0,  1};

    u16* indexdata = (u16*) &transferdata[4];

    indexdata[0] = 0;
    indexdata[1] = 1;
    indexdata[2] = 2;
    indexdata[3] = 0;
    indexdata[4] = 2;
    indexdata[5] = 3;

    SDL_UnmapGPUTransferBuffer(con->dev, transferbuf);

    const u32 imgsize = img1->w * img1->h * 4;

    SDL_GPUTransferBuffer* textransferbuf = SDL_CreateGPUTransferBuffer(con->dev, &(SDL_GPUTransferBufferCreateInfo) {.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD, .size = imgsize * 2});

    u8* textransferptr = SDL_MapGPUTransferBuffer(con->dev, textransferbuf, 0);

    SDL_memcpy(textransferptr, img1->pixels, imgsize);
    SDL_memcpy(textransferptr + imgsize, img2->pixels, imgsize);

    SDL_UnmapGPUTransferBuffer(con->dev, textransferbuf);

    SDL_GPUCommandBuffer* cmdbuf = SDL_AcquireGPUCommandBuffer(con->dev);
    SDL_GPUCopyPass* copypass = SDL_BeginGPUCopyPass(cmdbuf);

    SDL_UploadToGPUBuffer
    (
        copypass,
        &(SDL_GPUTransferBufferLocation) {
            .transfer_buffer = transferbuf,
            .offset = 0
        },
        &(SDL_GPUBufferRegion) {
            .buffer = vertexbuf,
            .offset = 0,
            .size = sizeof(texvertex_t) * 4
        },
        0
    );

    SDL_UploadToGPUBuffer
    (
        copypass,
        &(SDL_GPUTransferBufferLocation) {
            .transfer_buffer = transferbuf,
            .offset = sizeof(texvertex_t) * 4
        },
        &(SDL_GPUBufferRegion) {
            .buffer = indexbuf,
            .offset = 0,
            .size = sizeof(u16) * 6
        },
        0
    );

    SDL_UploadToGPUTexture
    (
        copypass,
        &(SDL_GPUTextureTransferInfo) {
            .transfer_buffer = textransferbuf,
            .offset = 0
        },
        &(SDL_GPUTextureRegion) {
            .texture = texture,
            .w = img1->w,
            .h = img1->h,
            .d = 1
        },
        0
    );

    SDL_UploadToGPUTexture
    (
        copypass,
        &(SDL_GPUTextureTransferInfo) {
            .transfer_buffer = textransferbuf,
            .offset = imgsize
        },
        &(SDL_GPUTextureRegion) {
            .texture = texture,
            .layer = 1,
            .w = img1->w,
            .h = img1->h,
            .d = 1
        },
        0
    );

    SDL_DestroySurface(img1);
    SDL_DestroySurface(img2);

    SDL_EndGPUCopyPass(copypass);
    SDL_SubmitGPUCommandBuffer(cmdbuf);

    SDL_ReleaseGPUTransferBuffer(con->dev, transferbuf);
    SDL_ReleaseGPUTransferBuffer(con->dev, textransferbuf);
}

static void draw(context_t* con)
{
    SDL_GPUCommandBuffer* cmdbuf = SDL_AcquireGPUCommandBuffer(con->dev);

    if (!cmdbuf) {
        SDL_Log("Failed to acquire command buffer: %s", SDL_GetError());
        return;
    }

    SDL_GPUTexture* swapchaintex;

    if (!SDL_WaitAndAcquireGPUSwapchainTexture(cmdbuf, con->window, &swapchaintex, NULL, NULL)) {
        SDL_Log("Failed to acquire swapchain texture: %s", SDL_GetError());
        return;
    }

    if (swapchaintex) {
        SDL_GPUColorTargetInfo info = {0};

        info.texture = swapchaintex;
        info.clear_color = (SDL_FColor) {0.0f, 0.0f, 0.0f, 1.0f};
        info.load_op = SDL_GPU_LOADOP_CLEAR;
        info.store_op = SDL_GPU_STOREOP_STORE;

        SDL_GPURenderPass* renderpass = SDL_BeginGPURenderPass(cmdbuf, &info, 1, NULL);

        SDL_BindGPUGraphicsPipeline(renderpass, pipeline);
        SDL_BindGPUVertexBuffers(renderpass, 0, &(SDL_GPUBufferBinding) {.buffer = vertexbuf, .offset = 0}, 1);
        SDL_BindGPUIndexBuffer(renderpass, &(SDL_GPUBufferBinding) {.buffer = indexbuf, .offset = 0}, SDL_GPU_INDEXELEMENTSIZE_16BIT);
        SDL_BindGPUFragmentSamplers(renderpass, 0, &(SDL_GPUTextureSamplerBinding) {.texture = texture, .sampler = sampler}, 1);
        SDL_DrawGPUIndexedPrimitives(renderpass, 6, 1, 0, 0, 0);

        SDL_EndGPURenderPass(renderpass);
    }

    SDL_SubmitGPUCommandBuffer(cmdbuf);
}

static void update(u32 mode) { }

static void cleanup(context_t* con)
{
    SDL_ReleaseGPUGraphicsPipeline(con->dev, pipeline);
    SDL_ReleaseGPUBuffer(con->dev, vertexbuf);
    SDL_ReleaseGPUBuffer(con->dev, indexbuf);
    SDL_ReleaseGPUTexture(con->dev, texture);
    SDL_ReleaseGPUSampler(con->dev, sampler);
}

renderer_t r_ascii2D = {RENDERTYPE_2D, init, update, draw, cleanup};
*/