#ifndef OBJECTS_H
#define OBJECTS_H

#include <main.h>
#include <worldsim.h>

#define BIRD_X_SCALE    (0.1f)
#define BIRD_Y_SCALE    (0.13333333f)

#define CHERRY_X_SCALE  (0.06f)
#define CHERRY_Y_SCALE  (0.10f)

#define FLAG_Y_OFFSET   (0.006f)

typedef enum tag_e {
    TAG_UNDEF,
    TAG_PLAYER,
    TAG_WALL,
    TAG_WALL2,
    TAG_JET,
    TAG_TERRAIN
} tag;

typedef struct obj3D_s {
    const vec3f_t*  vtxbuf;
    mat4_t          transform;
    vec3f_t         pos;
    u32             vtxbufoffset;
    u32             numvtx;
    f32             pitch;
    f32             yaw;
    f32             roll;
    u32             flags;
    u64             tag;
} obj3D_t;

extern const sceneinfo_t scene_bird;
extern const sceneinfo_t scene_mario;

extern obj3D_t* scene3D_1;
extern obj3D_t* scene3D_2;

extern const f32 wall_vtx_uv[30];
extern const f32 wall2_vtx_uv[120];
extern const mat4_t wall2_transform;

obj3D_t* getObjectByTag3D(tag objtag, obj3D_t* scene);

vec3f_t* parseStl(const char* filename, u32* numvtx, u32 color);

void init3DScenes(void);
void objectsCleanup(void);

#endif