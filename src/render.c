#include <SDL3/SDL.h>

#include <com.h>
#include <render.h>

static SDL_GPUGraphicsPipeline* fillPipeline;
static SDL_GPUGraphicsPipeline* linePipeline;
static SDL_GPUViewport smallView = {160, 120, 320, 240, 0.1f, 1.0f};
static SDL_Rect scissorRect = {320, 240, 320, 240};

static bool wireframeMode = 0;
static bool smallViewportMode = 0;
static bool scissorRectMode = 0;

void init(context_t* context)
{
    SDL_GPUShader* vertexShader = comLoadShader(context->dev, "triangle.vert", 0, 0, 0, 0);

    if (!vertexShader) {
        SDL_Log("Failed to load vertex shader");
        return;
    }

    SDL_GPUShader* fragShader = comLoadShader(context->dev, "solid.frag", 0, 0, 0, 0);

    if (!fragShader) {
        SDL_Log("Failed to load fragment shader");
        return;
    }

    SDL_GPUGraphicsPipelineCreateInfo pipelineInfo = {
        .target_info = {
            .num_color_targets = 1,
            .color_target_descriptions = (SDL_GPUColorTargetDescription[]) {{
                .format = SDL_GetGPUSwapchainTextureFormat(context->dev, context->window)
            }},
        },
        .primitive_type = SDL_GPU_PRIMITIVETYPE_TRIANGLELIST,
        .vertex_shader = vertexShader,
        .fragment_shader = fragShader,
    };

    pipelineInfo.rasterizer_state.fill_mode = SDL_GPU_FILLMODE_FILL;

    fillPipeline = SDL_CreateGPUGraphicsPipeline(context->dev, &pipelineInfo);

    if (!fillPipeline) {
        SDL_Log("Failed to create fill pipeline");
        return;
    }

    pipelineInfo.rasterizer_state.fill_mode = SDL_GPU_FILLMODE_LINE;

    linePipeline = SDL_CreateGPUGraphicsPipeline(context->dev, &pipelineInfo);

    if (!linePipeline) {
        SDL_Log("Failed to create line pipeline");
        return;
    }

    SDL_ReleaseGPUShader(context->dev, vertexShader);
    SDL_ReleaseGPUShader(context->dev, fragShader);
}

void chmode(u32 mode)
{
    switch (mode) {
    case 0:
        wireframeMode = !wireframeMode;
        break;

    case 1:
        smallViewportMode = !smallViewportMode;
        break;

    case 2:
        scissorRectMode = !scissorRectMode;
        break;
    }
}

void draw(context_t* context)
{
    SDL_GPUCommandBuffer* cmdbuf = SDL_AcquireGPUCommandBuffer(context->dev);

    if (!cmdbuf) {
        SDL_Log("Failed to aquire command buf: %s", SDL_GetError());
        return;
    }

    SDL_GPUTexture* swapchainTex;

    if (!SDL_WaitAndAcquireGPUSwapchainTexture(cmdbuf, context->window, &swapchainTex, NULL, NULL)) {
        SDL_Log("Failed to aquire swapchain texture: %s", SDL_GetError());
        return;
    }

    // swapchain texture can be null
    if (swapchainTex) {
        SDL_GPUColorTargetInfo targetinfo = {0};

        targetinfo.texture = swapchainTex;
        targetinfo.clear_color = (SDL_FColor) {0.0f, 0.0f, 0.0f, 1.0f};
        targetinfo.load_op = SDL_GPU_LOADOP_CLEAR;
        targetinfo.store_op = SDL_GPU_STOREOP_STORE;

        SDL_GPURenderPass* renderpass = SDL_BeginGPURenderPass(cmdbuf, &targetinfo, 1, NULL);

        SDL_BindGPUGraphicsPipeline(renderpass, wireframeMode ? linePipeline : fillPipeline);

        if (smallViewportMode) {
            SDL_SetGPUViewport(renderpass, &smallView);
        }

        if (scissorRectMode) {
            SDL_SetGPUScissor(renderpass, &scissorRect);
        }

        SDL_DrawGPUPrimitives(renderpass, 3, 1, 0, 0);
        SDL_EndGPURenderPass(renderpass);
    }

    SDL_SubmitGPUCommandBuffer(cmdbuf);
}

void cleanup(context_t* context)
{
    SDL_ReleaseGPUGraphicsPipeline(context->dev, fillPipeline);
    SDL_ReleaseGPUGraphicsPipeline(context->dev, linePipeline);
}
