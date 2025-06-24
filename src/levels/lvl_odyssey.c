#include <levels.h>
#include <input.h>
#include <objects.h>
#include <render/render.h>
#include <render/camera.h>
#include <debug/rdebug.h>

#define MOVE_PLAYER         (0.8f)
#define MOUSE_SENSITIVITY   (0.04f)

#define ODYSSEY_TEXTURE_W   (1920u)
#define ODYSSEY_TEXTURE_H   (1080u)

static context_t* context;
static instate_t* input;
static mouseinput_t* mouse;
static camera_t* camera;

static obj3D_t* player;
static obj3D_t* wall;
static obj3D_t* jet;

static u32 odyssey;

//
//  Local functions
//
static inline void initOdyssey(void)
{
    odyssey = 1;

    renderSetupOdyssey(ODYSSEY_TEXTURE_W, ODYSSEY_TEXTURE_H);

    renderMode(RENDER_MODE_ODYSSEY);

    lvl_mario.init(NULL);

    rSetup3DVtxBuf(objects3D, 5);

    wall->flags &= ~OBJECT_VISIBLE;

    objectUpdate(wall);

    mouse->mode = MOUSE_DISABLE_ALL;
}

static inline void exitOdyssey(void)
{
    odyssey = 0;

    lvl_mario.exit();

    renderMode(RENDER_MODE_3D);

    rSetup3DVtxBuf(objects3D, 5);

    wall->flags |= OBJECT_VISIBLE;

    set3DInputMap();

    mouse->mode = 0;
}

static inline u32 handleInput(void)
{
    switch (input->dynamic) {
    case 1:
        input->dynamic = 0;
        return LEVEL_EXIT_1;

    case 2:
        input->dynamic = 0;
        initOdyssey();
        return LEVEL_CONTINUE;

    case 3:
        input->dynamic = 0;
        camera->flags ^= CAMERA_THIRD_PERSON;
        camera->flags |= CAMERA_NEED_REBUILD;

        player->flags ^= OBJECT_VISIBLE;

        setObjectPosition(player, camera->pos.x, 0.0f, camera->pos.z);

        return LEVEL_CONTINUE;
    }
    
    return LEVEL_CONTINUE;
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
    camera = renderGetCamera();

    SDL_SetWindowRelativeMouseMode(context->window, true);

    mouse->mode = 0;
    mouse->motion = rotateCamera;

    set3DInputMap();

    cameraInit(camera);
    setCameraPosition(camera, 0.0f, 0.5f, 2.0f);
    cameraUpdate(camera);
    
    player = getObjectByTag3D(TAG_PLAYER);
    wall = getObjectByTag3D(TAG_WALL);
    jet = getObjectByTag3D(TAG_JET);

    rAssert(player);
    rAssert(wall);
    rAssert(jet);
    
    if (jet->flags & OBJECT_INCOMPLETE) {
        if (!(jet->vtxbuf = parseStl("jet-blend", &jet->numvtx, 0xA1A1A1FF)))
            return 1;
    }

    jet->flags &= ~OBJECT_INCOMPLETE;
    jet->flags |= OBJECT_HEAP_ALLOC;

    rSetup3DVtxBuf(objects3D, 5);

    player->flags &= ~OBJECT_VISIBLE;

    return 0;
}

//
//  Level update
//
static u32 update(f64 dt)
{
    // update mario level in odyssey mode
    if (odyssey) {
        if (lvl_mario.update(dt) == LEVEL_EXIT_1)
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

        setCameraPosition(camera, camera->pos.x + c * dx - s * dy, camera->pos.y, camera->pos.z + s * dx + c *dy);

        if (camera->flags & CAMERA_THIRD_PERSON)
            setObjectPosition(player, camera->pos.x, 0.0f, camera->pos.z);
    }

    cameraUpdate(camera);

    return handleInput();
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