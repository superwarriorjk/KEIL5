#ifndef SF6_CHARACTER_PROFILE_H
#define SF6_CHARACTER_PROFILE_H

#include "../Core/action_queue.h"

/* One entry is one frame, including explicit 0 entries for released buttons.
 * Profiles and their arrays must have static lifetime; no allocation is used.
 * Direction mirroring is applied by the player, not by the profile.
 * frame_count must be 1..255 and match the array length. Only registered
 * action IDs may be enqueued; find_action returns 0 for unknown IDs.
 */
typedef struct
{
    const OUT_STATE *frames;
    uint8_t frame_count;
} ActionSequence;

typedef struct
{
    const char *name;
    const ActionSequence *(*find_action)(uint8_t type, uint8_t dir);
} CharacterProfile;

extern const CharacterProfile current_profile;
const CharacterProfile *CharacterProfile_GetActive(void);

#endif
