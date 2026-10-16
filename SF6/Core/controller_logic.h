#ifndef SF6_CONTROLLER_LOGIC_H
#define SF6_CONTROLLER_LOGIC_H
#include "controller_types.h"
extern int test_flag;
extern uint8_t facing_left;
#define SPECIAL_IDLE 0
#define SPECIAL_WAITING 1
#define SPECIAL_COMMITTED 2
#define SPECIAL_CHARGING 3
typedef struct
{
    IN_STATE previous_input;
    IN_STATE consumed_keys;
    uint8_t state;
    uint8_t dir;
    uint8_t mirror;
    uint32_t held_charge_id;
    uint32_t next_charge_id;
    uint16_t mk_hold_samples;
} SpecialSession;
extern SpecialSession special_session;
OUT_STATE Logic_Process(IN_STATE in);
#endif
