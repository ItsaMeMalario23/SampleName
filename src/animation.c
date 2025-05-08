#include <SDL3/SDL.h>

#include <animation.h>
#include <render/render2D.h>
#include <debug/rdebug.h>
#include <debug/memtrack.h>

static SDL_Mutex* lock;
static animation_t animbuf[ANIM_BUF_SIZE];

static SDL_Thread* thread;
static u32 stop;

animation_t* addAnimation(gameobj_t* restrict obj, const animframe_t* restrict animation, u16 frames, u16 reset, i32 cycles)
{
    rAssert(obj);
    rAssert(obj->len);
    rAssert(obj->data);
    rAssert(animation);
    rAssert(frames);

    if (!lock) {
        SDL_Log("Failed to add animation, animation buf lock null");
        return NULL;
    }

    animation_t* anim = NULL;

    SDL_LockMutex(lock);

    for (animation_t* i = animbuf; i < animbuf + ANIM_BUF_SIZE; i++) {
        if (!i->object) {
            anim = i;
            break;
        }
    }

    if (!anim) {
        SDL_Log("Failed to add animation, buffer full");
        return NULL;
    }

    anim->numframes = frames;
    anim->numcycles = cycles;
    anim->frames = animation;
    anim->object = obj;
    anim->timer = animation->duration;
    anim->current = 0;

    // save original char positions
    anim->stdpos = (vec2f_t*) memAlloc(sizeof(vec2f_t) * obj->len);

    vec2f_t* k = anim->stdpos;

    for (asciidata_t* i = obj->data; i < obj->data + obj->len; i++, k++) {
        k->x = i->x;
        k->y = i->y;
    }

    SDL_UnlockMutex(lock);

    return anim;
}

static void vec2ToAsciiBuf(const vec2f_t* restrict pos, asciidata_t* restrict data, u32 len)
{
    for (const vec2f_t* i = pos; i < pos + len; i++, data++) {
        data->x = i->x;
        data->y = i->y;
    }
}

static int animationThread(void* restrict ptr)
{
    f64 dt, frq = (f64) SDL_GetPerformanceFrequency();
    u64 t, prev = SDL_GetPerformanceCounter();

    for (;;) {
        if (stop)
            break;

        // await animation tick
        for (;;) {
            t = SDL_GetPerformanceCounter();
            dt = (((f64) t - prev) / frq);

            if (dt >= ANIM_TICK_DURATION)
                break;
        }

        SDL_LockMutex(lock);

        for (animation_t* i = animbuf; i < animbuf + ANIM_BUF_SIZE; i++) {
            if (!i->object)
                continue;

            // reset or destroy animation
            if (i->current >= i->numframes) {
                if (i->numcycles == -1)
                {
                    i->current = 0;
                    i->timer = i->frames->duration;

                    if (i->reset)
                        vec2ToAsciiBuf(i->stdpos, i->object->data, i->object->len);
                }
                else if (--(i->numcycles) == 0)
                {
                    if (i->reset)
                        vec2ToAsciiBuf(i->stdpos, i->object->data, i->object->len);

                    memFree(i->stdpos);

                    memset(i, 0, sizeof(animation_t));

                    continue;
                }
                else
                {
                    i->current = 0;
                    i->timer = i->frames->duration;
                }
            }

            if (i->timer) {
                i->timer--;
                continue;
            }

            // animate if timer is 0
            SDL_LockMutex(renderBufLock);

            for (u32 k = 0; k < i->frames[i->current].len; k++) {
                i->object->data[k].x += i->frames[i->current].pos[k].x;
                i->object->data[k].y += i->frames[i->current].pos[k].y;
            }

            SDL_UnlockMutex(renderBufLock);

            // increment frame
            if (++(i->current) < i->numframes)
                i->timer = i->frames[i->current].duration;
        }

        SDL_UnlockMutex(lock);

        prev = t;
    }

    SDL_Log("Animation thread terminated");

    return 0;
}

void initAnimationThread(void)
{
    stop = 0;

    lock = SDL_CreateMutex();

    if (!lock) {
        SDL_Log("Failed to create animation buf mutex");
        return;
    }

    memset(animbuf, 0, sizeof(animation_t) * ANIM_BUF_SIZE);

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
}

