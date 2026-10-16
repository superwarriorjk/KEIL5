#ifndef SF6_ACTION_PLAYER_H
#define SF6_ACTION_PLAYER_H
#include "action_queue.h"
typedef struct
{
    uint8_t active;
    uint8_t type;
    uint8_t frame;
    uint8_t dir;
    uint8_t mirror;
    uint8_t holding;
    uint32_t charge_id;
    uint16_t auto_lp_frames;
} ActionState;
extern ActionState current_action;
/* EXCLUSIVE includes release frames; BLEND adds LP to ordinary input. */
#define PLAYER_IDLE 0
#define PLAYER_EXCLUSIVE 1
#define PLAYER_BLEND 2
uint8_t ActionPlayer_Process(OUT_STATE *out, uint32_t held_charge_id);
#endif
