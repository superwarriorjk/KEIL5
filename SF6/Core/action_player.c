#include "action_player.h"
#include "../Profiles/character_profile.h"
ActionState current_action = {0};
static OUT_STATE MirrorOutput(OUT_STATE out)
{
    OUT_STATE result = out & ~(OUT_LEFT | OUT_RIGHT);
    if (out & OUT_LEFT)
        result |= OUT_RIGHT;
    if (out & OUT_RIGHT)
        result |= OUT_LEFT;
    return result;
}
uint8_t ActionPlayer_Process(OUT_STATE *out, uint32_t held_charge_id)
{
    Action next;
    const ActionSequence *sequence;
    if (current_action.holding)
    {
        if (held_charge_id && held_charge_id == current_action.charge_id)
        {
            *out = OUT_LP;
            if (current_action.auto_lp_frames < UINT16_MAX)
                current_action.auto_lp_frames++;
            return PLAYER_BLEND;
        }
        current_action.holding = 0;
        current_action.active = 0;
        current_action.frame = 0;
    }
    if (!current_action.active && ActionQueue_Pop(&next))
    {
        current_action.active = 1;
        current_action.type = next.type;
        current_action.dir = next.dir;
        current_action.mirror = next.mirror;
        current_action.frame = 0;
        current_action.charge_id = next.charge_id;
        if (next.type == ACT_MAI_CHARGE)
            current_action.auto_lp_frames = 0;
    }
    if (!current_action.active)
        return 0;
    sequence = CharacterProfile_GetActive()->find_action(current_action.type, current_action.dir);
    *out = 0;
    if (sequence && current_action.frame < sequence->frame_count)
        *out = sequence->frames[current_action.frame];
    if (current_action.mirror)
        *out = MirrorOutput(*out);
    /* The first LP frame is guaranteed even if MK was released in the queue.
     * Direction prefix is exclusive; LP phase permits ordinary inputs. */
    if (current_action.type == ACT_MAI_CHARGE && sequence &&
        current_action.frame == sequence->frame_count - 1)
    {
        current_action.auto_lp_frames = 1;
        current_action.holding = held_charge_id &&
            held_charge_id == current_action.charge_id;
        current_action.active = current_action.holding;
        current_action.frame = 0;
        return PLAYER_BLEND;
    }
    current_action.frame++;
    if (sequence && current_action.frame >= sequence->frame_count)
    {
        current_action.active = 0;
        current_action.frame = 0;
    }
    return 1;
}
