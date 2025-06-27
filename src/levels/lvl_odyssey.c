#include <levels.h>
#include <input.h>
#include <objects.h>
#include <render/render.h>
#include <render/camera.h>
#include <debug/rdebug.h>

#define EXIT_1  (0u)
#define EXIT_2  (0u)
#define EXIT_3  (0u)
#define EXIT_4  (0u)

#define MOVE_PLAYER         (1.2f)
#define MOUSE_SENSITIVITY   (0.06f)

#define ODYSSEY_TEXTURE_W   (1920u)
#define ODYSSEY_TEXTURE_H   (1080u)

static context_t* context;
static instate_t* input;
static mouseinput_t* mouse;
static camera_t* camera;

static obj3D_t* player;
static obj3D_t* wall;
static obj3D_t* wall2;
static obj3D_t* jet;

static u32 odyssey;

//
//  Local functions
//
static inline void initOdyssey(u32 mode)
{
    odyssey = mode + 1;

    mouse->mode = MOUSE_DISABLE_ALL;

    renderSetupOdyssey(ODYSSEY_TEXTURE_W, ODYSSEY_TEXTURE_H, mode);

    renderMode(RENDER_MODE_ODYSSEY);

    lvl_mario.init(NULL);

    rSetup3DVtxBuf(objects3D, 6);

    if (mode)
        wall2->flags &= ~OBJECT_VISIBLE;
    else
        wall->flags &= ~OBJECT_VISIBLE;

    objectUpdate(wall);
}

static inline void exitOdyssey(void)
{
    lvl_mario.exit();

    renderMode(RENDER_MODE_3D);

    rSetup3DVtxBuf(objects3D, 6);

    wall->flags |= OBJECT_VISIBLE;
    
    if (odyssey == 2)
        wall2->flags |= OBJECT_VISIBLE;
    else
        wall2->flags |= OBJECT_VISIBLE;

    set3DInputMap();

    mouse->mode = 0;

    odyssey = 0;
}

static inline u32 handleInput(void)
{
    switch (input->dynamic) {
    case 1:
        input->dynamic = 0;
        return LEVEL_EXIT_1;

    case 2:
        input->dynamic = 0;
        initOdyssey(0);
        return LEVEL_CONTINUE;

    case 3:
        input->dynamic = 0;
        camera->flags ^= CAMERA_THIRD_PERSON;
        camera->flags |= CAMERA_NEED_REBUILD;

        player->flags ^= OBJECT_VISIBLE;

        setObjectPosition(player, camera->pos.x, 0.0f, camera->pos.z);

        return LEVEL_CONTINUE;

    case 4:
        input->dynamic = 0;
        initOdyssey(1);
        return LEVEL_CONTINUE;

    default:
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

    cameraInit(camera);
    setCameraPosition(camera, 0.0f, 0.5f, 2.0f);
    cameraUpdate(camera);
    
    player = getObjectByTag3D(TAG_PLAYER);
    wall = getObjectByTag3D(TAG_WALL);
    wall2 = getObjectByTag3D(TAG_WALL2);
    jet = getObjectByTag3D(TAG_JET);

    rAssert(player);
    rAssert(wall);
    rAssert(wall2);
    rAssert(jet);
    
    if (jet->flags & OBJECT_INCOMPLETE) {
        if (!(jet->vtxbuf = parseStl("jet-blend", &jet->numvtx, 0xA1A1A1FF)))
            return LVL_INIT_FAILURE;
    }

    jet->flags &= ~OBJECT_INCOMPLETE;
    jet->flags |= OBJECT_HEAP_ALLOC;

    rSetup3DVtxBuf(objects3D, 6);

    rSetupStaticUIBuf(&(uiobject_t) {(asciidata_t[]) {{1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f / 80.0f, 'x' - 32}}, 1}, 1);

    player->flags &= ~OBJECT_VISIBLE;

    mouse->mode = MOUSE_MODE_STD;
    mouse->motion = rotateCamera;

    set3DInputMap();

    return LVL_INIT_SUCCESS;
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

level_t lvl_odyssey = { "Odyssey", init, update, lvlexit, 0, RENDER_MODE_3D | RENDER_MODE_UI_STATIC, {EXIT_1, EXIT_2, EXIT_3, EXIT_4} };