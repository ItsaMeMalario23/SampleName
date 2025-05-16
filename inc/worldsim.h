#ifndef WORLDSIM_H
#define WORLDSIM_H

#include <main.h>
#include <input.h>
#include <animation.h>

#define ASCII_CHAR_BUF_SIZE     4096
#define ASCII_OBJ_BUF_SIZE      128

#define SCENE_RENDER_ALL        (1u << 0)       /* only has effect on non-layered objects */

#define MOVE_PLAYER_X           (0.144)
#define MOVE_PLAYER_Y           (0.256)
#define MOVE_SCREEN_X           (0.144)
#define MOVE_SCREEN_Y           (0.256)

typedef struct ascii2info_s {
    u32     charID;
    u32     color;
    vec2f_t pos;
} ascii2info_t;

typedef struct animinfo_s {
    u32 type;
    u32 ticks;              /* for callback animations */
    u32 numframes;          /* for static animations */
    u32 flags;
    union {
        const animframe_t* staticf;
        animate_f callback;
    };
} animinfo_t;

typedef struct objectinfo_s {
    const ascii2info_t* data;
    const animinfo_t*   animations;
    u32                 len;
    u32                 numanimations;
    f32                 x, y;
    f32                 xscale;
    f32                 yscale;
    f32                 hitbox_dx;
    f32                 hitbox_dy;
} objectinfo_t;

typedef struct sceneinfo_s {
    inputmap_f          kbmapping;
    u32                 rendermode;
    u32                 numobjects;
    u64                 flags;
    f32                 player_x, player_y; /* player spawnpoint */
    const objectinfo_t* objects;
    const objectinfo_t* player;
    const i8*           layers;             /* for layer mode, must point to numobjects+1 long array */
} sceneinfo_t;

gameobj_t* getObjectBuf(void);
gameobj_t* getObject(u32 id);

void loadScene(const sceneinfo_t* restrict scene);
void resetScene(void);

void movePlayer(f64 dt);

#endif