#include <SDL3/SDL.h>

#include <com.h>
#include <render.h>

static SDL_GPUGraphicsPipeline* pipeline;
static SDL_GPUBuffer* vertexbuf;

void vertexInit(context_t* con)
{
    SDL_GPUShader* vertexShader = comLoadShader(con->dev, "PositionColor.vert", 0, 0, 0, 0);

    if (!vertexShader) {
        SDL_Log("Failed to load vertex shader");
        return;
    }

    SDL_GPUShader* fragShader = comLoadShader(con->dev, "solid.frag", 0, 0, 0, 0);

    if (!fragShader) {
        SDL_Log("Failed to load fragment shader");
        return;
    }

    SDL_GPUGraphicsPipelineCreateInfo pipelineInfo = {
        .target_info = {
            .num_color_targets = 1,
            .color_target_descriptions = (SDL_GPUColorTargetDescription[]) {{
                .format = SDL_GetGPUSwapchainTextureFormat(con->dev, con->window),
            }},
        },
        // set up to match vertex shader layout
        .vertex_input_state = (SDL_GPUVertexInputState) {
            .num_vertex_buffers = 1,
            .vertex_buffer_descriptions = (SDL_GPUVertexBufferDescription[]) {{
                .slot = 0,
                .input_rate = SDL_GPU_VERTEXINPUTRATE_VERTEX,
                .instance_step_rate = 0,
                .pitch = sizeof(vertPosCol_t),
            }},
            .num_vertex_attributes = 2,
            .vertex_attributes = (SDL_GPUVertexAttribute[]) {{
                .buffer_slot = 0,
                .format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3,
                .location = 0,
                .offset = 0
            }, {
                .buffer_slot = 0,
                .format = SDL_GPU_VERTEXELEMENTFORMAT_UBYTE4_NORM,
                .location = 1,
                .offset = sizeof(f32) * 3
            }}
        },
        .primitive_type = SDL_GPU_PRIMITIVETYPE_TRIANGLELIST,
        .vertex_shader = vertexShader,
        .fragment_shader = fragShader,
    };

    pipeline = SDL_CreateGPUGraphicsPipeline(con->dev, &pipelineInfo);

    if (!pipeline) {
        SDL_Log("Failed to create render pipeline");
        return;
    }

    SDL_ReleaseGPUShader(con->dev, vertexShader);
    SDL_ReleaseGPUShader(con->dev, fragShader);

    vertexbuf = SDL_CreateGPUBuffer(
        con->dev,
        &(SDL_GPUBufferCreateInfo) {
            .usage = SDL_GPU_BUFFERUSAGE_VERTEX,
            .size = sizeof(vertPosCol_t) * 6
        }
    );

    SDL_GPUTransferBuffer* transfbuf = SDL_CreateGPUTransferBuffer(
        con->dev,
        &(SDL_GPUTransferBufferCreateInfo) {
            .usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD,
            .size = sizeof(vertPosCol_t) * 6
        }
    );

    vertPosCol_t* data = SDL_MapGPUTransferBuffer(con->dev, transfbuf, 0);

    data[0] = (vertPosCol_t) {-0.3f,     0, 0,   0,   0, 255, 255};
    data[1] = (vertPosCol_t) {-0.3f, -0.3f, 0,   0,   0, 255, 255};
    data[2] = (vertPosCol_t) { 0.3f, -0.3f, 0,   0,   0, 255, 255};

    data[3] = (vertPosCol_t) {-0.3f,     0, 0,   0,   0, 255, 255};
    data[4] = (vertPosCol_t) { 0.3f,     0, 0,   0,   0, 255, 255};
    data[5] = (vertPosCol_t) { 0.3f, -0.3f, 0,   0,   0, 255, 255};

    SDL_UnmapGPUTransferBuffer(con->dev, transfbuf);

    SDL_GPUCommandBuffer* uploadcmdbuf = SDL_AcquireGPUCommandBuffer(con->dev);
    SDL_GPUCopyPass* copypass = SDL_BeginGPUCopyPass(uploadcmdbuf);

    SDL_UploadToGPUBuffer(
        copypass,
        &(SDL_GPUTransferBufferLocation) {
            .transfer_buffer = transfbuf,
            .offset = 0
        },
        &(SDL_GPUBufferRegion) {
            .buffer = vertexbuf,
            .offset = 0,
            .size = sizeof(vertPosCol_t) * 6
        },
        0
    );

    SDL_EndGPUCopyPass(copypass);
    SDL_SubmitGPUCommandBuffer(uploadcmdbuf);
    SDL_ReleaseGPUTransferBuffer(con->dev, transfbuf);
}

void vertexDraw(context_t* con)
{
    SDL_GPUCommandBuffer* cmdbuf = SDL_AcquireGPUCommandBuffer(con->dev);

    if (!cmdbuf) {
        SDL_Log("Failed to aquire command buffer: %s", SDL_GetError());
        return;
    }

    SDL_GPUTexture* swapchaintex;

    if (!SDL_WaitAndAcquireGPUSwapchainTexture(cmdbuf, con->window, &swapchaintex, NULL, NULL)) {
        SDL_Log("Failed to aquire swapchain texture: %s", SDL_GetError());
        return;
    }

    if (swapchaintex) {
        SDL_GPUColorTargetInfo targetinfo = {0};

        targetinfo.texture = swapchaintex;
        targetinfo.clear_color = (SDL_FColor) {0.0f, 0.0f, 0.0f, 1.0f};
        targetinfo.load_op = SDL_GPU_LOADOP_CLEAR;
        targetinfo.store_op = SDL_GPU_STOREOP_STORE;

        SDL_GPURenderPass* renderpass = SDL_BeginGPURenderPass(cmdbuf, &targetinfo, 1, NULL);

        SDL_BindGPUGraphicsPipeline(renderpass, pipeline);
        SDL_BindGPUVertexBuffers(renderpass, 0, &(SDL_GPUBufferBinding) {.buffer = vertexbuf, .offset = 0}, 1);
        SDL_DrawGPUPrimitives(renderpass, 6, 1, 0, 0);

        SDL_EndGPURenderPass(renderpass);
    }

    SDL_SubmitGPUCommandBuffer(cmdbuf);
}

void vertexCleanup(context_t* con)
{
    SDL_ReleaseGPUGraphicsPipeline(con->dev, pipeline);
    SDL_ReleaseGPUBuffer(con->dev, vertexbuf);
}