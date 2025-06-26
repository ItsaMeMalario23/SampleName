#ifndef OBJECTS_H
#define OBJECTS_H

#include <main.h>
#include <worldsim.h>
#include <render/camera.h>

#define BIRD_X_SCALE    (0.1f)
#define BIRD_Y_SCALE    (0.13333333f)

#define CHERRY_X_SCALE  (0.06f)
#define CHERRY_Y_SCALE  (0.10f)

#define FLAG_Y_OFFSET   (0.006f)

extern const sceneinfo_t scene_bird;
extern const sceneinfo_t scene_mario;

typedef enum tag_e {
    TAG_UNDEF,
    TAG_PLAYER,
    TAG_WALL,
    TAG_JET
} tag;

extern obj3D_t* pyramid_3D;
extern obj3D_t* terrain_3D;
extern obj3D_t* wall_3D;

extern obj3D_t objects3D[];

extern const f32 wall_vtx_uv[30];

obj3D_t* getObjectByTag3D(tag objtag);

vec3f_t* parseStl(const char* filename, u32* numvtx, u32 color);

#endif