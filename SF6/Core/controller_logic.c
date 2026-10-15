#include "controller_logic.h"
#include "input_bindings.h"
#include "action_player.h"

typedef struct
{
    uint8_t pressed;
    uint8_t prev;
    uint16_t hold_frames;
} KeyState;

KeyState super_key = {0};
uint8_t facing_left = 0;

static void UpdateKey(KeyState *k, uint8_t is_pressed)
{
    k->prev = k->pressed;
    k->pressed = is_pressed;
    if (k->pressed)
        k->hold_frames++;
    else
        k->hold_frames = 0;
}

/* The special-move key is reserved until its move table is specified. */
int test_flag = 10;

OUT_STATE Logic_Process(IN_STATE in)
{
    OUT_STATE out = 0;
    uint8_t dir = 3;

    UpdateKey(&super_key, (in & BIND_SUPER) ? 1 : 0);

    /* Keep selection priority: down, left, right, then unspecified. */
    if (in & BIND_DOWN)
        dir = 0;
    else if (in & BIND_LEFT)
        dir = 1;
    else if (in & BIND_RIGHT)
        dir = 2;
    if (in & BIND_SUPER)
    {
        test_flag = 1;
    }
    else
    {
        test_flag = 0;
    }

    if (!super_key.prev && super_key.pressed)
    {
        if (!(in & BIND_DIRECTIONS))
            facing_left ^= 1;
        else if (dir != 3)
            EnqueueAction(ACT_SUPER, dir, facing_left);
    }

    /* A macro owns the entire output frame, even when its output is zero. */
    if (ActionPlayer_Process(&out))
        return out;

    /* Ordinary inputs are never mirrored. */
    if (in & BIND_UP)
        out |= OUT_UP;
    if (in & BIND_DOWN)
        out |= OUT_DOWN;
    if (in & BIND_LEFT)
        out |= OUT_LEFT;
    if (in & BIND_RIGHT)
        out |= OUT_RIGHT;
    if (in & IN_LP)
        out |= OUT_LP;
    if (in & IN_MP)
        out |= OUT_MP;
    if (in & IN_HP)
        out |= OUT_HP;
    if (in & IN_LK)
        out |= OUT_LK;
    if (in & IN_MK)
        out |= OUT_MK;
    if (in & IN_HK)
        out |= OUT_HK;
    if (in & BIND_MEDIUM_PAIR)
        out |= OUT_MP | OUT_MK;
    return out;
}
