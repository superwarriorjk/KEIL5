#ifndef SF6_ACTION_QUEUE_H
#define SF6_ACTION_QUEUE_H
#include "controller_types.h"
#include "controller_config.h"
#define ACT_SP1 1
#define ACT_SP2 2
#define ACT_SP3 3
#define ACT_SP4 4
#define ACT_SUPER 5
#define ACT_SP1_HOLD 11
#define ACT_SP2_HOLD 12
#define ACT_SP3_HOLD 13
#define ACT_SP4_HOLD 14
typedef struct
{
    uint8_t type;
    uint8_t dir;
} Action;
/* Exported for the existing debugger watch workflow. */
extern Action action_queue[QUEUE_LEN];
extern uint8_t queue_head;
extern uint8_t queue_tail;
void EnqueueAction(uint8_t type, uint8_t dir);
uint8_t ActionQueue_Pop(Action *action);
#endif
