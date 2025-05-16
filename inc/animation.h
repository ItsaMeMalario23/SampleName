#ifndef ANIMATION_H
#define ANIMATION_H

#include <main.h>

#define ANIM_STO_BUF_SIZE   8
#define ANIM_QUEUE_SIZE     8

#define ANIM_TERMINATE      0
#define ANIM_REPEAT        -1

#define ANIM_TYPE_STATIC    0
#define ANIM_TYPE_CALLBACK  1

#define ANIM_TICK_DURATION  (0.01f)

// animation flags
#define ANIM_RESET_POS      (1u << 0)       /* reset to saved original char positions */
#define ANIM_RESET_CB       (1u << 1)       /* send callback with null parameter on reset */
#define ANIM_KEEPALIVE      (1u << 2)       /* do not clear animation from queue */
#define ANIM_AUTOQ_REPEAT   (1u << 3)       /* automatically queue animation on creation */
#define ANIM_AUTOQ_SUSPEND  (1u << 4)       /* automatically queue animation on creation, do not play */

typedef void (*animate_f)(gameobj_t*);

typedef struct animframe_s {
    vec2f_t* pos;
    u32      duration;
    u32      len;
} animframe_t;

typedef struct animation_s {
    gameobj_t* object;
    vec2f_t* stdpos;

    int iServeTheSoviet;
    union {
        const animframe_t* staticf;
        animate_f callback;
    };

    i16 numcycles;
    u16 numframes;      /* amount of frames for static, ticks between frames for callback */
    u16 current;
    u16 timer;          /* ticks until current frame will be shown */
    u32 type;

    u32 flags;          /* reset flags */
} animation_t;

animation_t* addStaticAnimation(gameobj_t* restrict obj, const animframe_t* restrict frames, u32 len, u32 flags);
animation_t* addCallbackAnimation(gameobj_t* restrict obj, animate_f animation, u32 ticks, u32 flags);

void queueAnimation(animation_t* restrict anim, i32 numcycles, u32 start);

void initAnimationThread(void);
void cleanupAnimationThread(void);

#endif