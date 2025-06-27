#include <render/camera.h>
#include <debug/rdebug.h>

//
//  Local functions
//
static inline void mul(mat4_t* restrict dst, const mat4_t* restrict m1, const mat4_t* restrict m2)
{
    dst->m11 = (m1->m11 * m2->m11) + (m1->m12 * m2->m21) + (m1->m13 * m2->m31) + (m1->m14 * m2->m41);
    dst->m12 = (m1->m11 * m2->m12) + (m1->m12 * m2->m22) + (m1->m13 * m2->m32) + (m1->m14 * m2->m42);
    dst->m13 = (m1->m11 * m2->m13) + (m1->m12 * m2->m23) + (m1->m13 * m2->m33) + (m1->m14 * m2->m43);
    dst->m14 = (m1->m11 * m2->m14) + (m1->m12 * m2->m24) + (m1->m13 * m2->m34) + (m1->m14 * m2->m44);

    dst->m21 = (m1->m21 * m2->m11) + (m1->m22 * m2->m21) + (m1->m23 * m2->m31) + (m1->m24 * m2->m41);
    dst->m22 = (m1->m21 * m2->m12) + (m1->m22 * m2->m22) + (m1->m23 * m2->m32) + (m1->m24 * m2->m42);
    dst->m23 = (m1->m21 * m2->m13) + (m1->m22 * m2->m23) + (m1->m23 * m2->m33) + (m1->m24 * m2->m43);
    dst->m24 = (m1->m21 * m2->m14) + (m1->m22 * m2->m24) + (m1->m23 * m2->m34) + (m1->m24 * m2->m44);

    dst->m31 = (m1->m31 * m2->m11) + (m1->m32 * m2->m21) + (m1->m33 * m2->m31) + (m1->m34 * m2->m41);
    dst->m32 = (m1->m31 * m2->m12) + (m1->m32 * m2->m22) + (m1->m33 * m2->m32) + (m1->m34 * m2->m42);
    dst->m33 = (m1->m31 * m2->m13) + (m1->m32 * m2->m23) + (m1->m33 * m2->m33) + (m1->m34 * m2->m43);
    dst->m34 = (m1->m31 * m2->m14) + (m1->m32 * m2->m24) + (m1->m33 * m2->m34) + (m1->m34 * m2->m44);

    dst->m41 = (m1->m41 * m2->m11) + (m1->m42 * m2->m21) + (m1->m43 * m2->m31) + (m1->m44 * m2->m41);
    dst->m42 = (m1->m41 * m2->m12) + (m1->m42 * m2->m22) + (m1->m43 * m2->m32) + (m1->m44 * m2->m42);
    dst->m43 = (m1->m41 * m2->m13) + (m1->m42 * m2->m23) + (m1->m43 * m2->m33) + (m1->m44 * m2->m43);
    dst->m44 = (m1->m41 * m2->m14) + (m1->m42 * m2->m24) + (m1->m43 * m2->m34) + (m1->m44 * m2->m44);
}

static void printMatrix(const mat4_t* const m, const char* const str)
{
    SDL_Log(str);
    SDL_Log("%4.4f %4.4f %4.4f %4.4f", m->m11, m->m12, m->m13, m->m14);
    SDL_Log("%4.4f %4.4f %4.4f %4.4f", m->m21, m->m22, m->m23, m->m24);
    SDL_Log("%4.4f %4.4f %4.4f %4.4f", m->m31, m->m32, m->m33, m->m34);
    SDL_Log("%4.4f %4.4f %4.4f %4.4f", m->m41, m->m42, m->m43, m->m44);
}

static inline mat4_t rot(f32 x, f32 y, f32 z, f32 rad)
{
    f32 sin = SDL_sinf(rad);
    f32 cos = SDL_cosf(rad);
    f32 inv = 1.0f - cos;

    return (mat4_t) {
        inv * x * x + cos, inv * x * y - z * sin, inv * z * x + y * sin, 0.0f,
        inv * x * y + z * sin, inv * y * y + cos, inv * y * z - x * sin, 0.0f,
        inv * z * x - y * sin, inv * y * z + x * sin, inv * z * z + cos, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f
    };
}

static inline mat4_t rotIntrRad(f32 roll, f32 yaw, f32 pitch)
{
    f32 sa = SDL_sinf(roll), ca = SDL_cosf(roll);
    f32 sb = SDL_sinf(yaw), cb = SDL_cosf(yaw);
    f32 sg = SDL_sinf(pitch), cg = SDL_cosf(pitch);

    return (mat4_t) {
        ca * cb, (ca * sb * sg) - (sa * cg), (ca * sb * cg) - (sa * sg), 0.0f,
        sa * cb, (sa * sb * sg) + (ca * cg), (sa * sb * cg) - (ca * sg), 0.0f,
            -sb,                    cb * sg,                    cb * cg, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f
    };
}

static inline mat4_t perspective(f32 fov, f32 aspect, f32 near, f32 far)
{
    f32 f = 1.0f / SDL_tanf(fov * 0.5f);

    return (mat4_t) {
        f / aspect, 0, 0, 0,
        0, f, 0, 0,
        0, 0, -(far + near) / (far - near), -1,
        0, 0, -(2.0f * far * near) / (far - near), 0
    };
}

static inline void thirdPers(camera_t* camera)
{
    camera->rendermat = (mat4_t) { 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, -camera->pos.x, -camera->pos.y, -camera->pos.z, 1 };

    camera->proj = rotIntrRad(0.0f, radf(camera->yaw), radf(camera->pitch));

    mul(&camera->view, &camera->rendermat, &camera->proj);

    mat4_t m;

    mul(&m, &camera->view, &(mat4_t) {1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, CAM_THIRD_PERS_OFFS, 1});

    camera->proj = perspective(camera->fov, camera->aspect, camera->nearplane, camera->farplane);

    mul(&camera->rendermat, &m, &camera->proj);
}

//
//  Public functions
//
void cameraInit(camera_t* const camera)
{
    rAssert(camera);

    context_t* context = getContext();

    camera->pos = (vec3f_t) { 0.0f, 0.0f, 0.0f };
    camera->pitch = 0.0f;
    camera->yaw = 0.0f;
    camera->nearplane = 0.1f;
    camera->farplane = 10.0f;
    camera->aspect = (f32) context->width / (f32) context->height;
    camera->fov = radf(75.0f);
    camera->flags = CAMERA_NEED_REBUILD;

    cameraUpdate(camera);
}

void cameraUpdate(camera_t* const camera)
{
    rAssert(camera);

    if (!(camera->flags & CAMERA_NEED_REBUILD))
        return;

    camera->flags &= ~CAMERA_NEED_REBUILD;

    if (camera->flags & CAMERA_THIRD_PERSON) {
        thirdPers(camera);
        return;
    }

    camera->rendermat = (mat4_t) { 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, -camera->pos.x, -camera->pos.y, -camera->pos.z, 1 };

    camera->proj = rotIntrRad(0.0f, radf(camera->yaw), radf(camera->pitch));

    mul(&camera->view, &camera->rendermat, &camera->proj);

    camera->proj = perspective(camera->fov, camera->aspect, camera->nearplane, camera->farplane);

    mul(&camera->rendermat, &camera->view, &camera->proj);
}

void setCameraViewport(camera_t* const camera, f32 width, f32 height)
{
    rAssert(camera);
    rAssert(width);
    rAssert(height);

    camera->aspect = width / height;
    camera->flags |= CAMERA_NEED_REBUILD;
}

void setCameraPosition(camera_t* const camera, f32 x, f32 y, f32 z)
{
    rAssert(camera);

    camera->pos.x = x;
    camera->pos.y = y;
    camera->pos.z = z;
    camera->flags |= CAMERA_NEED_REBUILD;
}

void setCameraRotationDeg(camera_t* const camera, f32 pitch, f32 yaw)
{
    rAssert(camera);

    if (camera->pitch == pitch && camera->yaw == yaw)
        return;

    /*f32 f = SDL_PI_F / 2.0f - EPSILON;

    if (pitch < -f)
        camera->pitch = -f;
    else if (pitch > f)
        camera->pitch = f;
    else
        camera->pitch = pitch;*/

    camera->pitch = pitch;
    camera->yaw = yaw;
    camera->flags |= CAMERA_NEED_REBUILD;
}

//
//  Objects
//
void objectUpdate(obj3D_t* const object)
{
    rAssert(object);

    if (!(object->flags & OBJECT_NEED_REBUILD))
        return;

    mat4_t m = rotIntrRad(object->yaw, object->pitch, object->roll);

    mul(&object->transform, &(mat4_t) { 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, object->pos.x, object->pos.y, object->pos.z, 1 }, &m);

    object->flags &= ~OBJECT_NEED_REBUILD;
}

void setObjectPosition(obj3D_t* const object, f32 x, f32 y, f32 z)
{
    rAssert(object);

    object->pos.x = x;
    object->pos.y = y;
    object->pos.z = z;
    object->flags |= OBJECT_NEED_REBUILD;
}

void setObjectRotationDeg(obj3D_t* const object, f32 pitch, f32 yaw, f32 roll)
{
    rAssert(object);

    if (object->pitch == pitch && object->yaw == yaw && object->roll == roll)
        return;

    object->pitch = pitch;
    object->yaw = yaw;
    object->roll = roll;
    object->flags |= OBJECT_NEED_REBUILD;
}

//
//  Math
//
vec3f_t normalizeV3F(vec3f_t vec)
{
	f32 m = SDL_sqrtf((vec.x * vec.x) + (vec.y * vec.y) + (vec.z * vec.z));
	return (vec3f_t) { vec.x / m, vec.y / m, vec.z / m };
}

f32 dotV3F(vec3f_t a, vec3f_t b)
{
	return (a.x * b.x) + (a.y * b.y) + (a.z * b.z);
}

vec3f_t crossV3F(vec3f_t a, vec3f_t b)
{
	return (vec3f_t) { a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x };
}

mat4_t mulM4(mat4_t m1, mat4_t m2)
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

mat4_t rotationM4(f32 rad)
{
    return (mat4_t) {
        SDL_cosf(rad), SDL_sinf(rad), 0, 0,
       -SDL_sinf(rad), SDL_cosf(rad), 0, 0,
       0, 0, 1, 0,
       0, 0, 0, 1
    };
}

mat4_t translationM4(f32 dx, f32 dy, f32 dz)
{
    return (mat4_t) {
         1,  0,  0, 0,
         0,  1,  0, 0,
         0,  0,  1, 0,
        dx, dy, dz, 1
    };
}

mat4_t cameraM4(vec3f_t position, vec3f_t direction, vec3f_t camZ)
{
    vec3f_t a = normalizeV3F(direction);
    vec3f_t b = normalizeV3F(crossV3F(camZ, a));
    vec3f_t c = crossV3F(a, b);

    return (mat4_t) {
        b.x, c.x, a.x, 0,
        b.y, c.y, a.y, 0,
        b.z, c.z, a.z, 0,
        -dotV3F(b, position), -dotV3F(c, position), -dotV3F(a, position), 1
    };
}