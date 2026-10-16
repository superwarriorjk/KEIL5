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
SpecialSession special_session = {0};

static void UpdateKey(KeyState *k, uint8_t is_pressed)
{
    k->prev = k->pressed;
    k->pressed = is_pressed;
    if (k->pressed)
        k->hold_frames++;
    else
        k->hold_frames = 0;
}

int test_flag = 10;

/* Skip IDs still in flight on wrap, so a later MK press cannot revive an
 * old queued request. Zero always means no live MK-controlled request. */
static uint32_t NextChargeId(void)
{
    uint8_t i, used;
    do
    {
        special_session.next_charge_id++;
        if (!special_session.next_charge_id)
            special_session.next_charge_id++;
        used = current_action.active &&
            current_action.charge_id == special_session.next_charge_id;
        for (i = queue_head; i != queue_tail; i = (i + 1) % QUEUE_LEN)
            if (action_queue[i].charge_id == special_session.next_charge_id)
                used = 1;
    } while (used);
    return special_session.next_charge_id;
}

static void ProcessSpecial(IN_STATE in, uint8_t dir)
{
    IN_STATE rising = in & ~special_session.previous_input;
    uint32_t id;
    special_session.consumed_keys &= in;
    if (!(in & IN_MK))
        special_session.held_charge_id = 0;

    if (rising & BIND_SPECIAL)
    {
        special_session.dir = dir;
        special_session.mirror = facing_left;
        special_session.state = (dir == 3 && (in & BIND_UP)) ?
            SPECIAL_IDLE : SPECIAL_WAITING;
    }
    if (!(in & BIND_SPECIAL))
    {
        /* Release wins over a simultaneous LK/MK edge. No retry on overflow. */
        if (special_session.state == SPECIAL_WAITING)
            EnqueueAction(ACT_MAI_NORMAL, special_session.dir, special_session.mirror);
        special_session.state = SPECIAL_IDLE;
    }
    else if (special_session.state == SPECIAL_WAITING)
    {
        if (rising & IN_LK)
        {
            special_session.state = SPECIAL_COMMITTED;
            special_session.consumed_keys |= IN_LK;
            EnqueueAction(ACT_MAI_OD, special_session.dir, special_session.mirror);
        }
        else if (special_session.dir == 3 && (rising & IN_MK))
        {
            special_session.state = SPECIAL_COMMITTED;
            special_session.consumed_keys |= IN_MK;
            id = NextChargeId();
            if (EnqueueCharge(3, special_session.mirror, id))
            {
                special_session.state = SPECIAL_CHARGING;
                special_session.held_charge_id = id;
                special_session.mk_hold_samples = 0;
            }
        }
    }
    if (special_session.held_charge_id && special_session.mk_hold_samples < UINT16_MAX)
        special_session.mk_hold_samples++;
    special_session.previous_input = in;
}

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

    /* SUPER/toggle precedes special submission; prepared facing stays fixed. */
    ProcessSpecial(in, dir);
    if (ActionPlayer_Process(&out, special_session.held_charge_id) == PLAYER_EXCLUSIVE)
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
    if ((in & IN_LK) && !(special_session.consumed_keys & IN_LK))
        out |= OUT_LK;
    if ((in & IN_MK) && !(special_session.consumed_keys & IN_MK))
        out |= OUT_MK;
    if (in & IN_HK)
        out |= OUT_HK;
    if (in & BIND_MEDIUM_PAIR)
        out |= OUT_MP | OUT_MK;
    return out;
}
