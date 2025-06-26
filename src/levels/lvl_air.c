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

#define MOUSE_SENSITIVITY   (0.04f)

#define F_AFTERBURNER       (1u << 0)

typedef struct aircraft_s {
    obj3D_t*    model;
    tag         tag;

    // constants
    f32         stdthrust_c;            // thrust rating
    f32         abthrust_c;             // afterburner thrust
    f32         lift_c[4];              // lift coefficients per flaps state
    u8          thrustadjspeed_c;
    u8          ctrlsurfspeed_c;
    // limits
    f32         stallspeed_c[4];        // stall speeds per flaps state  
    f32         maxspeed_c[4];          // failure speeds per flaps state
    u8          maxtemp_c;              // failure engine temperature
    // munitions
    u8          missiles_c;             // num missiles
    u8          acrounds_c;             // num autocannon rounds
    f32         rateoffire_c;           // autocannon rate of fire
    f32         missilevelocity_c;      // missile velocity
    f32         shellvelocity_c;        // autocannon shell velocity
    f32         aimtime_c;              // time to achieve missile lock
    f32         missilerange_c;

    // aircraft state
    vec3f_t     v;
    f32         airspeed;
    f32         vspeed;                 // vertical speed
    u8          thrustset;              // thrust setting
    u8          thrust;                 // actual thrust
    i8          ailerons;
    i8          rudder;
    i8          elevator;
    i8          elevatortrim;
    u8          engtemp;
    u8          flaps;
    u8          missiles;
    u8          acrounds;
    u32         ctrlflags;              // various state flags
    
} aircraft_t;

static aircraft_t jet_1 = {
    .tag = TAG_JET,
    .stdthrust_c = 1.0f,
    .abthrust_c = 1.4f,
    .lift_c = {1.0f, 1.2f, 1.4f, 1.6f},
    .thrustadjspeed_c = 1,
    .ctrlsurfspeed_c = 1,
    .stallspeed_c = {3.0f, 2.6f, 2.2f, 1.8f},
    .maxspeed_c = {6.0f, 5.5f, 5.0f, 4.5f},
    .maxtemp_c = 120,
    .missiles_c = 4,
    .acrounds_c = 60,
    .rateoffire_c = 0.4f,
    .missilevelocity_c = 18.0f,
    .shellvelocity_c = 28.0f,
    .aimtime_c = 3.0f,
    .missilerange_c = 200.0f,
};

static aircraft_t* jet = &jet_1;

static context_t* context;
static instate_t* input;
static mouseinput_t* mouse;
static camera_t* camera;

static SDL_AppResult rotateCamera(f32 x, f32 y)
{
    setCameraRotationDeg(camera, camera->pitch + (-y * MOUSE_SENSITIVITY), camera->yaw + (-x * MOUSE_SENSITIVITY));
    cameraUpdate(camera);

    return SDL_APP_CONTINUE;
}

static inline u32 setupJet(aircraft_t* jet)
{
    jet->model = getObjectByTag3D(jet->tag);

    rAssert(jet->model);

    if (jet->model->flags & OBJECT_INCOMPLETE) {
        if (!(jet->model->vtxbuf = parseStl("jet-blend", &jet->model->numvtx, 0xA1A1A1FF)))
            return LVL_INIT_FAILURE;
    }

    jet->model->flags &= ~OBJECT_INCOMPLETE;
    jet->model->flags |= OBJECT_HEAP_ALLOC;
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
    camera->flags |= CAMERA_THIRD_PERSON | CAMERA_NEED_REBUILD;
    cameraUpdate(camera);

    if (setupJet(jet))
        return LVL_INIT_FAILURE;
    
    mouse->mode = MOUSE_MODE_STD;
    mouse->motion = rotateCamera;

    set3DAirInputMap();

    return LVL_INIT_SUCCESS;
}

//
//  Level update
//
static u32 update(f64 dt)
{

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

level_t lvl_air = { "Air", init, update, lvlexit, 0, RENDER_MODE_3D, {EXIT_1, EXIT_2, EXIT_3, EXIT_4} };