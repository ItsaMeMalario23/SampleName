#ifndef ALGEBRA_H
#define ALGEBRA_H

#include <main.h>

typedef struct quat_s {
    f32 x, y, z, w;
} quat_t;

// Matrices
void mulF(mat4_t* restrict dst, const mat4_t* restrict m1, const mat4_t* restrict m2);
void mulA(mat4_t* restrict dst, const mat4_t* restrict m1, const mat4_t* restrict m2);

mat4_t m4Mul(mat4_t m1, mat4_t m2);

mat4_t m4Translate(f32 dx, f32 dy, f32 dz);
mat4_t m4RotY(f32 rad);
mat4_t rot(f32 x, f32 y, f32 z, f32 rad);
mat4_t m4RotIntrRad(f32 roll, f32 yaw, f32 pitch);
mat4_t m4RotAxis(vec3f_t axis, f32 rad);

mat4_t m4Camera(vec3f_t position, vec3f_t direction, vec3f_t camZ);
mat4_t m4Projection(f32 fov, f32 aspect, f32 near, f32 far);

void m4DebugPrint(const mat4_t* m, const char* str);

// Vectors
vec3f_t v3Add(vec3f_t a, vec3f_t b);
vec3f_t v3Scale(vec3f_t v, f32 s);
f32     v3Len(vec3f_t v);
f32     v3Dot(vec3f_t a, vec3f_t b);
vec3f_t v3Normalize(vec3f_t vec);
vec3f_t v3Cross(vec3f_t a, vec3f_t b);

// Quaternions
quat_t  qIdentidy(void);
quat_t  qConjugate(quat_t q);
quat_t  qNormalize(quat_t q);
quat_t  qFromAxis(vec3f_t axis, f32 rad);
quat_t  qMul(quat_t a, quat_t b);
vec3f_t qRotateVec3(vec3f_t v, quat_t q);
mat4_t  qToM4(quat_t q);

#endif