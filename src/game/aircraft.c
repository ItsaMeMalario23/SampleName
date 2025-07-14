#include <math.h>

#include <input.h>
#include <algebra.h>
#include <game/aircraft.h>
#include <debug/rdebug.h>

aircraft_t jet_1 = {
    .tag = TAG_JET,
    .abthrust_c = 1.4f,
    .stdthrust_c = { 0.0f, 5.8f, 6.0f, 7.2f, 8.4f, 9.6f },
    .stdlift_c = { 0.1f, 0.12f, 0.14f, 0.16f },
    .drag_c = { 0.02f, 0.04f, 0.06f, 0.1f },
    .thrustadjrate_c = 0.01f,
    .ctrlsurfspeed_c = 1,
    .stallspeed_c = { 3.0f, 2.6f, 2.2f, 1.8f },
    .stallaoa_c = { radf(20.0f), radf(20.0f), radf(20.0f), radf(20.0f) },
    .maxspeed_c = { 6.0f, 5.5f, 5.0f, 4.5f },
    .maxtemp_c = 700
};

inline void jetHandleThrustSetting(aircraft_t* jet, instate_t* input)
{
    rAssert(jet->thrustset < 6);

    // adjust thrust if necessary
    if (jet->thrust + EPSILON < jet->stdthrust_c[jet->thrustset])
        jet->thrust += jet->thrustadjrate_c;

    if (jet->thrust > jet->stdthrust_c[jet->thrustset] + EPSILON)
        jet->thrust -= jet->thrustadjrate_c;

    if (input->dynamic & IN_INC_THRUST && input->dynamic & IN_DEC_THRUST)
        return;

    // handle thrust adjustment input
    if (input->dynamic & IN_INC_THRUST) {
        input->dynamic &= ~IN_INC_THRUST;

        if (jet->thrustset + 1 < THRUST_SET_MAX)
            jet->thrustset++;

        jet->ctrlflags |= F_RECALC_TARGET_V;
    }

    if (input->dynamic & IN_DEC_THRUST) {
        input->dynamic &= ~IN_DEC_THRUST;

        if (jet->thrustset)
            jet->thrustset--;

        jet->ctrlflags |= F_RECALC_TARGET_V;
    }
}

inline void jetHandleCtrlSurfaceInput(aircraft_t* restrict jet, const instate_t* restrict input)
{
    // reset control surfaces
    if (jet->rudder > 0)
        jet->rudder--;
    else if (jet->rudder < 0)
        jet->rudder++;

    if (jet->ailerons > 0)
        jet->ailerons--;
    else if (jet->ailerons < 0)
        jet->ailerons++;

    if (jet->elevator > jet->elevatortrim)
        jet->elevator--;
    else if (jet->elevator < jet->elevatortrim)
        jet->elevator++;

    // handle inputs
    if (input->down) {
        if (jet->elevator < ELEVATOR_MAX)
            jet->elevator += 3;
    }
    
    if (input->up) {
        if (jet->elevator > -ELEVATOR_MAX)
            jet->elevator -= 3;
    }

    if (input->left) {
        if (jet->ailerons < AILERON_MAX)
            jet->ailerons += 3;
    }
    
    if (input->right) {
        if (jet->ailerons > -AILERON_MAX)
            jet->ailerons -= 3;
    }

    if (input->dynamic & IN_RUDDER_R) {
        if (jet->rudder < RUDDER_MAX)
            jet->rudder += 3;
    }

    if (input->dynamic & IN_RUDDER_L) {
        if (jet->rudder > -RUDDER_MAX)
            jet->rudder -= 3;
    }
}

inline void jetGetLocalAxes(aircraft_t* jet)
{
    mat4_t rot = qToM4(jet->attitude);

    jet->local[LAXIS_RIGHT] = v3Normalize((vec3f_t) { rot.m11, rot.m12, rot.m13 });
    jet->local[LAXIS_UP] = v3Normalize((vec3f_t) { rot.m21, rot.m22, rot.m23 });
    jet->local[LAXIS_FORWARD] = v3Normalize((vec3f_t) { rot.m31, rot.m32, rot.m33 });
}

inline void jetComputeThrust(aircraft_t* jet, f32 dt)
{
    rAssert(jet->thrustset < 6);

    f32 target = jet->stdthrust_c[jet->thrustset];

    if (jet->ctrlflags & F_AFTERBURNER)
    {
        target *= jet->abthrust_c;
        jet->engtemp++;
    }
    else if (jet->engtemp)
    {
        jet->engtemp--;
    }

    f32 diff = target - jet->thrust;
    f32 rate = jet->thrustadjrate_c * dt;

    if (SDL_fabsf(diff) > rate)
        jet->thrust += (diff > 0.0f ? rate : -rate);
    else
        jet->thrust = target;

    vec3f_t dir = v3Normalize(jet->local[LAXIS_FORWARD]);
    vec3f_t f = v3Scale(dir, jet->thrust * dt);

    jet->v = v3Add(jet->v, f);
}

static inline f32 getAirfoilLiftCoeff(f32 aoa)
{
    if (aoa > 90.0f || aoa < -90.0f)
        return 0.0f;

    return (aoa / 180.0f) + 0.2f;
}

static inline f32 getAirfoilDragCoeff(f32 aoa)
{
    if (aoa < 0.0f)
        aoa = -aoa;

    return aoa / 900.0f;
}

inline void jetHandleLift(aircraft_t* jet, f32 dt)
{
    rAssert(jet->flaps < 4);

    // airspeed
    vec3f_t dir = v3Normalize(jet->v);

    register f32 airspeed = SDL_fabsf(v3Dot(jet->v, jet->local[LAXIS_FORWARD]));

    //register f32 airspeed = v3Len(jet->v);
    jet->airspeed = airspeed;

    // gravity
    jet->v = v3Add(jet->v, (vec3f_t) { 0.0f, GRAVITY * dt, 0.0f });

    if (airspeed < EPSILON) {
        jet->lift = 0.0f;
        return;
    }

    // lift
    f32 aoa = -SDL_atan2f(jet->local[LAXIS_UP].z, jet->local[LAXIS_UP].y);
    jet->aoa = SDL_fmodf(degf(aoa), 180.0f);

    //f32 cl = jet->stdlift_c[jet->flaps] * (aoa / jet->stallaoa_c[jet->flaps]);

    airspeed *= airspeed;

    f32 lift = 0.05f * airspeed * getAirfoilLiftCoeff(jet->aoa);
    jet->lift = lift;

    vec3f_t liftf = v3Scale(jet->local[LAXIS_UP], lift * dt);

    jet->v = v3Add(jet->v, liftf);

    // drag
    f32 drag = 0.5f * airspeed * jet->drag_c[jet->flaps] + getAirfoilDragCoeff(jet->aoa);
    vec3f_t dragf = v3Scale(dir, -drag * dt);

    jet->v = v3Add(jet->v, dragf);
}

inline void jetUpdatePosition(aircraft_t* jet, f32 dt)
{
    f32 yaw = radf((f32) jet->rudder * dt * 0.25f);
    f32 pitch = radf((f32) jet->elevator * dt * 0.25f);
    f32 roll = radf((f32) jet->ailerons * dt * 0.25f);

    quat_t qdy = qFromAxis(jet->local[LAXIS_UP], yaw);
    quat_t qdp = qFromAxis(jet->local[LAXIS_RIGHT], pitch);
    quat_t qdr = qFromAxis(jet->local[LAXIS_FORWARD], roll);

    // delta rotation roll -> pitch -> yaw
    quat_t d = qMul(qMul(qdr, qdp), qdy);

    jet->attitude = qMul(jet->attitude, d);
    jet->attitude = qNormalize(jet->attitude);

    jet->model->transform = qToM4(jet->attitude);

    jet->pos.x += jet->v.x * dt;
    jet->pos.z += jet->v.z * dt;

    f32 dy = jet->v.y * dt;

    // ground collision
    if (jet->pos.y + dy > FLOOR_PLANE) {
       jet->pos.y += jet->v.y * dt;
    } else {
        jet->pos.y = FLOOR_PLANE;
        jet->v.y = 0.0f;
    }

    jet->model->transform.m41 = jet->pos.x;
    jet->model->transform.m42 = jet->pos.y;
    jet->model->transform.m43 = jet->pos.z;
}

void jetPrintDebugVals(const aircraft_t* jet, f32 t)
{
    static u32 fcnt;
    SDL_Log
    (
        "frame %6ld +%.6f\n"
        "POS X %6.2f  Y %6.2f  Z %6.2f\n"
        "ROT Y %6.2f  P %6.2f  R %6.2f\n"
        "VEL X %6.2f  Y %6.2f  Z %6.2f\n"
        "thrustset:    %d / 5\n"
        "thrust:       %.4f\n"
        "lift:         %.4f\n"
        "aoa           %.4f\n"
        "airspeed      %.4f\n"
        "rudder:       %d\n"
        "ailerons:     %d\n"
        "elevator:     %d\n\n\n",
        ++fcnt, t - FIXED_DT,
        jet->pos.x, jet->pos.y, jet->pos.z,
        jet->model->yaw, jet->model->pitch, jet->model->roll,
        jet->v.x, jet->v.y, jet->v.z,
        jet->thrustset, jet->thrust, jet->lift, jet->aoa, jet->airspeed,
        jet->rudder, jet->ailerons, jet->elevator
    );
}