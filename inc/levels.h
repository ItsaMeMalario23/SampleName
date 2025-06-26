#ifndef LEVELS_H
#define LEVELS_H

#include <main.h>

#define LEVEL_CONTINUE      (0u)
#define LEVEL_PAUSE         (1u)
#define LEVEL_EXIT_1        (2u)
#define LEVEL_EXIT_2        (3u)
#define LEVEL_EXIT_3        (4u)
#define LEVEL_EXIT_4        (5u)

#define LVL_INIT_SUCCESS    (0u)
#define LVL_INIT_FAILURE    (1u)

typedef u32 (*levelInit_f)(void*);
typedef u32 (*levelUpdate_f)(f64);
typedef void (*levelExit_f)(void);

typedef struct level_s {
    const char*     name;
    levelInit_f     init;
    levelUpdate_f   update;
    levelExit_f     exit;
    u32             flags;
    u32             rendermode;
    u8              exits[8];
} level_t;

extern level_t lvl_mario;
extern level_t lvl_bird;
extern level_t lvl_odyssey;
extern level_t lvl_air;

#endif