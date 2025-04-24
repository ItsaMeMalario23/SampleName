#include <SDL3/SDL.h>

#include <render/rendercom.h>

static const char* names[] = {
    "PointClamp",
    "PointWrap",
    "LinearClamp",
    "LinearWrap",
    "AnisotropicClamp",
    "AnisotropicWrap"
};

static SDL_GPUGraphicsPipeline* pipeline;
static SDL_GPUBuffer* vertexbuf;
static SDL_GPUBuffer* indexbuf;
static SDL_GPUTexture* texture;
static SDL_GPUSampler* samplers[SDL_arraysize(names)];

static int samplerIdx = 0;

static void init(context_t* con)
{
	// Create the shaders
	SDL_GPUShader* vertexShader = renderLoadShader(con->dev, "TexturedQuad.vert", 0, 0, 0, 0);
	if (vertexShader == NULL)
	{
		SDL_Log("Failed to create vertex shader!");
		return;
	}

	SDL_GPUShader* fragmentShader = renderLoadShader(con->dev, "TexturedQuad.frag", 1, 0, 0, 0);
	if (fragmentShader == NULL)
	{
		SDL_Log("Failed to create fragment shader!");
		return;
	}

	// Load the image
	SDL_Surface *imageData = renderLoadBmp("test1.bmp");
	if (imageData == NULL)
	{
		SDL_Log("Could not load image data!");
		return;
	}

	// Create the pipeline
	SDL_GPUGraphicsPipelineCreateInfo pipelineCreateInfo = {
		.target_info = {
			.num_color_targets = 1,
			.color_target_descriptions = (SDL_GPUColorTargetDescription[]){{
				.format = SDL_GetGPUSwapchainTextureFormat(con->dev, con->window)
			}},
		},
		.vertex_input_state = (SDL_GPUVertexInputState){
			.num_vertex_buffers = 1,
			.vertex_buffer_descriptions = (SDL_GPUVertexBufferDescription[]){{
				.slot = 0,
				.input_rate = SDL_GPU_VERTEXINPUTRATE_VERTEX,
				.instance_step_rate = 0,
				.pitch = sizeof(texvertex_t)
			}},
			.num_vertex_attributes = 2,
			.vertex_attributes = (SDL_GPUVertexAttribute[]){{
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
		.vertex_shader = vertexShader,
		.fragment_shader = fragmentShader
	};

	pipeline = SDL_CreateGPUGraphicsPipeline(con->dev, &pipelineCreateInfo);
	if (pipeline == NULL)
	{
		SDL_Log("Failed to create pipeline!");
		return;
	}

	SDL_ReleaseGPUShader(con->dev, vertexShader);
	SDL_ReleaseGPUShader(con->dev, fragmentShader);

	// PointClamp
	samplers[0] = SDL_CreateGPUSampler(con->dev, &(SDL_GPUSamplerCreateInfo){
		.min_filter = SDL_GPU_FILTER_NEAREST,
		.mag_filter = SDL_GPU_FILTER_NEAREST,
		.mipmap_mode = SDL_GPU_SAMPLERMIPMAPMODE_NEAREST,
		.address_mode_u = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE,
		.address_mode_v = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE,
		.address_mode_w = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE,
	});
	// PointWrap
	samplers[1] = SDL_CreateGPUSampler(con->dev, &(SDL_GPUSamplerCreateInfo){
		.min_filter = SDL_GPU_FILTER_NEAREST,
		.mag_filter = SDL_GPU_FILTER_NEAREST,
		.mipmap_mode = SDL_GPU_SAMPLERMIPMAPMODE_NEAREST,
		.address_mode_u = SDL_GPU_SAMPLERADDRESSMODE_REPEAT,
		.address_mode_v = SDL_GPU_SAMPLERADDRESSMODE_REPEAT,
		.address_mode_w = SDL_GPU_SAMPLERADDRESSMODE_REPEAT,
	});
	// LinearClamp
	samplers[2] = SDL_CreateGPUSampler(con->dev, &(SDL_GPUSamplerCreateInfo){
		.min_filter = SDL_GPU_FILTER_LINEAR,
		.mag_filter = SDL_GPU_FILTER_LINEAR,
		.mipmap_mode = SDL_GPU_SAMPLERMIPMAPMODE_LINEAR,
		.address_mode_u = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE,
		.address_mode_v = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE,
		.address_mode_w = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE,
	});
	// LinearWrap
	samplers[3] = SDL_CreateGPUSampler(con->dev, &(SDL_GPUSamplerCreateInfo){
		.min_filter = SDL_GPU_FILTER_LINEAR,
		.mag_filter = SDL_GPU_FILTER_LINEAR,
		.mipmap_mode = SDL_GPU_SAMPLERMIPMAPMODE_LINEAR,
		.address_mode_u = SDL_GPU_SAMPLERADDRESSMODE_REPEAT,
		.address_mode_v = SDL_GPU_SAMPLERADDRESSMODE_REPEAT,
		.address_mode_w = SDL_GPU_SAMPLERADDRESSMODE_REPEAT,
	});
	// AnisotropicClamp
	samplers[4] = SDL_CreateGPUSampler(con->dev, &(SDL_GPUSamplerCreateInfo){
		.min_filter = SDL_GPU_FILTER_LINEAR,
		.mag_filter = SDL_GPU_FILTER_LINEAR,
		.mipmap_mode = SDL_GPU_SAMPLERMIPMAPMODE_LINEAR,
		.address_mode_u = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE,
		.address_mode_v = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE,
		.address_mode_w = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE,
		.enable_anisotropy = true,
		.max_anisotropy = 4
	});
	// AnisotropicWrap
	samplers[5] = SDL_CreateGPUSampler(con->dev, &(SDL_GPUSamplerCreateInfo){
		.min_filter = SDL_GPU_FILTER_LINEAR,
		.mag_filter = SDL_GPU_FILTER_LINEAR,
		.mipmap_mode = SDL_GPU_SAMPLERMIPMAPMODE_LINEAR,
		.address_mode_u = SDL_GPU_SAMPLERADDRESSMODE_REPEAT,
		.address_mode_v = SDL_GPU_SAMPLERADDRESSMODE_REPEAT,
		.address_mode_w = SDL_GPU_SAMPLERADDRESSMODE_REPEAT,
		.enable_anisotropy = true,
		.max_anisotropy = 4
	});

	// Create the GPU resources
	vertexbuf = SDL_CreateGPUBuffer(
		con->dev,
		&(SDL_GPUBufferCreateInfo) {
			.usage = SDL_GPU_BUFFERUSAGE_VERTEX,
			.size = sizeof(texvertex_t) * 4
		}
	);
	SDL_SetGPUBufferName(
		con->dev,
		vertexbuf,
		"Ravioli Vertex Buffer"
	);

	indexbuf = SDL_CreateGPUBuffer(
		con->dev,
		&(SDL_GPUBufferCreateInfo) {
			.usage = SDL_GPU_BUFFERUSAGE_INDEX,
			.size = sizeof(Uint16) * 6
		}
	);

	texture = SDL_CreateGPUTexture(con->dev, &(SDL_GPUTextureCreateInfo){
		.type = SDL_GPU_TEXTURETYPE_2D,
		.format = SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM,
		.width = imageData->w,
		.height = imageData->h,
		.layer_count_or_depth = 1,
		.num_levels = 1,
		.usage = SDL_GPU_TEXTUREUSAGE_SAMPLER
	});
	SDL_SetGPUTextureName(
		con->dev,
		texture,
		"Ravioli Texture"
	);

	// Set up buffer data
	SDL_GPUTransferBuffer* bufferTransferBuffer = SDL_CreateGPUTransferBuffer(
		con->dev,
		&(SDL_GPUTransferBufferCreateInfo) {
			.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD,
			.size = (sizeof(texvertex_t) * 4) + (sizeof(Uint16) * 6)
		}
	);

	texvertex_t* transferData = SDL_MapGPUTransferBuffer(
		con->dev,
		bufferTransferBuffer,
		false
	);

    f32 x = 200.0f;  // 1280
    f32 y = 200.0f;  // 720
    f32 s = 0.2f;

    // origin: (x / 640) - 1, (y / 360) - 1
    // bot right: nx + 2s, ny + 2s

    f32 u = (x / 640.0f) - 1.0f;
    f32 v = (y / 360.0f) - 1.0f;

    // tl: u     , v
    // tr: u + 2s, v
    // bl: u     , v + 2s
    // br: u + 2s, v + 2s

    SDL_Log("u: %.2f, v: %.2f, u+: %.2f, v+: %.2f", u, v, u + (2*s), v + (2*s));

	transferData[0] = (texvertex_t) {  u,  v + (2 * s), 0, 0, 0 };
	transferData[1] = (texvertex_t) {  u + (2 * s),  v + (2 * s), 0, 1, 0 };
	transferData[2] = (texvertex_t) {  u + (2 * s),  v, 0, 1, 1 };
	transferData[3] = (texvertex_t) {  u,  v, 0, 0, 1 };

    /*
	transferData[0] = (texvertex_t) { -1,  1, 0, 0, 0 };
	transferData[1] = (texvertex_t) {  1,  1, 0, 4, 0 };
	transferData[2] = (texvertex_t) {  1, -1, 0, 4, 4 };
	transferData[3] = (texvertex_t) { -1, -1, 0, 0, 4 };
    */

	Uint16* indexData = (Uint16*) &transferData[4];
	indexData[0] = 0;
	indexData[1] = 1;
	indexData[2] = 2;
	indexData[3] = 0;
	indexData[4] = 2;
	indexData[5] = 3;

	SDL_UnmapGPUTransferBuffer(con->dev, bufferTransferBuffer);

	// Set up texture data
	SDL_GPUTransferBuffer* textureTransferBuffer = SDL_CreateGPUTransferBuffer(
		con->dev,
		&(SDL_GPUTransferBufferCreateInfo) {
			.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD,
			.size = imageData->w * imageData->h * 4
		}
	);

	Uint8* textureTransferPtr = SDL_MapGPUTransferBuffer(
		con->dev,
		textureTransferBuffer,
		false
	);
	SDL_memcpy(textureTransferPtr, imageData->pixels, imageData->w * imageData->h * 4);
	SDL_UnmapGPUTransferBuffer(con->dev, textureTransferBuffer);

	// Upload the transfer data to the GPU resources
	SDL_GPUCommandBuffer* uploadCmdBuf = SDL_AcquireGPUCommandBuffer(con->dev);
	SDL_GPUCopyPass* copyPass = SDL_BeginGPUCopyPass(uploadCmdBuf);

	SDL_UploadToGPUBuffer(
		copyPass,
		&(SDL_GPUTransferBufferLocation) {
			.transfer_buffer = bufferTransferBuffer,
			.offset = 0
		},
		&(SDL_GPUBufferRegion) {
			.buffer = vertexbuf,
			.offset = 0,
			.size = sizeof(texvertex_t) * 4
		},
		false
	);

	SDL_UploadToGPUBuffer(
		copyPass,
		&(SDL_GPUTransferBufferLocation) {
			.transfer_buffer = bufferTransferBuffer,
			.offset = sizeof(texvertex_t) * 4
		},
		&(SDL_GPUBufferRegion) {
			.buffer = indexbuf,
			.offset = 0,
			.size = sizeof(Uint16) * 6
		},
		false
	);

	SDL_UploadToGPUTexture(
		copyPass,
		&(SDL_GPUTextureTransferInfo) {
			.transfer_buffer = textureTransferBuffer,
			.offset = 0, /* Zeros out the rest */
		},
		&(SDL_GPUTextureRegion){
			.texture = texture,
			.w = imageData->w,
			.h = imageData->h,
			.d = 1
		},
		false
	);

	SDL_EndGPUCopyPass(copyPass);
	SDL_SubmitGPUCommandBuffer(uploadCmdBuf);
	SDL_DestroySurface(imageData);
	SDL_ReleaseGPUTransferBuffer(con->dev, bufferTransferBuffer);
	SDL_ReleaseGPUTransferBuffer(con->dev, textureTransferBuffer);

    SDL_Log("Sampler: %s", names[samplerIdx]);
}

static void update(u32 mode)
{
    if (mode == 0 && --samplerIdx < 0)
    {
        samplerIdx = SDL_arraysize(samplers) - 1;
    }
    else if (mode == 1)
    {
        samplerIdx = (samplerIdx + 1) % SDL_arraysize(samplers);
    }

    SDL_Log("Sampler: %s", names[samplerIdx]);
}

static void draw(context_t* context)
{
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
		colorTargetInfo.clear_color = (SDL_FColor){ 0.0f, 0.0f, 0.0f, 1.0f };
		colorTargetInfo.load_op = SDL_GPU_LOADOP_CLEAR;
		colorTargetInfo.store_op = SDL_GPU_STOREOP_STORE;

		SDL_GPURenderPass* renderPass = SDL_BeginGPURenderPass(cmdbuf, &colorTargetInfo, 1, NULL);

		SDL_BindGPUGraphicsPipeline(renderPass, pipeline);

		SDL_BindGPUVertexBuffers(renderPass, 0, &(SDL_GPUBufferBinding){ .buffer = vertexbuf, .offset = 0 }, 1);
		SDL_BindGPUIndexBuffer(renderPass, &(SDL_GPUBufferBinding){ .buffer = indexbuf, .offset = 0 }, SDL_GPU_INDEXELEMENTSIZE_16BIT);
		
        SDL_BindGPUFragmentSamplers(renderPass, 0, &(SDL_GPUTextureSamplerBinding){ .texture = texture, .sampler = samplers[samplerIdx] }, 1);
		
        SDL_DrawGPUIndexedPrimitives(renderPass, 6, 1, 0, 0, 0);

		SDL_EndGPURenderPass(renderPass);
	}

	SDL_SubmitGPUCommandBuffer(cmdbuf);
}

static void cleanup(context_t* context)
{
	SDL_ReleaseGPUGraphicsPipeline(context->dev, pipeline);
	SDL_ReleaseGPUBuffer(context->dev, vertexbuf);
	SDL_ReleaseGPUBuffer(context->dev, indexbuf);
	SDL_ReleaseGPUTexture(context->dev, texture);

	for (int i = 0; i < SDL_arraysize(samplers); i += 1)
	{
		SDL_ReleaseGPUSampler(context->dev, samplers[i]);
	}

	samplerIdx = 0;
}

renderer_t r_textest = {RENDERTYPE_2D, init, update, draw, cleanup};