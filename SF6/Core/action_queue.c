#include "action_queue.h"
Action action_queue[QUEUE_LEN] = {0};
uint8_t queue_head = 0;
uint8_t queue_tail = 0;
/* One slot stays empty; a full queue drops the new request, as before. */
static uint8_t Push(uint8_t type, uint8_t dir, uint8_t mirror, uint32_t charge_id)
{
    uint8_t next = (queue_tail + 1) % QUEUE_LEN;
    if (next != queue_head)
    {
        action_queue[queue_tail].type = type;
        action_queue[queue_tail].dir = dir;
        action_queue[queue_tail].mirror = mirror;
        action_queue[queue_tail].charge_id = charge_id;
        queue_tail = next;
        return 1;
    }
    return 0;
}
uint8_t EnqueueAction(uint8_t type, uint8_t dir, uint8_t mirror)
{
    return Push(type, dir, mirror, 0);
}
uint8_t EnqueueCharge(uint8_t dir, uint8_t mirror, uint32_t charge_id)
{
    return Push(ACT_MAI_CHARGE, dir, mirror, charge_id);
}
uint8_t ActionQueue_Pop(Action *action)
{
    if (queue_head == queue_tail)
        return 0;
    *action = action_queue[queue_head];
    queue_head = (queue_head + 1) % QUEUE_LEN;
    return 1;
}
