#ifndef CAMERA_H
#define CAMERA_H

#include <main.h>
#include <objects.h>

#define radf( x ) ((x) * SDL_PI_F / 180.0f)
#define degf( x ) ((x) * 180.0f / SDL_PI_F)

#define CAM_THIRD_PERS_OFFS (1.2f)

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
    f32     offset;
    u64     flags;
} camera_t;

#define OBJECT_VISIBLE      (1u << 0)
#define OBJECT_NEED_REBUILD (1u << 1)
#define OBJECT_RENDER       (1u << 2)
#define OBJECT_HEAP_ALLOC   (1u << 3)
#define OBJECT_INCOMPLETE   (1u << 4)
#define OBJECT_RENDER_DEBUG (1u << 5)

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

#endif