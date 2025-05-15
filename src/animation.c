#include <SDL3/SDL.h>

#include <animation.h>
#include <render/renderer.h>
#include <debug/rdebug.h>
#include <debug/memtrack.h>

static SDL_Mutex* lock;
static animation_t animbuf[ANIM_STO_BUF_SIZE];

static SDL_Mutex* qlock;
static u32 numqueued;
static animation_t* animqueue[ANIM_QUEUE_SIZE];

static SDL_Thread* thread;
static u32 stop;

//
//  Local functions
//

// allocate animation object in buf
static animation_t* getAnimMem(gameobj_t* restrict obj)
{
    rAssert(obj);
    rAssert(obj->len);
    rAssert(obj->data);

    if (!lock) {
        SDL_Log("Failed to add animation, animation buf lock null ref");
        return NULL;
    }

    for (animation_t* i = animbuf; i < animbuf + ANIM_STO_BUF_SIZE; i++) {
        if (!i->object) {
            i->object = obj;
            return i;
        }
    }

    SDL_Log("Failed to add animation, animation buf full");

    return NULL;
}

// reset char positions to saved
static void resetCharPos(const vec2f_t* restrict pos, asciidata_t* restrict data, u32 len, f32 dx, f32 dy)
{
    rAssert(pos);
    rAssert(data);

    for (const vec2f_t* i = pos; i < pos + len; i++, data++) {
        data->x = i->x + dx;
        data->y = i->y + dy;
    }
}

// save original char positions
static void saveCharPos(animation_t* restrict anim)
{
    rAssert(anim);
    rAssert(anim->object);
    rAssert(anim->object->len);
    rAssert(anim->object->data);
    rAssert(anim->reset & ANIM_RESET_POS);

    anim->stdpos = (vec2f_t*) memAlloc(sizeof(vec2f_t) * anim->object->len);

    vec2f_t* k = anim->stdpos;

    for (asciidata_t* i = anim->object->data; i < anim->object->data + anim->object->len; i++, k++) {
        k->x = i->x - anim->object->x;
        k->y = i->y - anim->object->y;
    }
}

// animation thread implementation
static int animationThread(void* restrict ptr)
{
    f64 dt, frq = (f64) SDL_GetPerformanceFrequency();
    u64 t = 0, prev = SDL_GetPerformanceCounter();

    for (;;) {
        if (stop)
            break;

        if (t)
            prev = t;

        // await animation tick
        for (;;) {
            t = SDL_GetPerformanceCounter();
            dt = (((f64) t - prev) / frq);

            if (dt >= ANIM_TICK_DURATION)
                break;
        }

        if (!numqueued)
            continue;

        SDL_LockMutex(qlock);
        SDL_LockMutex(lock);

        for (u32 i = 0; i < ANIM_QUEUE_SIZE; i++) {
            if (!animqueue[i])
                continue;

            rAssert(animqueue[i]->object);
            rAssert(animqueue[i]->animate.staticf);

            if (animqueue[i]->numcycles == 0)
            {
                if (animqueue[i]->reset & ANIM_RESET_POS) {
                    resetCharPos
                    (
                        animqueue[i]->stdpos,
                        animqueue[i]->object->data,
                        animqueue[i]->object->len,
                        animqueue[i]->object->x,
                        animqueue[i]->object->y
                    );
                }

                // reset callback if flag is set
                if (animqueue[i]->reset & ANIM_RESET_CB && animqueue[i]->type == ANIM_TYPE_CALLBACK)
                    animqueue[i]->animate.callback(NULL);

                // do not clear animation if keepalive flag is set
                if (animqueue[i]->reset & ANIM_KEEPALIVE)
                    continue;

                // clear animation from queue if no more cycles
                animqueue[i] = NULL;
                numqueued--;
            }
            else if (animqueue[i]->type == ANIM_TYPE_CALLBACK)
            {
                if (animqueue[i]->timer) {
                    animqueue[i]->timer--;
                    continue;
                }

                // animate if timer is 0
                SDL_LockMutex(renderBufLock);

                animqueue[i]->animate.callback(animqueue[i]->object);

                SDL_UnlockMutex(renderBufLock);

                // set duration until next frame
                animqueue[i]->timer = animqueue[i]->numframes;

                if (animqueue[i]->numcycles > 0)
                    animqueue[i]->numcycles--;
            }
            else if (animqueue[i]->type == ANIM_TYPE_STATIC)
            {
                // reset frame to first
                if (animqueue[i]->current >= animqueue[i]->numframes) {
                    animqueue[i]->current = 0;
                    animqueue[i]->timer = animqueue[i]->animate.staticf->duration;

                    if (animqueue[i]->reset & ANIM_RESET_POS) {
                        resetCharPos
                        (
                            animqueue[i]->stdpos,
                            animqueue[i]->object->data,
                            animqueue[i]->object->len,
                            animqueue[i]->object->x,
                            animqueue[i]->object->y
                        );
                    }

                    if (animqueue[i]->numcycles > 0)
                        animqueue[i]->numcycles--;
                }

                if (animqueue[i]->timer) {
                    animqueue[i]->timer--;
                    continue;
                }

                // animate if timer is 0
                SDL_LockMutex(renderBufLock);

                for (u32 k = 0; k < animqueue[i]->animate.staticf[animqueue[i]->current].len; k++) {
                    animqueue[i]->object->data[k].x += animqueue[i]->animate.staticf[animqueue[i]->current].pos[k].x;
                    animqueue[i]->object->data[k].y += animqueue[i]->animate.staticf[animqueue[i]->current].pos[k].y;
                }

                SDL_UnlockMutex(renderBufLock);

                // set duration until next frame
                if (++(animqueue[i]->current) < animqueue[i]->numframes)
                    animqueue[i]->timer = animqueue[i]->animate.staticf[animqueue[i]->current].duration;
            }
            else
            {
                SDL_Log("Invalid animation type");
            }
        }

        SDL_UnlockMutex(qlock);
        SDL_UnlockMutex(lock);
    }

    SDL_Log("Animation thread terminated");

    return 0;
}

//
//  Public functions
//

animation_t* addStaticAnimation(gameobj_t* restrict obj, const animframe_t* restrict frames, u32 len, u32 reset)
{
    rAssert(len);
    rAssert(frames);

    SDL_LockMutex(lock);

    animation_t* anim = getAnimMem(obj);

    if (!anim)
        return NULL;

    anim->type = ANIM_TYPE_STATIC;
    anim->numcycles = 0;
    anim->numframes = len;
    anim->reset = reset;

    anim->animate.staticf = frames;

    if (reset & ANIM_RESET_POS)
        saveCharPos(anim);

    SDL_UnlockMutex(lock);

    return anim;
}

animation_t* addCallbackAnimation(gameobj_t* restrict obj, animate_f animation, u32 ticks, u32 reset)
{
    rAssert(animation);

    SDL_LockMutex(lock);

    animation_t* anim = getAnimMem(obj);

    if (!anim)
        return NULL;

    anim->type = ANIM_TYPE_CALLBACK;
    anim->numcycles = 0;
    anim->numframes = ticks;
    anim->reset = reset;

    anim->animate.callback = animation;

    if (reset & ANIM_RESET_POS)
        saveCharPos(anim);

    SDL_UnlockMutex(lock);

    return anim;
}

void queueAnimation(animation_t* restrict anim, i32 numcycles, u32 start)
{
    rAssert(anim);
    rAssert(anim->object);
    rAssert(anim->type == ANIM_TYPE_STATIC || anim->type == ANIM_TYPE_CALLBACK);

    if (numqueued >= ANIM_QUEUE_SIZE) {
        SDL_Log("Failed to queue animation, queue full");
        return;
    }

    anim->numcycles = numcycles;

    if (anim->type == ANIM_TYPE_STATIC) {
        if (start < anim->numframes)
            anim->current = start;
        else
            anim->current = 0;

        anim->timer = anim->animate.staticf[anim->current].duration;
    } else if (anim->type == ANIM_TYPE_CALLBACK) {
        anim->timer = anim->numframes;
    } else {
        return;
    }

    SDL_LockMutex(qlock);

    for (u32 i = 0; i < ANIM_QUEUE_SIZE; i++) {
        if (!animqueue[i]) {
            animqueue[i] = anim;
            numqueued++;
            break;
        }
    }

    SDL_UnlockMutex(qlock);
}

void initAnimationThread(void)
{
    stop = 0;

    lock = SDL_CreateMutex();
    qlock = SDL_CreateMutex();

    if (!lock) {
        SDL_Log("Failed to create animation buf mutex");
        return;
    }

    memset(animbuf, 0, sizeof(animation_t) * ANIM_STO_BUF_SIZE);

    thread = SDL_CreateThread(animationThread, "animation thread", NULL);

    if (!thread)
        SDL_Log("Failed to create animation thread");

    SDL_DetachThread(thread);

    SDL_Log("Animation thread started");
}

void cleanupAnimationThread(void)
{
    stop = 1;

    SDL_DestroyMutex(lock);
    SDL_DestroyMutex(qlock);
}

