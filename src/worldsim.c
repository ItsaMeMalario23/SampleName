#include <worldsim.h>
#include <levels.h>
#include <render/render.h>
#include <debug/rdebug.h>

static asciidata_t charbuf[ASCII_CHAR_BUF_SIZE];
static const asciidata_t* charbound = charbuf + ASCII_CHAR_BUF_SIZE;

static gameobj_t objbuf[ASCII_OBJ_BUF_SIZE];
static const gameobj_t* objbound = objbuf + ASCII_OBJ_BUF_SIZE;

static u32 numchars;
static u32 numobjects;

static level_t* currentlvl;
static u32 currentlvlid;

//
//  Public functions
//
gameobj_t* getObjectBuf(void)
{
    return objbuf;
}

gameobj_t* getObject(u32 id)
{
    if (id >= numobjects) {
        SDL_Log("[ERROR] Get object id out of bounds");
        return NULL;
    }

    return objbuf + id;
}

// allocate and init 2D ascii object
i32 initGameObject(const objectinfo_t* info, bool render)
{
    rAssert(info);
    rAssert(info->len);
    rAssert(info->data);

    if (objbuf + numobjects >= objbound) {
        SDL_Log("[ERROR] Failed to add game object, object buffer full");
        return -1;
    }

    if (charbuf + info->len >= charbound) {
        SDL_Log("[ERROR] Failed to add game object, char buffer full");
        return -1;
    }

    gameobj_t* obj = objbuf + numobjects;

    if (render)
        rAddObjectToRenderBuf(numobjects);

    obj->visible = 1;
    obj->animation = NULL;
    obj->id = numobjects;
    obj->len = info->len;
    obj->x = info->x;
    obj->y = info->y;
    obj->xscale = info->xscale;
    obj->yscale = info->yscale;
    obj->hitbox_dx = info->hitbox_dx;
    obj->hitbox_dy = info->hitbox_dy;
    obj->data = charbuf + numchars;

    numchars += info->len;
    
    const ascii2info_t* dat = info->data;
    for (asciidata_t* i = obj->data; i < obj->data + obj->len; i++, dat++)
    {
        i->r = (f32) ((u8) (dat->color >> 24)) / 255.0f;
        i->g = (f32) ((u8) (dat->color >> 16)) / 255.0f;
        i->b = (f32) ((u8) (dat->color >> 8)) / 255.0f;
        i->a = (f32) ((u8) dat->color) / 255.0f;
        i->x = dat->pos.x + info->x;
        i->y = dat->pos.y + info->y;
        i->scale = 1.0f / 80.0f;

        if (dat->charID < 33)
            i->charID = 0;
        else
            i->charID = dat->charID - 32;
    }

    SDL_Log("[INFO] Allocated object of length %ld", obj->len);

    return numobjects++;
}

// should only be called by level implementations
void loadScene(const sceneinfo_t* scene, gameobj_t** restrict player)
{
    rAssert(scene);
    rAssert(scene->kbmapping || scene->flags & SCENE_NO_INPUT_MAP);
    rAssert(scene->player);
    rAssert(!scene->numobjects || scene->objects);
    rAssert((!(scene->rendermode & RENDER_MODE_LAYERED) || scene->layers) || scene->flags & SCENE_NO_RENDERMODE);

    resetScene();
    renderResetBuffers();
    resetAnimationQueue();

    if (!(scene->flags & SCENE_NO_RENDERMODE))
        renderMode(scene->rendermode);

    if (!(scene->flags & SCENE_NO_INPUT_MAP))
        scene->kbmapping();

    // add player object
    if (initGameObject(scene->player, scene->flags & SCENE_RENDER_ALL) < 0)
        return;

    gameobj_t* playerobj = objbuf;

    for (asciidata_t* i = playerobj->data; i < playerobj->data + playerobj->len; i++) {
        i->x += scene->player_x;
        i->y += scene->player_y;
    }

    playerobj->dx = 0.0f;
    playerobj->dy = 0.0f;
    playerobj->x = scene->player_x;
    playerobj->y = scene->player_y;

    if (scene->flags & SCENE_PLAYER_HITBOX)
        rAddHitbox(playerobj);

    // add player animations
    for (const animinfo_t* i = scene->player->animations; i < scene->player->animations + scene->player->numanimations; i++) {
        if (i->type == ANIM_TYPE_STATIC)
            addStaticAnimation(playerobj, i->staticf, i->numframes, i->flags);
        else if (i->type == ANIM_TYPE_CALLBACK)
            addCallbackAnimation(playerobj, i->callback, i->ticks, i->flags);
        else
            SDL_Log("[ERROR] Invalid animation type in scene: %d", i->type);
    }

    // add other objects
    for (const objectinfo_t* i = scene->objects; i < scene->objects + scene->numobjects; i++) {
        i32 id = initGameObject(i, scene->flags & SCENE_RENDER_ALL);

        if (id < 0)
            return;

        // add object animations
        for (const animinfo_t* k = i->animations; k < i->animations + i->numanimations; k++) {
            if (k->type == ANIM_TYPE_STATIC)
                addStaticAnimation(objbuf + id, k->staticf, k->numframes, k->flags);
            else if (k->type == ANIM_TYPE_CALLBACK)
                addCallbackAnimation(objbuf + id, k->callback, k->ticks, k->flags);
            else
                SDL_Log("[ERROR] Invalid animation type in scene: %d", k->type);
        }
    }

    // setup render layers
    if (scene->rendermode & RENDER_MODE_LAYERED) {
        for (u32 i = 0; i < scene->numobjects; i++) {
            if (scene->layers[i] < 0)
                continue;

            rAddObjectToLayer(i + 1, scene->layers[i]);
        }

        if (scene->layers[scene->numobjects] > -1)
            rAddObjectToLayer(0, scene->layers[scene->numobjects]);
    }

    *player = playerobj;
}

void resetScene(void)
{
    numobjects = 0;
    numchars = 0;

    memset(objbuf, 0, sizeof(objbuf));
}

u32 startLevel(u32 levelid)
{
    if (currentlvl)
        currentlvl->exit();

    currentlvlid = levelid;

    switch (levelid) {
    case LVL_ID_BIRD:
        currentlvl = &lvl_bird;
        break;

    case LVL_ID_MARIO:
        currentlvl = &lvl_mario;
        break;

    case LVL_ID_ODYSSEY:
        currentlvl = &lvl_odyssey;
        break;

    case LVL_ID_AIR:
        currentlvl = &lvl_air;
        break;

    default:
        SDL_Log("[ERROR] Invalid level ID");
        return 1;
    }

    renderMode(currentlvl->rendermode);

    if (currentlvl->init(NULL)) {
        SDL_Log("[ERROR] Failed to initialize level");
        return 1;
    }

    return 0;
}

static inline u32 getNextLevel(u32 levelid, u32 exitid)
{
    switch (levelid) {
    case LVL_ID_BIRD:
        return LVL_ID_MARIO;

    case LVL_ID_MARIO:
        return LVL_ID_ODYSSEY;

    case LVL_ID_ODYSSEY:
        return LVL_ID_BIRD;
    }

    return 0xffffffff;
}

u32 gameUpdate(f64 dt)
{
    rAssert(currentlvl);
    rAssert(currentlvl->update);

    u32 ret;
    
    if (ret = currentlvl->update(dt)) {
        if (ret > 1 && ret < 6) {
            if (startLevel(currentlvl->exits[ret - 2]))
                return 1;
        }
    }

    return 0;
}