#include <levels.h>
#include <input.h>
#include <objects.h>
#include <render/render.h>
#include <render/camera.h>
#include <debug/rdebug.h>

#define MOVE_PLAYER         (0.8f)
#define MOUSE_SENSITIVITY   (0.04f)

static context_t* context;
static instate_t* input;
static mouseinput_t* mouse;
static camera_t* camera;
static gameobj_t* player;

static u32 odyssey;

//
//  Local functions
//
static void initOdyssey(void)
{
    odyssey = 1;

    renderMode(RENDER_MODE_ODYSSEY);

    lvl_mario.init(NULL);

    setup3DVtxBuf(objects3D, 3);

    objects3D[1].flags &= ~OBJECT_VISIBLE;

    objectUpdate(wall_3D);

    mouse->mode = MOUSE_DISABLE_ALL;
}

static void exitOdyssey(void)
{
    odyssey = 0;

    lvl_mario.exit();

    renderMode(RENDER_MODE_3D);

    setup3DVtxBuf(objects3D, 3);

    objects3D[1].flags |= OBJECT_VISIBLE;

    set3DInputMap();

    mouse->mode = 0;
}

// mouse input callback
static SDL_AppResult rotateCamera(f32 x, f32 y)
{
    setCameraRotationDeg(camera, camera->pitch + (-y * MOUSE_SENSITIVITY), camera->yaw + (-x * MOUSE_SENSITIVITY));
    cameraUpdate(camera);

    return SDL_APP_CONTINUE;
}

//
//  Level init
//
static u32 init(void* restrict data)
{
    input = getInputState();
    mouse = getMouse();
    context = getContext();
    camera = getCamera();

    SDL_SetWindowRelativeMouseMode(context->window, true);

    mouse->mode = 0;
    mouse->motion = rotateCamera;

    set3DInputMap();

    cameraInit(camera);
    setCameraPosition(camera, 0.0f, 0.5f, 2.0f);
    cameraUpdate(camera);

    setup3DVtxBuf(objects3D, 4);

    return 0;
}

//
//  Level update
//
static u32 update(f64 dt)
{
    if (odyssey) {
        u32 ret = lvl_mario.update(dt);

        if (ret == LEVEL_EXIT_1)
            exitOdyssey();

        return LEVEL_CONTINUE;
    }

    f32 dx = 0.0f, dy = 0.0f;

    if (input->up)
        dy -= MOVE_PLAYER * dt;

    if (input->down)
        dy += MOVE_PLAYER * dt;

    if (input->left)
        dx -= MOVE_PLAYER * dt;

    if (input->right)
        dx += MOVE_PLAYER * dt;

    // move relative to camera direction
    if (fnotzero(dy) || fnotzero(dx)) {
        f32 s = SDL_sinf(radf(-camera->yaw));
        f32 c = SDL_cosf(radf(-camera->yaw));
        setCameraPosition(camera, camera->pos.x + ((c * dx) + (-s * dy)), camera->pos.y, camera->pos.z + ((s * dx) + (c * dy)));
    }

    switch (input->dynamic) {
    case 1:
        input->dynamic = 0;
        return LEVEL_EXIT_1;

    case 2:
        input->dynamic = 0;
        initOdyssey();
        break;

    case 3:
        input->dynamic = 0;
        camera->flags ^= CAMERA_THIRD_PERSON;
        break;
    }

    cameraUpdate(camera);

    return LEVEL_CONTINUE;
}

//
//  Level exit
//
static void lvlexit(void)
{
    SDL_Log("[INFO] Odyssey level exit");
    SDL_SetWindowRelativeMouseMode(context->window, false);
}

level_t lvl_odyssey = { "Odyssey", init, update, lvlexit, 0, RENDER_MODE_3D };