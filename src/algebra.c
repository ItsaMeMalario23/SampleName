#include <algebra.h>

// full matrix 4x4 multiplication (for projection matrices, etc)
inline void mulF(mat4_t* restrict dst, const mat4_t* restrict m1, const mat4_t* restrict m2)
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

// optimized, only works for affine transformations (model transforms, etc)
inline void mulA(mat4_t* restrict dst, const mat4_t* restrict m1, const mat4_t* restrict m2)
{
    mat4_t tmp;

    register f32 e0;
    register f32 e1;
    register f32 e2;

    e0 = m1->m11;
    e1 = m1->m12;
    e2 = m1->m13;
    tmp.m11 = e0 * m2->m11 + e1 * m2->m21 + e2 * m2->m31;
    tmp.m12 = e0 * m2->m12 + e1 * m2->m22 + e2 * m2->m32;
    tmp.m13 = e0 * m2->m13 + e1 * m2->m23 + e2 * m2->m33;

    e0 = m1->m21;
    e1 = m1->m22;
    e2 = m1->m23;
    tmp.m21 = e0 * m2->m11 + e1 * m2->m21 + e2 * m2->m31;
    tmp.m22 = e0 * m2->m12 + e1 * m2->m22 + e2 * m2->m32;
    tmp.m23 = e0 * m2->m13 + e1 * m2->m23 + e2 * m2->m33;

    e0 = m1->m31;
    e1 = m1->m32;
    e2 = m1->m33;
    tmp.m31 = e0 * m2->m11 + e1 * m2->m21 + e2 * m2->m31;
    tmp.m32 = e0 * m2->m12 + e1 * m2->m22 + e2 * m2->m32;
    tmp.m33 = e0 * m2->m13 + e1 * m2->m23 + e2 * m2->m33;

    e0 = m1->m41;
    e1 = m1->m42;
    e2 = m1->m43;
    tmp.m41 = e0 * m2->m11 + e1 * m2->m21 + e2 * m2->m31;
    tmp.m42 = e0 * m2->m12 + e1 * m2->m22 + e2 * m2->m32;
    tmp.m43 = e0 * m2->m13 + e1 * m2->m23 + e2 * m2->m33;

    tmp.m14 = 0.0f;
    tmp.m24 = 0.0f;
    tmp.m34 = 0.0f;
    tmp.m44 = 1.0f;

    memcpy(dst, &tmp, sizeof(mat4_t));
}

inline mat4_t m4Mul(mat4_t m1, mat4_t m2)
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

inline mat4_t m4Translate(f32 dx, f32 dy, f32 dz)
{
    return (mat4_t) {
         1,  0,  0, 0,
         0,  1,  0, 0,
         0,  0,  1, 0,
        dx, dy, dz, 1
    };
}

inline mat4_t m4RotY(f32 rad)
{
    return (mat4_t) {
        SDL_cosf(rad), SDL_sinf(rad), 0, 0,
       -SDL_sinf(rad), SDL_cosf(rad), 0, 0,
       0, 0, 1, 0,
       0, 0, 0, 1
    };
}

inline mat4_t rot(f32 x, f32 y, f32 z, f32 rad)
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

inline mat4_t m4RotIntrRad(f32 roll, f32 yaw, f32 pitch)
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

inline mat4_t m4RotAxis(vec3f_t axis, f32 rad)
{
    f32 c = SDL_cosf(rad);
    f32 s = SDL_sinf(rad);
    f32 ic = 1.0f - c;

    axis = v3Normalize(axis);
    f32 x = axis.x, y = axis.y, z = axis.z;

    return (mat4_t) {
        c + x * x * ic,     x * y * ic - z * s, x * z * ic + y * s, 0.0f,
        y * x * ic + z * s, c + y * y * ic,     y * z * ic - x * s, 0.0f,
        z * x * ic - y * s, z * y * ic + x * s, c + z * z * ic,     0.0f,
        0.0f,               0.0f,               0.0f,               1.0f
    };
}

inline mat4_t m4Camera(vec3f_t position, vec3f_t direction, vec3f_t camZ)
{
    vec3f_t a = v3Normalize(direction);
    vec3f_t b = v3Normalize(v3Cross(camZ, a));
    vec3f_t c = v3Cross(a, b);

    return (mat4_t) {
        b.x, c.x, a.x, 0,
        b.y, c.y, a.y, 0,
        b.z, c.z, a.z, 0,
        -v3Dot(b, position), -v3Dot(c, position), -v3Dot(a, position), 1
    };
}

inline mat4_t m4Projection(f32 fov, f32 aspect, f32 near, f32 far)
{
    register f32 f = 1.0f / SDL_tanf(fov * 0.5f);
    register f32 d = far - near;

    return (mat4_t) {
        f / aspect, 0, 0, 0,
        0, f, 0, 0,
        0, 0, -(far + near) / d, -1.0f,
        0, 0, -(2.0f * far * near) / d, 0
    };
}

inline void m4DebugPrint(const mat4_t* m, const char* str)
{
    SDL_Log(str);
    SDL_Log("%4.4f %4.4f %4.4f %4.4f", m->m11, m->m12, m->m13, m->m14);
    SDL_Log("%4.4f %4.4f %4.4f %4.4f", m->m21, m->m22, m->m23, m->m24);
    SDL_Log("%4.4f %4.4f %4.4f %4.4f", m->m31, m->m32, m->m33, m->m34);
    SDL_Log("%4.4f %4.4f %4.4f %4.4f", m->m41, m->m42, m->m43, m->m44);
}

//
//  Vectors
//
inline vec3f_t v3Add(vec3f_t a, vec3f_t b)
{
    return (vec3f_t) { a.x + b.x, a.y + b.y, a.z + b.z };
}

inline vec3f_t v3Scale(vec3f_t v, f32 s)
{
    return (vec3f_t) { v.x * s, v.y * s, v.z * s };
}

inline f32 v3Len(vec3f_t v)
{
    return SDL_sqrtf((v.x * v.x) + (v.y * v.y) + (v.z * v.z));
}

inline f32 v3Dot(vec3f_t a, vec3f_t b)
{
	return (a.x * b.x) + (a.y * b.y) + (a.z * b.z);
}

inline vec3f_t v3Normalize(vec3f_t v)
{
	f32 m = SDL_sqrtf((v.x * v.x) + (v.y * v.y) + (v.z * v.z));

    if (m == 0.0f)
        return v;

	return (vec3f_t) { v.x / m, v.y / m, v.z / m };
}

inline vec3f_t v3Cross(vec3f_t a, vec3f_t b)
{
	return (vec3f_t) { a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x };
}

//
//  Quaternions
//
inline quat_t qIdentidy(void)
{
    return (quat_t) { 0.0f, 0.0f, 0.0f, 1.0f };
}

inline quat_t qConjugate(quat_t q)
{
    return (quat_t) { -q.x, -q.y, -q.z, q.w };
}

inline quat_t qNormalize(quat_t q)
{
    f32 len = SDL_sqrtf(q.x * q.x + q.y * q.y + q.z * q.z + q.w * q.w);

    if (len == 0.0f)
        return qIdentidy();

    f32 i = 1.0f / len;

    return (quat_t) { q.x * i, q.y * i, q.z * i, q.w * i };
}

// assumes normalized axis vector
inline quat_t qFromAxis(vec3f_t axis, f32 rad) {
    f32 h = rad * 0.5f;

    f32 s = SDL_sinf(h);
    f32 c = SDL_cosf(h);

    return (quat_t) {axis.x * s, axis.y * s, axis.z * s, c};
}

inline quat_t qMul(quat_t a, quat_t b) {
    return (quat_t) {
        a.w * b.x + a.x * b.w + a.y * b.z - a.z * b.y,
        a.w * b.y - a.x * b.z + a.y * b.w + a.z * b.x,
        a.w * b.z + a.x * b.y - a.y * b.x + a.z * b.w,
        a.w * b.w - a.x * b.x - a.y * b.y - a.z * b.z
    };
}

inline vec3f_t qRotateVec3(vec3f_t v, quat_t q)
{
    quat_t vq = { v.x, v.y, v.z, 0.0f };
    quat_t qc = qConjugate(q);
    quat_t r = qMul(qMul(q, vq), qc);
    
    return (vec3f_t) { r.x, r.y, r.z };
}

inline mat4_t qToM4(quat_t q) {
    f32 x = q.x, y = q.y, z = q.z, w = q.w;

    f32 xx = x * x, yy = y * y, zz = z * z;
    f32 xy = x * y, xz = x * z, yz = y * z;
    f32 wx = w * x, wy = w * y, wz = w * z;

    return (mat4_t){
        1.0f - 2.0f * (yy + zz), 2.0f * (xy - wz),     2.0f * (xz + wy),     0.0f,
        2.0f * (xy + wz),     1.0f - 2.0f * (xx + zz), 2.0f * (yz - wx),     0.0f,
        2.0f * (xz - wy),     2.0f * (yz + wx),     1.0f - 2.0f * (xx + yy), 0.0f,
        0.0f,                 0.0f,                 0.0f,                 1.0f
    };
}