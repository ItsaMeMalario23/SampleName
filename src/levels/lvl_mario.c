#include <levels.h>
#include <input.h>
#include <objects.h>
#include <worldsim.h>
#include <render/render.h>
#include <debug/rdebug.h>

#define MARIO_FIXED_DT          (0.01666666f)     /* run mario logic at 60 hz */

#define MOVE_MARIO_X            (0.00666666f)
#define MARIO_JUMP_STRENGTH     (0.04)
#define MARIO_JUMP_DECAY        (0.89f)
#define MARIO_MOVE_DECAY        (0.9f)
#define MARIO_FLOOR_PLANE       (0.412f)
#define MARIO_GRAVITY           (-0.04f)

static instate_t* input;
static gameobj_t* player;

static u32 jmp = 0;

//
//  Local functions
//
static inline void updateMarioPosition(bool dyreset)
{
    const asciidata_t* const bound = player->data + player->len;
    for (asciidata_t* i = player->data; i < bound; i++) {
        i->x += player->dx;
        i->y += player->dy;
    }

    player->x += player->dx;
    player->y += player->dy;
    
    player->dx *= MARIO_MOVE_DECAY;

    if ((player->dx > 0.0f && player->dx < 0.0042f) || (player->dx < 0.0f && player->dx > -0.0042f))
        player->dx = 0.0f;

    if (dyreset)
        player->dy = 0.0f;
}

static inline void handleMarioGravity(f64 t)
{
    if (player->y <= MARIO_FLOOR_PLANE)
        return;

    if (player->dy > 0.0f)          // jumping
    {
        player->dy *= MARIO_JUMP_DECAY;

        if (player->dy < 0.005f)
            player->dy = 0.0f;
    }
    else if (player->dy <= 0.0f)    // falling
    {
        player->dy += MARIO_GRAVITY * t;
    }
}

static inline bool handleMarioCollision(void)
{
    // clamp mario to floor plane
    if (player->y + player->dy < MARIO_FLOOR_PLANE) {
        player->dy = MARIO_FLOOR_PLANE - player->y;
        jmp = 0;

        return true;
    }

    return false;
}

//
//  Level init
//
static u32 init(void* restrict data)
{
    input = getInputState();

    loadScene(&scene_mario, &player);

    rAssert(player);

    return 0;
}

//
//  Level update
//
static u32 update(f64 dt)
{
    static f64 t;

    if ((t += dt) < MARIO_FIXED_DT)
        return LEVEL_CONTINUE;

    handleMarioGravity(t);

    if (input->left)
        player->dx = -MOVE_MARIO_X;

    if (input->right)
        player->dx = MOVE_MARIO_X;

    if (input->up && !jmp) {
        player->dy = MARIO_JUMP_STRENGTH;
        jmp = 1;
    }
    
    t = 0.0f;

    updateMarioPosition(handleMarioCollision());

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
    SDL_Log("[INFO] Mario level exit");
}

level_t lvl_mario = { "Mario", init, update, lvlexit, 0, RENDER_MODE_2D };