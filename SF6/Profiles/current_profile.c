#include "character_profile.h"

/* Exact pre-refactor output, including dormant SP actions and SUPER release.
 * Rows: DOWN selection, LEFT selection, RIGHT selection, no selection.
 * This is the working baseline, not an inferred game character move list.
 */

static const OUT_STATE sp1_frames[4][MAX_FRAMES] = {
    {OUT_DOWN, OUT_RIGHT, OUT_LEFT, OUT_UP},
    {OUT_DOWN, OUT_RIGHT, OUT_LEFT, OUT_UP},
    {OUT_DOWN, OUT_RIGHT, OUT_LEFT, OUT_UP},
    {OUT_DOWN, OUT_RIGHT, OUT_LEFT, OUT_UP},
};
static const ActionSequence sp1[4] = {
    {sp1_frames[0], MAX_FRAMES},
    {sp1_frames[1], MAX_FRAMES},
    {sp1_frames[2], MAX_FRAMES},
    {sp1_frames[3], MAX_FRAMES},
};

static const OUT_STATE sp2_frames[4][MAX_FRAMES] = {
    {OUT_RIGHT, OUT_DOWN, OUT_DOWN | OUT_RIGHT, OUT_MK},
    {OUT_RIGHT, OUT_DOWN, OUT_DOWN | OUT_RIGHT, OUT_LK},
    {OUT_RIGHT, OUT_DOWN, OUT_DOWN | OUT_RIGHT, OUT_MK | OUT_HK},
    {OUT_RIGHT, OUT_DOWN, OUT_DOWN | OUT_RIGHT, OUT_HK},
};
static const ActionSequence sp2[4] = {
    {sp2_frames[0], MAX_FRAMES},
    {sp2_frames[1], MAX_FRAMES},
    {sp2_frames[2], MAX_FRAMES},
    {sp2_frames[3], MAX_FRAMES},
};

static const OUT_STATE sp3_frames[4][MAX_FRAMES] = {
    {OUT_DOWN, OUT_DOWN | OUT_LEFT, OUT_LEFT, OUT_MK},
    {OUT_DOWN, OUT_DOWN | OUT_LEFT, OUT_LEFT, OUT_LK},
    {OUT_DOWN, OUT_DOWN | OUT_LEFT, OUT_LEFT, OUT_HK},
    {OUT_DOWN, OUT_DOWN | OUT_LEFT, OUT_LEFT, OUT_MK | OUT_HK},
};
static const ActionSequence sp3[4] = {
    {sp3_frames[0], MAX_FRAMES},
    {sp3_frames[1], MAX_FRAMES},
    {sp3_frames[2], MAX_FRAMES},
    {sp3_frames[3], MAX_FRAMES},
};

static const OUT_STATE sp4_frames[4][MAX_FRAMES] = {
    {OUT_RIGHT, OUT_MK | OUT_MP, OUT_RIGHT, OUT_RIGHT},
    {OUT_RIGHT, OUT_MK | OUT_MP, OUT_RIGHT, OUT_RIGHT},
    {OUT_RIGHT, OUT_MK | OUT_MP, OUT_RIGHT, OUT_RIGHT},
    {OUT_RIGHT, OUT_MK | OUT_MP, OUT_RIGHT, OUT_RIGHT},
};
static const ActionSequence sp4[4] = {
    {sp4_frames[0], MAX_FRAMES},
    {sp4_frames[1], MAX_FRAMES},
    {sp4_frames[2], MAX_FRAMES},
    {sp4_frames[3], MAX_FRAMES},
};

static const OUT_STATE hold_left_frames[4][MAX_FRAMES] = {
    {OUT_DOWN, OUT_DOWN | OUT_LEFT, OUT_LEFT, OUT_MP | OUT_MK},
    {OUT_DOWN, OUT_DOWN | OUT_LEFT, OUT_LEFT, OUT_LP | OUT_LK},
    {OUT_DOWN, OUT_DOWN | OUT_LEFT, OUT_LEFT, OUT_HP | OUT_HK},
    {OUT_DOWN, OUT_DOWN | OUT_LEFT, OUT_LEFT, OUT_MP | OUT_HP | OUT_MK},
};
static const ActionSequence hold_left[4] = {
    {hold_left_frames[0], MAX_FRAMES},
    {hold_left_frames[1], MAX_FRAMES},
    {hold_left_frames[2], MAX_FRAMES},
    {hold_left_frames[3], MAX_FRAMES},
};

static const OUT_STATE hold_right_frames[4][MAX_FRAMES] = {
    {OUT_DOWN, OUT_DOWN | OUT_RIGHT, OUT_RIGHT, OUT_MP | OUT_MK},
    {OUT_DOWN, OUT_DOWN | OUT_RIGHT, OUT_RIGHT, OUT_LP | OUT_LK},
    {OUT_DOWN, OUT_DOWN | OUT_RIGHT, OUT_RIGHT, OUT_HP | OUT_HK},
    {OUT_DOWN, OUT_DOWN | OUT_RIGHT, OUT_RIGHT, OUT_MP | OUT_HP | OUT_MK},
};
static const ActionSequence hold_right[4] = {
    {hold_right_frames[0], MAX_FRAMES},
    {hold_right_frames[1], MAX_FRAMES},
    {hold_right_frames[2], MAX_FRAMES},
    {hold_right_frames[3], MAX_FRAMES},
};

static const OUT_STATE super_frames[4][SUPER_FRAMES] = {
    {OUT_DOWN, OUT_DOWN | OUT_LEFT, OUT_LEFT, OUT_DOWN, OUT_DOWN | OUT_LEFT, OUT_LEFT, OUT_HP, 0},
    {OUT_DOWN, OUT_DOWN | OUT_RIGHT, OUT_RIGHT, OUT_DOWN, OUT_DOWN | OUT_RIGHT, OUT_RIGHT, OUT_HK,
     0},
    {OUT_DOWN, OUT_DOWN | OUT_RIGHT, OUT_RIGHT, OUT_DOWN, OUT_DOWN | OUT_RIGHT, OUT_RIGHT, OUT_HP,
     0},
    {0, 0, 0, 0, 0, 0, 0, 0},
};
static const ActionSequence super[4] = {
    {super_frames[0], SUPER_FRAMES},
    {super_frames[1], SUPER_FRAMES},
    {super_frames[2], SUPER_FRAMES},
    {super_frames[3], SUPER_FRAMES},
};

static const ActionSequence *FindAction(uint8_t type, uint8_t dir)
{
    /* Original ordinary SP actions use default for any other dir value.
     * Original SUPER has no matching branch for those values: eight zeros. */
    if (dir > 3)
        dir = 3;
    switch (type)
    {
    case ACT_SP1:
        return &sp1[dir];
    case ACT_SP2:
        return &sp2[dir];
    case ACT_SP3:
        return &sp3[dir];
    case ACT_SP4:
        return &sp4[dir];
    case ACT_SP1_HOLD:
    case ACT_SP3_HOLD:
        return &hold_left[dir];
    case ACT_SP2_HOLD:
    case ACT_SP4_HOLD:
        return &hold_right[dir];
    case ACT_SUPER:
        return &super[dir];
    default:
        return 0;
    }
}

const CharacterProfile current_profile = {"current", FindAction};
