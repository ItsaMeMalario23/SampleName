#include <com.h>

static const char* bpath = NULL;

bool comInit(context_t* context, SDL_WindowFlags flags)
{
    context->dev = SDL_CreateGPUDevice(SDL_GPU_SHADERFORMAT_SPIRV | SDL_GPU_SHADERFORMAT_DXIL | SDL_GPU_SHADERFORMAT_MSL, 0, NULL);

    if (!context->dev) {
        SDL_Log("Failed to create GPU device");
        return 0;
    }

    context->window = SDL_CreateWindow(context->name, 1280, 720, flags);

    if (!context->window) {
        SDL_Log("Failed to create window");
        return 0;
    }

    if (!SDL_ClaimWindowForGPUDevice(context->dev, context->window)) {
        SDL_Log("Failed to claim window for GPU device");
        return 0;
    }

    bpath = SDL_GetBasePath();

    return 1;
}

bool comCleanup(context_t* context)
{
    SDL_ReleaseWindowFromGPUDevice(context->dev, context->window);
    SDL_DestroyWindow(context->window);
    SDL_DestroyGPUDevice(context->dev);
}

SDL_GPUShader* comLoadShader(SDL_GPUDevice* dev, const char* filename, u32 samplerCount, u32 uniformBufCount, u32 storageBufCount, u32 storageTexCount)
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

    SDL_GPUShaderFormat backendFmts = SDL_GetGPUShaderFormats(dev);
    SDL_GPUShaderFormat fmt = SDL_GPU_SHADERFORMAT_INVALID;

    if (backendFmts & SDL_GPU_SHADERFORMAT_SPIRV) {
        SDL_snprintf(path, sizeof(path), "%sshaders/compiled/SPIRV/%s.spv", bpath, filename);
        fmt = SDL_GPU_SHADERFORMAT_SPIRV;
        entry = "main";
    } else if (backendFmts & SDL_GPU_SHADERFORMAT_MSL) {
        SDL_snprintf(path, sizeof(path), "%sshaders/compiled/MSL/%s.msl", bpath, filename);
        fmt = SDL_GPU_SHADERFORMAT_MSL;
        entry = "main0";
    } else if (backendFmts & SDL_GPU_SHADERFORMAT_DXIL) {
        SDL_snprintf(path, sizeof(path), "%sshaders/compiled/DXIL/%s.dxil", bpath, filename);
        fmt = SDL_GPU_SHADERFORMAT_SPIRV;
        entry = "main";
    } else {
        SDL_Log("Failed to recognize shader format");
        return NULL;
    }

    size_t codeLen;
    void* code = SDL_LoadFile(path, &codeLen);

    if (!code) {
        SDL_Log("Failed to load shader: %s", path);
        return NULL;
    }

    SDL_GPUShaderCreateInfo sinfo = {
        .code = code,
        .code_size = codeLen,
        .entrypoint = entry,
        .format = fmt,
        .stage = stage,
        .num_samplers = samplerCount,
        .num_uniform_buffers = uniformBufCount,
        .num_storage_buffers = storageBufCount,
        .num_storage_textures = storageTexCount
    };

    SDL_GPUShader* shader = SDL_CreateGPUShader(dev, &sinfo);

    if (!shader) {
        SDL_Log("Failed to create shader");
        SDL_free(code);
        return NULL;
    }

    SDL_free(code);
    
    return shader;
}

SDL_Surface* comLoadBmp(const char* filename)
{
    char path[256];

    SDL_Surface *surf;

    SDL_snprintf(path, sizeof(path), "%sresources/%s", bpath, filename);

    surf = SDL_LoadBMP(path);

    if (!surf) {
        SDL_Log("Failed to load BMP: %s", SDL_GetError());
        return NULL;
    }

    return surf;
}

mat4x4_t matMultiply(mat4x4_t m1, mat4x4_t m2)
{
    mat4x4_t res;

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

mat4x4_t rotationMatZ(f32 rad)
{
    return (mat4x4_t) {
        SDL_cosf(rad), SDL_sinf(rad), 0, 0,
        -SDL_sinf(rad), SDL_cosf(rad), 0, 0,
        0, 0, 1, 0,
        0, 0, 0, 1
    };
}

mat4x4_t translationMat(f32 dx, f32 dy, f32 dz)
{
    return (mat4x4_t) {
        1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, 1, 0,
        dx, dy, dz, 1
    };
}

vec3f_t normalizeVec3(vec3f_t v)
{
    f32 mag = SDL_sqrtf((v.x * v.x) + (v.y * v.y) + (v.z * v.z));

    return (vec3f_t) {v.x / mag, v.y / mag, v.z / mag};
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