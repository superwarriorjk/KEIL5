#include "action_player.h"
#include "../Profiles/character_profile.h"
ActionState current_action = {0, 0, 0, 0};
static OUT_STATE MirrorOutput(OUT_STATE out)
{
    OUT_STATE result = out & ~(OUT_LEFT | OUT_RIGHT);
    if (out & OUT_LEFT)
        result |= OUT_RIGHT;
    if (out & OUT_RIGHT)
        result |= OUT_LEFT;
    return result;
}
uint8_t ActionPlayer_Process(uint8_t mirror, OUT_STATE *out)
{
    Action next;
    const ActionSequence *sequence;
    if (!current_action.active && ActionQueue_Pop(&next))
    {
        current_action.active = 1;
        current_action.type = next.type;
        current_action.dir = next.dir;
        current_action.frame = 0;
    }
    if (!current_action.active)
        return 0;
    sequence = CharacterProfile_GetActive()->find_action(current_action.type, current_action.dir);
    *out = 0;
    if (sequence && current_action.frame < sequence->frame_count)
        *out = sequence->frames[current_action.frame];
    if (mirror)
        *out = MirrorOutput(*out);
    current_action.frame++;
    if (sequence && current_action.frame >= sequence->frame_count)
    {
        current_action.active = 0;
        current_action.frame = 0;
    }
    return 1;
}
