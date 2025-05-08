#ifndef ANIMATION_H
#define ANIMATION_H

#include <main.h>

#define ANIM_BUF_SIZE       8
#define ANIM_TICK_DURATION (0.04f)

#define ANIM_REPEAT     -1
#define ANIM_SUSPENDED  -2

typedef struct animframe_s {
    vec2f_t* pos;
    u32      duration;
    u32      len;
} animframe_t;

typedef struct animation_s {
    const animframe_t* frames;
    gameobj_t* object;
    vec2f_t* stdpos;
    i32 numcycles;
    u16 numframes;
    u16 current;
    u32 timer;
    u16 reset;
    u16 keep;
} animation_t;

animation_t* addAnimation(gameobj_t* restrict obj, const animframe_t* restrict animation, u16 frames, u16 reset, i32 cycles);

void initAnimationThread(void);
void cleanupAnimationThread(void);

#endif