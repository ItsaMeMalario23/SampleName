#include <levels.h>
#include <input.h>
#include <objects.h>
#include <animation.h>
#include <worldsim.h>
#include <render/render.h>
#include <debug/rdebug.h>

#define MOVE_BIRD_X             (0.144)
#define MOVE_BIRD_Y             (0.256)
#define MOVE_SCREEN_X           (0.144)
#define MOVE_SCREEN_Y           (0.256)

static instate_t* input;
static gameobj_t* player;
static const asciidata_t* bound;

static inline void updateBirdPosition(void)
{
    for (asciidata_t* i = player->data; i < bound; i++) {
        i->x += player->dx;
        i->y += player->dy;
    }

    player->x += player->dx;
    player->y += player->dy;
    player->dx = 0.0f;
    player->dy = 0.0f;
}

//
//  Level init
//
static u32 init(void* restrict data)
{
    input = getInputState();
    input->screen_dx = 0.0f;
    input->screen_dy = -0.14f;

    loadScene(&scene_bird, &player);

    rAssert(player);
    rAssert(player->animation);

    bound = player->data + player->len;

    return 0;
}

//
//  Level update
//
static u32 update(f64 dt)
{
    if (input->up)
        player->dy += MOVE_BIRD_Y * dt;

    if (input->down)
        player->dy -= MOVE_BIRD_Y * dt;

    if (input->left) {
        input->screen_dx += MOVE_SCREEN_X * dt;
        player->dx -= MOVE_BIRD_X * dt;
    }

    if (input->right) {
        input->screen_dx -= MOVE_SCREEN_X * dt;
        player->dx += MOVE_BIRD_X * dt;
    }

    if (fnotzero(player->dx) || fnotzero(player->dy)) {
        player->animation->numcycles = ANIM_REPEAT;
        updateBirdPosition();
    } else {
        player->animation->numcycles = ANIM_TERMINATE;
    }

    if (input->dynamic) {
        input->dynamic = 0;
        return LEVEL_EXIT_1;
    }

    return LEVEL_CONTINUE;
}

//
//  Level exit
//
static void lvlexit(void)
{
    SDL_Log("[INFO] Bird level exit");
}

level_t lvl_bird = { "Bird", init, update, lvlexit, 0, RENDER_MODE_LAYERED };