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
} ActionState;
extern ActionState current_action;
/* Returns 1 when this frame belongs to a macro, including a release frame. */
uint8_t ActionPlayer_Process(OUT_STATE *out);
#endif
