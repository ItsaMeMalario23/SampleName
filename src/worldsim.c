#include <worldsim.h>
#include <render/renderer.h>
#include <debug/rdebug.h>

static asciidata_t charbuf[ASCII_CHAR_BUF_SIZE];
static const asciidata_t* charbound = charbuf + ASCII_CHAR_BUF_SIZE;

static gameobj_t objbuf[ASCII_OBJ_BUF_SIZE];
static const gameobj_t* objbound = objbuf + ASCII_OBJ_BUF_SIZE;

static u32 numchars;
static u32 numobjects;

static gameobj_t* player;

static instate_t* input;

//
//  Local functions
//
static i32 addGameObject(const objectinfo_t* restrict info, bool render)
{
    rAssert(info);
    rAssert(info->len);
    rAssert(info->data);

    if (objbuf + numobjects >= objbound) {
        SDL_Log("ERROR: Failed to add game object, object buffer full");
        return -1;
    }

    if (charbuf + info->len >= charbound) {
        SDL_Log("ERROR: Failed to add game object, char buffer full");
        return -1;
    }

    gameobj_t* obj = objbuf + numobjects;

    if (render)
        addObjectToRenderBuf(numobjects);

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

    SDL_Log("INFO: Allocated object of length %ld", obj->len);

    return numobjects++;
}

static inline void updateObjectPosition(gameobj_t* restrict object)
{
    rAssert(object);

    for (asciidata_t* i = object->data; i < object->data + object->len; i++) {
        i->x += object->dx;
        i->y += object->dy;
    }

    object->x += object->dx;
    object->y += object->dy;
    object->dx = 0.0f;
    object->dy = 0.0f;
}

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
        SDL_Log("ERROR: Get object id out of bounds");
        return NULL;
    }

    return objbuf + id;
}

void loadScene(const sceneinfo_t* restrict scene)
{
    rAssert(scene);
    rAssert(scene->kbmapping);
    rAssert(scene->player);
    rAssert(!scene->numobjects || scene->objects);
    rAssert(!(scene->rendermode & RENDER_MODE_LAYERED) || scene->layers);

    resetScene();
    resetRenderBuffers();

    if (!input)
        input = getInputState();

    // add player object
    if (addGameObject(scene->player, scene->flags & SCENE_RENDER_ALL) < 0)
        return;

    player = objbuf;
    player->dx = scene->player_x;
    player->dy = scene->player_y;

    // add player animations
    for (const animinfo_t* i = scene->player->animations; i < scene->player->animations + scene->player->numanimations; i++) {
        if (i->type == ANIM_TYPE_STATIC)
            addStaticAnimation(player, i->staticf, i->numframes, i->flags);
        else if (i->type == ANIM_TYPE_CALLBACK)
            addCallbackAnimation(player, i->callback, i->ticks, i->flags);
        else
            SDL_Log("ERROR: Invalid animation type in scene: %d", i->type);
    }

    // add other objects
    for (const objectinfo_t* i = scene->objects; i < scene->objects + scene->numobjects; i++) {
        i32 id = addGameObject(i, scene->flags & SCENE_RENDER_ALL);

        if (id < 0)
            return;

        // add object animations
        for (const animinfo_t* k = i->animations; k < i->animations + i->numanimations; k++) {
            if (k->type == ANIM_TYPE_STATIC)
                addStaticAnimation(objbuf + id, k->staticf, k->numframes, k->flags);
            else if (k->type == ANIM_TYPE_CALLBACK)
                addCallbackAnimation(objbuf + id, k->callback, k->ticks, k->flags);
            else
                SDL_Log("ERROR: Invalid animation type in scene: %d", k->type);
        }
    }

    // setup render layers
    if (scene->rendermode & RENDER_MODE_LAYERED) {
        for (u32 i = 0; i < scene->numobjects; i++) {
            if (scene->layers[i] < 0)
                continue;

            addObjectToLayer(i + 1, scene->layers[i]);
        }

        if (scene->layers[scene->numobjects] > -1)
            addObjectToLayer(0, scene->layers[scene->numobjects]);
    }

    renderMode(scene->rendermode);

    scene->kbmapping();
}

void resetScene(void)
{
    player = NULL;
    numobjects = 0;
    numchars = 0;

    memset(objbuf, 0, sizeof(objbuf));
}

void movePlayer(f64 dt)
{
    rAssert(player);
    rAssert(input);

    if (input->up)
        player->dy += MOVE_PLAYER_Y * dt;

    if (input->down)
        player->dy -= MOVE_PLAYER_Y * dt;

    if (input->left) {
        input->screen_dx += MOVE_SCREEN_X * dt;
        player->dx -= MOVE_PLAYER_X * dt;
    }

    if (input->right) {
        input->screen_dx -= MOVE_SCREEN_X * dt;
        player->dx += MOVE_PLAYER_X * dt;
    }

    rAssert(player->animation);

    if (fnotzero(player->dx) || fnotzero(player->dy)) {
        player->animation->numcycles = ANIM_REPEAT;
        updateObjectPosition(player);
    } else {
        player->animation->numcycles = ANIM_TERMINATE;
    }
}