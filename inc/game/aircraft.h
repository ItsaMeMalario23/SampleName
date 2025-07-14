#ifndef AIRCRAFT_H
#define AIRCRAFT_H

#include <main.h>
#include <algebra.h>
#include <objects.h>

#define radf( x ) ((x) * SDL_PI_F / 180.0f)
#define degf( x ) ((x) * 180.0f / SDL_PI_F)

//
//  Global constants
//
#define FIXED_DT            (0.016666666f)
//#define FIXED_DT            (0.008333333f)

#define GRAVITY             (-9.81f)

#define FLOOR_PLANE         (0.0f)

#define THRUST_SET_MAX      (6u)

#define RUDDER_MAX          (127)
#define AILERON_MAX         (127)
#define ELEVATOR_MAX        (127)

#define F_AFTERBURNER       (1u << 0)
#define F_RECALC_TARGET_V   (1u << 1)
#define F_GEAR_DOWN         (1u << 2)
#define F_STALL_WARN        (1u << 3)
#define F_STALL             (1u << 4)

#define IN_INC_THRUST       (1u << 4)
#define IN_DEC_THRUST       (1u << 5)
#define IN_RUDDER_L         (1u << 6)
#define IN_RUDDER_R         (1u << 7)

#define LAXIS_RIGHT         (0u)
#define LAXIS_UP            (1u)
#define LAXIS_FORWARD       (2u)

typedef struct aircraft_s {
    obj3D_t*    model;
    tag         tag;

    // constants
    const f32   stdthrust_c[6];         // thrust coefficients per thrust setting
    const f32   stdlift_c[4];           // lift coefficients per flaps state
    const f32   drag_c[4];              // drag coefficients per flaps state
    f32         abthrust_c;             // afterburner thrust multiplier
    f32         thrustadjrate_c;
    u8          ctrlsurfspeed_c;
    // limits
    const f32   stallspeed_c[4];        // stall speeds per flaps state
    const f32   stallaoa_c[4];          // stall aoa (rad) per flaps state
    const f32   maxspeed_c[4];          // failure speeds per flaps state
    u16         maxtemp_c;              // failure engine temperature

    // aircraft state
    vec3f_t     pos;
    vec3f_t     v;
    quat_t      attitude;
    vec3f_t     local[3];               // local axes
    f32         lift;                   // current lift
    f32         aoa;                    // angle of attack
    f32         airspeed;
    f32         hspeed;                 // ground (horizontal) speed
    f32         vspeed;                 // vertical speed
    u8          thrustset;              // thrust setting
    f32         thrust;                 // actual thrust
    i16         ailerons;
    i16         rudder;
    i16         elevator;
    i16         elevatortrim;
    u16         engtemp;                // engine temperature
    u8          flaps;                  // flaps state
    u32         ctrlflags;              // various state flags
    
} aircraft_t;

extern aircraft_t jet_1;

void jetHandleThrustSetting(aircraft_t* jet, instate_t* input);
void jetHandleCtrlSurfaceInput(aircraft_t* restrict jet, const instate_t* restrict input);

void jetGetLocalAxes(aircraft_t* jet);

void jetComputeThrust(aircraft_t* jet, f32 dt);
void jetHandleLift(aircraft_t* jet, f32 dt);

void jetUpdatePosition(aircraft_t* jet, f32 dt);

void jetPrintDebugVals(const aircraft_t* jet, f32 t);

#endif