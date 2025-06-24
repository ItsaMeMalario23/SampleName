#ifndef CAMERA_H
#define CAMERA_H

#include <main.h>

#define radf( x ) ((x) * SDL_PI_F / 180.0f)
#define degf( x ) ((x) * 180.0f / SDL_PI_F)

typedef struct mat4_s {
    f32 m11, m12, m13, m14;
    f32 m21, m22, m23, m24;
    f32 m31, m32, m33, m34;
    f32 m41, m42, m43, m44;
} mat4_t;

#define CAM_THIRD_PERS_OFFS (-0.8f)

#define CAMERA_NEED_REBUILD (1u << 0)
#define CAMERA_THIRD_PERSON (1u << 1)

typedef struct camera_s {
    mat4_t  rendermat;
    mat4_t  view;
    mat4_t  proj;
    vec3f_t pos;
    f32     pitch, yaw;
    f32     nearplane;
    f32     farplane;
    f32     aspect;
    f32     fov;
    u64     flags;
} camera_t;

#define OBJECT_VISIBLE      (1u << 0)
#define OBJECT_NEED_REBUILD (1u << 1)
#define OBJECT_RENDER       (1u << 2)
#define OBJECT_HEAP_ALLOC   (1u << 3)
#define OBJECT_INCOMPLETE   (1u << 4)

typedef struct obj3D_s {
    const vec3f_t*  vtxbuf;
    mat4_t          transform;
    vec3f_t         pos;
    u32             vtxbufoffset;
    u32             numvtx;
    f32             pitch;
    f32             yaw;
    f32             roll;
    u32             flags;
    u64             tag;
} obj3D_t;

//
//  Camera
//
void cameraInit(camera_t* const camera);
void cameraUpdate(camera_t* const camera);

void setCameraViewport(camera_t* const camera, f32 width, f32 height);
void setCameraPosition(camera_t* const camera, f32 x, f32 y, f32 z);
void setCameraRotationDeg(camera_t* const camera, f32 pitch, f32 yaw);

//
//  3D Objects
//
void objectUpdate(obj3D_t* const object);
void setObjectPosition(obj3D_t* const object, f32 x, f32 y, f32 z);
void setObjectRotationDeg(obj3D_t* const object, f32 pitch, f32 yaw, f32 roll);

//
//  Math
//
vec3f_t normalizeV3F(vec3f_t vec);
f32     dotV3F(vec3f_t a, vec3f_t b);
vec3f_t crossV3F(vec3f_t a, vec3f_t b);

mat4_t mulM4(mat4_t m1, mat4_t m2);
mat4_t rotationM4(f32 rad);
mat4_t translationM4(f32 dx, f32 dy, f32 dz);
mat4_t cameraM4(vec3f_t position, vec3f_t direction, vec3f_t camZ);

#endif