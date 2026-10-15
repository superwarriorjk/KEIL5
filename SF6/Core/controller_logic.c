#include "controller_logic.h"
#include "input_bindings.h"
#include "action_player.h"

typedef struct
{
    uint8_t pressed;
    uint8_t prev;
    uint16_t hold_frames;
} KeyState;

KeyState sp1_key = {0};
KeyState sp2_key = {0};
KeyState sp3_key = {0};
KeyState sp4_key = {0};
KeyState super_key = {0};

static void UpdateKey(KeyState *k, uint8_t is_pressed)
{
    k->prev = k->pressed;
    k->pressed = is_pressed;
    if (k->pressed)
        k->hold_frames++;
    else
        k->hold_frames = 0;
}

/* SP1-SP4 key states remain dormant. Only SUPER is bound in this baseline.
 * HOLD_THRESHOLD and the legacy trigger branches are intentionally retained. */
int test_flag = 10;

OUT_STATE Logic_Process(IN_STATE in)
{
    OUT_STATE out = 0;
    uint8_t mirror = (in & BIND_MIRROR) ? 1 : 0;
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

    if (!sp1_key.prev && sp1_key.pressed)
        EnqueueAction((sp1_key.hold_frames >= HOLD_THRESHOLD) ? ACT_SP1_HOLD : ACT_SP1, dir);
    if (!sp2_key.prev && sp2_key.pressed)
        EnqueueAction((sp2_key.hold_frames >= HOLD_THRESHOLD) ? ACT_SP2_HOLD : ACT_SP2, dir);
    if (!sp3_key.prev && sp3_key.pressed)
        EnqueueAction((sp3_key.hold_frames >= HOLD_THRESHOLD) ? ACT_SP3_HOLD : ACT_SP3, dir);
    if (!sp4_key.prev && sp4_key.pressed)
        EnqueueAction((sp4_key.hold_frames >= HOLD_THRESHOLD) ? ACT_SP4_HOLD : ACT_SP4, dir);
    if (!super_key.prev && super_key.pressed)
        EnqueueAction(ACT_SUPER, dir);

    /* A macro owns the entire output frame, even when its output is zero. */
    if (ActionPlayer_Process(mirror, &out))
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
    if (in & BIND_THROW)
        out |= OUT_LP | OUT_LK;
    return out;
}
