#include <algebra.h>
#include <render/camera.h>
#include <debug/rdebug.h>

//
//  Local functions
//
static inline void thirdPers(camera_t* camera)
{
    camera->rendermat = (mat4_t) { 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, -camera->pos.x, -camera->pos.y, -camera->pos.z, 1 };

    camera->proj = m4RotIntrRad(0.0f, radf(camera->yaw), radf(camera->pitch));

    mulF(&camera->view, &camera->rendermat, &camera->proj);

    mat4_t m;

    mulF(&m, &camera->view, &(mat4_t) {1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, -camera->offset, 1});

    camera->proj = m4Projection(camera->fov, camera->aspect, camera->nearplane, camera->farplane);

    mulF(&camera->rendermat, &m, &camera->proj);
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
    camera->nearplane = CAM_STD_NEARPLANE;
    camera->farplane = CAM_STD_FARPLANE;
    camera->aspect = (f32) context->width / (f32) context->height;
    camera->fov = radf(75.0f);
    camera->offset = CAM_THIRD_PERS_OFFS;
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

    camera->proj = m4RotIntrRad(0.0f, radf(camera->yaw), radf(camera->pitch));

    mulF(&camera->view, &camera->rendermat, &camera->proj);

    camera->proj = m4Projection(camera->fov, camera->aspect, camera->nearplane, camera->farplane);

    mulF(&camera->rendermat, &camera->view, &camera->proj);
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

    mat4_t m = m4RotIntrRad(object->roll, object->pitch, object->yaw);

    mulF(&object->transform, &m, &(mat4_t) {1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, object->pos.x, object->pos.y, object->pos.z, 1});

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