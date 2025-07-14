#include <levels.h>
#include <input.h>
#include <objects.h>
#include <algebra.h>
#include <game/aircraft.h>
#include <render/render.h>
#include <render/camera.h>
#include <debug/rdebug.h>

#define EXIT_1  (0u)
#define EXIT_2  (0u)
#define EXIT_3  (0u)
#define EXIT_4  (0u)

#define MOUSE_SENSITIVITY   (0.04f)

#define C_NEARPLANE         (1.0f)
#define C_FARPLANE          (1000.0f)
#define C_OFFSET            (7.0f)
#define C_OFFSET_INCR       (0.5f)

#define COLOR_JET           0xA1A1A1FF
#define COLOR_TERRAIN       0x00AA00FF

static aircraft_t* jet = &jet_1;

static context_t* context;
static instate_t* input;
static mouseinput_t* mouse;
static camera_t* camera;

//
//  Local functions
//
static SDL_AppResult rotateCamera(f32 x, f32 y)
{
    setCameraRotationDeg(camera, camera->pitch + (-y * MOUSE_SENSITIVITY), camera->yaw + (-x * MOUSE_SENSITIVITY));
    return SDL_APP_CONTINUE;
}

static SDL_AppResult offsetCamera(f32 f)
{
    camera->offset -= f * C_OFFSET_INCR;
    camera->flags |= CAMERA_NEED_REBUILD;

    return SDL_APP_CONTINUE;
}

static inline u32 setupTerrain(void)
{
    obj3D_t* tmp = getObjectByTag3D(TAG_TERRAIN, lvl_air.scene3D);

    rAssert(tmp);

    if (tmp->flags & OBJECT_INCOMPLETE) {
        if (!(tmp->vtxbuf = parseStl("terrain", &tmp->numvtx, COLOR_TERRAIN)))
            return LVL_INIT_FAILURE;
    }

    tmp->flags &= ~OBJECT_INCOMPLETE;
    tmp->flags |= OBJECT_HEAP_ALLOC;

    return 0;
}

static inline u32 setupJet(aircraft_t* restrict jet)
{
    jet->model = getObjectByTag3D(jet->tag, lvl_air.scene3D);

    rAssert(jet->model);

    if (jet->model->flags & OBJECT_INCOMPLETE) {
        if (!(jet->model->vtxbuf = parseStl("jet-blend", &jet->model->numvtx, COLOR_JET)))
            return LVL_INIT_FAILURE;
    }

    jet->model->flags &= ~OBJECT_INCOMPLETE;
    jet->model->flags |= OBJECT_HEAP_ALLOC;

    jet->attitude = qIdentidy();

    return 0;
}

static inline void setupCamera(camera_t* restrict cam)
{
    cameraInit(cam);
    setCameraPosition(cam, 2.0f, 0.5f, 4.0f);

    cam->flags |= CAMERA_THIRD_PERSON | CAMERA_NEED_REBUILD;
    cam->offset = C_OFFSET;
    cam->nearplane = C_NEARPLANE;
    cam->farplane = C_FARPLANE;

    cameraUpdate(cam);
}

//
//  Level init
//
static u32 init(void* restrict data)
{
    rAssert(lvl_air.scene3D);

    input = getInputState();
    mouse = getMouse();
    context = getContext();
    camera = renderGetCamera();

    SDL_SetWindowRelativeMouseMode(context->window, true);

    setupCamera(camera);

    if (setupJet(jet))
        return LVL_INIT_FAILURE;

    if (setupTerrain())
        return LVL_INIT_FAILURE;

    rSetup3DVtxBuf(lvl_air.scene3D);
    
    mouse->mode = MOUSE_MODE_STD;
    mouse->motion = rotateCamera;
    mouse->wheel = offsetCamera;

    set3DAirInputMap();

    return LVL_INIT_SUCCESS;
}

//
//  Level update
//
static u32 update(f64 dt)
{
    static f32 t;

    if ((t += (f32) dt) < FIXED_DT)
        return LEVEL_CONTINUE;

    if (input->dynamic == 1) {
        input->dynamic = 0;
        return LEVEL_EXIT_1;
    }

    jetGetLocalAxes(jet);

    jetHandleThrustSetting(jet, input);
    jetHandleCtrlSurfaceInput(jet, input);

    jetComputeThrust(jet, t);
    jetHandleLift(jet, t);

    jetUpdatePosition(jet, t);

    jetPrintDebugVals(jet, t);

    setCameraPosition(camera, jet->pos.x, jet->pos.y + 0.5f, jet->pos.z);
    cameraUpdate(camera);

    f32* ptr = rDebugVectors(1);

    ptr[0] = jet->pos.x;
    ptr[1] = jet->pos.y;
    ptr[2] = jet->pos.z;
    ptr[4] = jet->pos.x + jet->v.x;
    ptr[5] = jet->pos.y + jet->v.y;
    ptr[6] = jet->pos.z + jet->v.z;

    t = 0.0f;

    return LEVEL_CONTINUE;
}

//
//  Level exit
//
static void lvlexit(void)
{
    SDL_Log("[INFO] Air level exit");
    SDL_SetWindowRelativeMouseMode(context->window, false);
}

level_t lvl_air = { "Air", init, update, lvlexit, NULL, 0, RENDER_MODE_3D | RENDER_MODE_DEBUG_3D | RENDER_MODE_DEBUG_VEC, {EXIT_1, EXIT_2, EXIT_3, EXIT_4} };