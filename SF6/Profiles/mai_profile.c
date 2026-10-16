#include "character_profile.h"
/* Rows use physical selection priority: down, left, right, neutral.
 * Facing is applied by the player, never by selection logic. */
static const OUT_STATE normal_frames[4][MAX_FRAMES] = {
    {OUT_RIGHT, OUT_DOWN, OUT_DOWN | OUT_RIGHT, OUT_HK},
    {OUT_DOWN, OUT_DOWN | OUT_LEFT, OUT_LEFT, OUT_LP},
    {OUT_DOWN, OUT_DOWN | OUT_RIGHT, OUT_RIGHT, OUT_LK},
    {OUT_DOWN, OUT_DOWN | OUT_RIGHT, OUT_RIGHT, OUT_LP}
};
static const OUT_STATE od_frames[4][MAX_FRAMES] = {
    {OUT_RIGHT, OUT_DOWN, OUT_DOWN | OUT_RIGHT, OUT_LK | OUT_MK},
    {OUT_DOWN, OUT_DOWN | OUT_LEFT, OUT_LEFT, OUT_LP | OUT_MP},
    {OUT_DOWN, OUT_DOWN | OUT_RIGHT, OUT_RIGHT, OUT_LK | OUT_MK},
    {OUT_DOWN, OUT_DOWN | OUT_RIGHT, OUT_RIGHT, OUT_LP | OUT_MP}
};
static const ActionSequence normal[4] = {
    {normal_frames[0], MAX_FRAMES}, {normal_frames[1], MAX_FRAMES},
    {normal_frames[2], MAX_FRAMES}, {normal_frames[3], MAX_FRAMES}
};
static const ActionSequence od[4] = {
    {od_frames[0], MAX_FRAMES}, {od_frames[1], MAX_FRAMES},
    {od_frames[2], MAX_FRAMES}, {od_frames[3], MAX_FRAMES}
};
static const ActionSequence *FindAction(uint8_t type, uint8_t dir)
{
    if (type == ACT_MAI_CHARGE) return &normal[3];
    if (type == ACT_MAI_NORMAL) return &normal[dir < 4 ? dir : 3];
    if (type == ACT_MAI_OD) return &od[dir < 4 ? dir : 3];
    /* Reuse the exact existing eight-frame SUPER and dormant legacy tables. */
    return current_profile.find_action(type, dir);
}
const CharacterProfile mai_profile = {"mai", FindAction};
