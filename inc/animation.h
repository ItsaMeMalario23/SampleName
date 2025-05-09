#ifndef ANIMATION_H
#define ANIMATION_H

#include <main.h>

#define ANIM_STO_BUF_SIZE       8
#define ANIM_QUEUE_SIZE         8

#define ANIM_TICK_DURATION  (0.04f)

#define ANIM_TERMINATE   0
#define ANIM_REPEAT     -1

#define ANIM_TYPE_STATIC    0
#define ANIM_TYPE_CALLBACK  1

// animation reset flags
#define ANIM_RESET_POS      (1u << 0)
#define ANIM_RESET_CB       (1u << 1)
#define ANIM_KEEPALIVE      (1u << 2)

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
    } animate;

    i16 numcycles;
    u16 numframes;
    u16 current;
    u16 timer;          /* ticks until current frame will be shown */
    u32 type;

    u32 reset;          /* reset flags */
} animation_t;

animation_t* addStaticAnimation(gameobj_t* restrict obj, const animframe_t* restrict frames, u32 len, u32 reset);
animation_t* addCallbackAnimation(gameobj_t* restrict obj, animate_f animation, u32 ticks, u32 reset);

void queueAnimation(animation_t* restrict anim, i32 numcycles, u32 start);

void initAnimationThread(void);
void cleanupAnimationThread(void);

#endif