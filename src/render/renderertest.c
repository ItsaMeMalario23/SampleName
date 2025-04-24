#include <SDL3/SDL.h>

#include <render/rendercom.h>

static SDL_GPUGraphicsPipeline* fillpipeline;
static SDL_GPUGraphicsPipeline* linepipeline;
static SDL_GPUViewport smallview = {160, 120, 320, 240, 0.1f, 1.0f};
static SDL_Rect scissorRect = {320, 240, 320, 240};

static bool m_wireframe = 0;
static bool m_smallview = 0;
static bool m_scissorrect = 0;

//static bool init = 0;

void init(context_t* con)
{
    SDL_GPUShader* vertshader = renderLoadShader(con->dev, "triangle.vert", 0, 0, 0, 0);

    if (!vertshader) {
        SDL_Log("Failed to load vertex shader");
        return;
    }

    SDL_GPUShader* fragshader = renderLoadShader(con->dev, "solid.frag", 0, 0, 0, 0);

    if (!fragshader) {
        SDL_Log("Failed to load fragment shader");
        return;
    }

    SDL_GPUGraphicsPipelineCreateInfo pipelineinfo = {
        .target_info = {
            .num_color_targets = 1,
            .color_target_descriptions = (SDL_GPUColorTargetDescription[]) {{
                .format = SDL_GetGPUSwapchainTextureFormat(con->dev, con->window)
            }},
        },
        .primitive_type = SDL_GPU_PRIMITIVETYPE_TRIANGLELIST,
        .vertex_shader = vertshader,
        .fragment_shader = fragshader,
    };

    pipelineinfo.rasterizer_state.fill_mode = SDL_GPU_FILLMODE_FILL;

    fillpipeline = SDL_CreateGPUGraphicsPipeline(con->dev, &pipelineinfo);

    if (!fillpipeline) {
        SDL_Log("Failed to create fill pipeline");
        return;
    }

    pipelineinfo.rasterizer_state.fill_mode = SDL_GPU_FILLMODE_LINE;

    linepipeline = SDL_CreateGPUGraphicsPipeline(con->dev, &pipelineinfo);

    if (!linepipeline) {
        SDL_Log("Failed to create line pipeline");
        return;
    }

    SDL_ReleaseGPUShader(con->dev, vertshader);
    SDL_ReleaseGPUShader(con->dev, fragshader);
}

void draw(context_t* con)
{
    SDL_GPUCommandBuffer* cmdbuf = SDL_AcquireGPUCommandBuffer(con->dev);

    if (!cmdbuf) {
        SDL_Log("Failed to acquire command buf: %s", SDL_GetError());
        return;
    }

    SDL_GPUTexture* swapchaintex;

    if (!SDL_WaitAndAcquireGPUSwapchainTexture(cmdbuf, con->window, &swapchaintex, NULL, NULL)) {
        SDL_Log("Failed to acquire swapchain texture: %s", SDL_GetError());
        return;
    }

    // swapchain texture can be null
    if (swapchaintex) {
        SDL_GPUColorTargetInfo targetinfo = {0};

        targetinfo.texture = swapchaintex;
        targetinfo.clear_color = (SDL_FColor) {0.0f, 0.0f, 0.0f, 1.0f};
        targetinfo.load_op = SDL_GPU_LOADOP_CLEAR;
        targetinfo.store_op = SDL_GPU_STOREOP_STORE;

        SDL_GPURenderPass* renderpass = SDL_BeginGPURenderPass(cmdbuf, &targetinfo, 1, NULL);

        SDL_BindGPUGraphicsPipeline(renderpass, m_wireframe ? linepipeline : fillpipeline);

        if (m_smallview)
            SDL_SetGPUViewport(renderpass, &smallview);

        if (m_scissorrect)
            SDL_SetGPUScissor(renderpass, &scissorRect);

        SDL_DrawGPUPrimitives(renderpass, 3, 1, 0, 0);
        SDL_EndGPURenderPass(renderpass);
    }

    SDL_SubmitGPUCommandBuffer(cmdbuf);
}

void update(u32 mode)
{
    switch (mode) {
    case 0:
        m_wireframe = !m_wireframe;
        break;

    case 1:
        m_smallview = !m_smallview;
        break;

    case 2:
        m_scissorrect = !m_scissorrect;
        break;
    }
}

void cleanup(context_t* con)
{
    SDL_ReleaseGPUGraphicsPipeline(con->dev, fillpipeline);
    SDL_ReleaseGPUGraphicsPipeline(con->dev, linepipeline);
}

renderer_t r_test = {RENDERTYPE_UNDEF, init, update, draw, cleanup};