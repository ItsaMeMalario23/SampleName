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

static aircraft_t* jet = &jet_1;

static context_t* context;
static instate_t* input;
static mouseinput_t* mouse;
static camera_t* camera;

static SDL_AppResult rotateCamera(f32 x, f32 y)
{
    setCameraRotationDeg(camera, camera->pitch + (-y * MOUSE_SENSITIVITY), camera->yaw + (-x * MOUSE_SENSITIVITY));
    return SDL_APP_CONTINUE;
}

static SDL_AppResult offsetCamera(f32 f)
{
    camera->offset -= f * 0.5f;
    camera->flags |= CAMERA_NEED_REBUILD;

    return SDL_APP_CONTINUE;
}

static inline u32 setupJet(aircraft_t* jet)
{
    jet->model = getObjectByTag3D(jet->tag, objects3D2, 5);

    rAssert(jet->model);

    if (jet->model->flags & OBJECT_INCOMPLETE) {
        if (!(jet->model->vtxbuf = parseStl("jet-blend", &jet->model->numvtx, 0xA1A1A1FF)))
            return LVL_INIT_FAILURE;
    }

    jet->model->flags &= ~OBJECT_INCOMPLETE;
    jet->model->flags |= OBJECT_HEAP_ALLOC;

    jet->attitude = qIdentidy();

    return 0;
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
    setCameraPosition(camera, 2.0f, 0.5f, 4.0f);
    camera->flags |= CAMERA_THIRD_PERSON | CAMERA_NEED_REBUILD;
    camera->offset = 7.0f;
    cameraUpdate(camera);

    if (setupJet(jet))
        return LVL_INIT_FAILURE;

    rSetup3DVtxBuf(objects3D2, 5);
    
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

    jetHandleThrustSetting(jet, input);
    jetHandleCtrSurfaceInput(jet, input);
    
    jetGetLocalAxes(jet);

    jetComputeThrust(jet, t);
    jetHandleLift(jet, t);

    jetUpdatePosition(jet, t);

    jetPrintDebugVals(jet, t);

    setCameraPosition(camera, jet->pos.x, jet->pos.y + 0.5f, jet->pos.z);
    cameraUpdate(camera);

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

level_t lvl_air = { "Air", init, update, lvlexit, 0, RENDER_MODE_3D | RENDER_MODE_DEBUG_3D, {EXIT_1, EXIT_2, EXIT_3, EXIT_4} };