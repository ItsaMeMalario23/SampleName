#include <SDL3/SDL.h>

#include <render/rendercom.h>

static SDL_GPUGraphicsPipeline* pipeline;
static SDL_GPUBuffer* vertexbuf;
static SDL_GPUBuffer* indexbuf;

static bool m_vertexoffset = false;
static bool m_indexoffset = false;
static bool m_indexbuf = true;

static void init(context_t* context)
{
	// Create the shaders
	SDL_GPUShader* vertexShader = renderLoadShader(context->dev, "PositionColorInstanced.vert", 0, 0, 0, 0);
	if (vertexShader == NULL)
	{
		SDL_Log("Failed to create vertex shader!");
		return;
	}

	SDL_GPUShader* fragmentShader = renderLoadShader(context->dev, "solid.frag", 0, 0, 0, 0);
	if (fragmentShader == NULL)
	{
		SDL_Log("Failed to create fragment shader!");
		return;
	}

	// Create the pipeline
	SDL_GPUGraphicsPipelineCreateInfo pipelineCreateInfo = {
		.target_info = {
			.num_color_targets = 1,
			.color_target_descriptions = (SDL_GPUColorTargetDescription[]){{
				.format = SDL_GetGPUSwapchainTextureFormat(context->dev, context->window)
			}},
		},
		// This is set up to match the vertex shader layout!
		.vertex_input_state = (SDL_GPUVertexInputState){
			.num_vertex_buffers = 1,
			.vertex_buffer_descriptions = (SDL_GPUVertexBufferDescription[]){{
				.slot = 0,
				.input_rate = SDL_GPU_VERTEXINPUTRATE_VERTEX,
				.instance_step_rate = 0,
				.pitch = sizeof(vertex_t)
			}},
			.num_vertex_attributes = 2,
			.vertex_attributes = (SDL_GPUVertexAttribute[]){{
				.buffer_slot = 0,
				.format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3,
				.location = 0,
				.offset = 0
			}, {
				.buffer_slot = 0,
				.format = SDL_GPU_VERTEXELEMENTFORMAT_UBYTE4_NORM,
				.location = 1,
				.offset = sizeof(float) * 3
			}}
		},
		.primitive_type = SDL_GPU_PRIMITIVETYPE_TRIANGLELIST,
		.vertex_shader = vertexShader,
		.fragment_shader = fragmentShader
	};

	pipeline = SDL_CreateGPUGraphicsPipeline(context->dev, &pipelineCreateInfo);
	if (pipeline == NULL)
	{
		SDL_Log("Failed to create pipeline!");
		return;
	}

	SDL_ReleaseGPUShader(context->dev, vertexShader);
	SDL_ReleaseGPUShader(context->dev, fragmentShader);

	// Create the vertex and index buffers
	vertexbuf = SDL_CreateGPUBuffer(
		context->dev,
		&(SDL_GPUBufferCreateInfo) {
			.usage = SDL_GPU_BUFFERUSAGE_VERTEX,
			.size = sizeof(vertex_t) * 9
		}
	);

    indexbuf = SDL_CreateGPUBuffer(
        context->dev,
		&(SDL_GPUBufferCreateInfo) {
			.usage = SDL_GPU_BUFFERUSAGE_INDEX,
			.size = sizeof(Uint16) * 6
		}
    );

	// Set the buffer data
	SDL_GPUTransferBuffer* transferBuffer = SDL_CreateGPUTransferBuffer(
		context->dev,
		&(SDL_GPUTransferBufferCreateInfo) {
			.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD,
			.size = (sizeof(vertex_t) * 9) + (sizeof(Uint16) * 6),
		}
	);

	vertex_t* transferData = SDL_MapGPUTransferBuffer(
		context->dev,
		transferBuffer,
		false
	);

	transferData[0] = (vertex_t) { -1, -1, 0, 255,   0,   0, 255 };
	transferData[1] = (vertex_t) {  1, -1, 0,   0, 255,   0, 255 };
	transferData[2] = (vertex_t) {  0,  1, 0,   0,   0, 255, 255 };

	transferData[3] = (vertex_t) { -1, -1, 0, 255, 165,   0, 255 };
	transferData[4] = (vertex_t) {  1, -1, 0,   0, 128,   0, 255 };
	transferData[5] = (vertex_t) {  0,  1, 0,   0, 255, 255, 255 };

	transferData[6] = (vertex_t) { -1, -1, 0, 255, 255, 255, 255 };
	transferData[7] = (vertex_t) {  1, -1, 0, 255, 255, 255, 255 };
	transferData[8] = (vertex_t) {  0,  1, 0, 255, 255, 255, 255 };

    Uint16* indexData = (Uint16*) &transferData[9];
    for (Uint16 i = 0; i < 6; i += 1)
    {
        indexData[i] = i;
    }

	SDL_UnmapGPUTransferBuffer(context->dev, transferBuffer);

	// Upload the transfer data to the vertex and index buffer
	SDL_GPUCommandBuffer* uploadCmdBuf = SDL_AcquireGPUCommandBuffer(context->dev);
	SDL_GPUCopyPass* copyPass = SDL_BeginGPUCopyPass(uploadCmdBuf);

	SDL_UploadToGPUBuffer(
		copyPass,
		&(SDL_GPUTransferBufferLocation) {
			.transfer_buffer = transferBuffer,
			.offset = 0
		},
		&(SDL_GPUBufferRegion) {
			.buffer = vertexbuf,
			.offset = 0,
			.size = sizeof(vertex_t) * 9
		},
		false
	);

	SDL_UploadToGPUBuffer(
		copyPass,
		&(SDL_GPUTransferBufferLocation) {
			.transfer_buffer = transferBuffer,
			.offset = sizeof(vertex_t) * 9
		},
		&(SDL_GPUBufferRegion) {
			.buffer = indexbuf,
			.offset = 0,
			.size = sizeof(Uint16) * 6
		},
		false
	);

	SDL_EndGPUCopyPass(copyPass);
	SDL_SubmitGPUCommandBuffer(uploadCmdBuf);
	SDL_ReleaseGPUTransferBuffer(context->dev, transferBuffer);
}

static void update(u32 mode)
{
	switch (mode) {
	case 0:
		m_vertexoffset = !m_vertexoffset;
		SDL_Log("Using vertex offset: %s", m_vertexoffset ? "true" : "false");
		break;

	case 1:
		m_indexoffset = !m_indexoffset;
		SDL_Log("Using index offset: %s", m_indexoffset ? "true" : "false");
		break;

	case 2:
		m_indexbuf = !m_indexbuf;
		SDL_Log("Using index buffer: %s", m_indexbuf ? "true" : "false");
		break;
	}
}

static void draw(context_t* context)
{
    Uint32 vertexOffset = m_vertexoffset ? 3 : 0;
    Uint32 indexOffset = m_indexoffset ? 3 : 0;

    SDL_GPUCommandBuffer* cmdbuf = SDL_AcquireGPUCommandBuffer(context->dev);
    if (cmdbuf == NULL)
    {
        SDL_Log("AcquireGPUCommandBuffer failed: %s", SDL_GetError());
        return;
    }

    SDL_GPUTexture* swapchainTexture;
    if (!SDL_WaitAndAcquireGPUSwapchainTexture(cmdbuf, context->window, &swapchainTexture, NULL, NULL)) {
        SDL_Log("WaitAndAcquireGPUSwapchainTexture failed: %s", SDL_GetError());
        return;
    }

	if (swapchainTexture != NULL)
	{
		SDL_GPUColorTargetInfo colorTargetInfo = { 0 };
		colorTargetInfo.texture = swapchainTexture;
		colorTargetInfo.clear_color = (SDL_FColor) { 0.0f, 0.0f, 0.0f, 1.0f };
		colorTargetInfo.load_op = SDL_GPU_LOADOP_CLEAR;
		colorTargetInfo.store_op = SDL_GPU_STOREOP_STORE;

		SDL_GPURenderPass* renderPass = SDL_BeginGPURenderPass(cmdbuf, &colorTargetInfo, 1, NULL);

		SDL_BindGPUGraphicsPipeline(renderPass, pipeline);
		SDL_BindGPUVertexBuffers(renderPass, 0, &(SDL_GPUBufferBinding){ .buffer = vertexbuf, .offset = 0 }, 1);

		if (m_indexbuf) {
			SDL_BindGPUIndexBuffer(renderPass, &(SDL_GPUBufferBinding){ .buffer = indexbuf, .offset = 0 }, SDL_GPU_INDEXELEMENTSIZE_16BIT);
			SDL_DrawGPUIndexedPrimitives(renderPass, 3, 16, indexOffset, vertexOffset, 0);
		} else {
			SDL_DrawGPUPrimitives(renderPass, 3, 16, vertexOffset, 0);
		}

		SDL_EndGPURenderPass(renderPass);
	}

	SDL_SubmitGPUCommandBuffer(cmdbuf);
}

static void cleanup(context_t* context)
{
	SDL_ReleaseGPUGraphicsPipeline(context->dev, pipeline);
	SDL_ReleaseGPUBuffer(context->dev, vertexbuf);
    SDL_ReleaseGPUBuffer(context->dev, indexbuf);

    m_vertexoffset = false;
    m_indexoffset = false;
	m_indexbuf = true;
}

renderer_t r_instanced = {RENDERTYPE_2D, init, update, draw, cleanup};