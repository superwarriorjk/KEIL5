#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "Core/controller_logic.h"
#include "Core/action_player.h"
#include "Core/input_bindings.h"
#include "Profiles/character_profile.h"

typedef struct { uint8_t pressed, prev; uint16_t hold_frames; } KeyState;
extern KeyState super_key;
static unsigned long checked;
#define CHECK(x) do { if (!(x)) { \
    fprintf(stderr, "Mai FAIL line %d: %s (step %lu)\n", __LINE__, #x, checked); \
    exit(1); } } while (0)
static void Reset(void)
{
    memset(&super_key, 0, sizeof(super_key));
    memset(&special_session, 0, sizeof(special_session));
    memset(&current_action, 0, sizeof(current_action));
    memset(action_queue, 0, sizeof(action_queue));
    queue_head = queue_tail = facing_left = 0;
}
static void Expect(IN_STATE in, OUT_STATE expected)
{
    OUT_STATE actual = Logic_Process(in);
    checked++;
    if (actual != expected) {
        fprintf(stderr, "input %lx expected %x actual %x\n",
            (unsigned long)in, expected, actual);
        CHECK(actual == expected);
    }
}
static void Idle(void)
{
    CHECK(!current_action.active && !current_action.holding);
    CHECK(queue_head == queue_tail);
}
static OUT_STATE Mirror(OUT_STATE out, unsigned mirror)
{
    OUT_STATE result = out;
    if (mirror) {
        result &= ~(OUT_LEFT | OUT_RIGHT);
        if (out & OUT_LEFT) result |= OUT_RIGHT;
        if (out & OUT_RIGHT) result |= OUT_LEFT;
    }
    return result;
}
static void TestMoves(void)
{
    /* Independent expectations: no production profile data used as oracle. */
    const OUT_STATE normal[4][4] = {
        {OUT_RIGHT, OUT_DOWN, OUT_DOWN|OUT_RIGHT, OUT_HK},
        {OUT_DOWN, OUT_DOWN|OUT_LEFT, OUT_LEFT, OUT_LP},
        {OUT_DOWN, OUT_DOWN|OUT_RIGHT, OUT_RIGHT, OUT_LK},
        {OUT_DOWN, OUT_DOWN|OUT_RIGHT, OUT_RIGHT, OUT_LP}
    };
    const OUT_STATE od_attack[4] = {
        OUT_LK|OUT_MK, OUT_LP|OUT_MP, OUT_LK|OUT_MK, OUT_LP|OUT_MP
    };
    unsigned bits, mirror, od, frame, delay;
    for (bits = 0; bits < 16; ++bits)
    for (mirror = 0; mirror < 2; ++mirror)
    for (od = 0; od < 2; ++od)
    for (delay = 0; delay < 2; ++delay) {
        IN_STATE dirs = ((bits & 1) ? BIND_UP : 0) |
            ((bits & 2) ? BIND_DOWN : 0) |
            ((bits & 4) ? BIND_LEFT : 0) |
            ((bits & 8) ? BIND_RIGHT : 0);
        OUT_STATE direct = (OUT_STATE)bits;
        unsigned selection = (bits & 2) ? 0 : (bits & 4) ? 1 : (bits & 8) ? 2 : 3;
        Reset(); facing_left = (uint8_t)mirror;
        /* Direction before key and direction same frame both work. */
        if (delay) Expect(dirs, direct);
        if (!od || delay) {
            Expect(BIND_SPECIAL | dirs, direct);
            Idle();
            Expect(BIND_SPECIAL, 0); /* Direction release cannot alter choice. */
        }
        if (bits == 1) {
            Expect(od ? BIND_SPECIAL|IN_LK|dirs : 0, od ? OUT_UP|OUT_LK : 0);
            Expect(0, 0); Idle();
            continue;
        }
        for (frame = 0; frame < 4; ++frame) {
            IN_STATE in = od ? BIND_SPECIAL|IN_LK : 0;
            OUT_STATE out = (od && frame == 3) ? od_attack[selection] : normal[selection][frame];
            if (od && !delay && frame == 0) in |= dirs;
            Expect(in, Mirror(out, mirror));
        }
        Expect(od ? BIND_SPECIAL|IN_LK : 0, 0);
        Expect(od ? IN_LK : 0, 0); /* Release special cannot duplicate OD. */
        Expect(0, 0); Idle();
        Expect(IN_LK, OUT_LK); /* Fresh LK is an ordinary kick again. */
    }
    CHECK(CharacterProfile_GetActive() == &mai_profile);
}
static void TestEdges(void)
{
    unsigned i;
    Reset();
    Expect(IN_LK|IN_MK, OUT_LK|OUT_MK);
    Expect(BIND_SPECIAL|IN_LK|IN_MK, OUT_LK|OUT_MK);
    for (i = 0; i < 20; ++i) Expect(BIND_SPECIAL|IN_LK|IN_MK, OUT_LK|OUT_MK);
    Idle(); /* Preheld modifiers never select a version. */
    Expect(BIND_SPECIAL, 0);
    Expect(BIND_SPECIAL|IN_LK|IN_MK, OUT_DOWN);
    CHECK(current_action.type == ACT_MAI_OD); /* OD wins simultaneous edges. */
    Expect(BIND_SPECIAL|IN_LK|IN_MK, OUT_DOWN|OUT_RIGHT);
    Expect(BIND_SPECIAL|IN_LK|IN_MK, OUT_RIGHT);
    Expect(BIND_SPECIAL|IN_LK|IN_MK, OUT_LP|OUT_MP);
    Expect(BIND_SPECIAL|IN_LK|IN_MK, OUT_MK); /* Unselected MK passes. */
    Expect(IN_LK|IN_MK, OUT_MK);
    Expect(0, 0); Idle();

    Reset();
    Expect(BIND_SPECIAL, 0);
    Expect(IN_LK|IN_MK, OUT_DOWN); /* Release wins over both edges. */
    Expect(IN_LK|IN_MK, OUT_DOWN|OUT_RIGHT);
    Expect(IN_LK|IN_MK, OUT_RIGHT);
    Expect(IN_LK|IN_MK, OUT_LP);
    Expect(IN_LK|IN_MK, OUT_LK|OUT_MK);
    Idle();

    Reset();
    Expect(BIND_SPECIAL|BIND_UP, OUT_UP);
    Expect(BIND_SPECIAL|BIND_DOWN|IN_LK|IN_MK, OUT_DOWN|OUT_LK|OUT_MK);
    Expect(0, 0); Idle(); /* Up-only session cannot become valid later. */

    Reset();
    Expect(BIND_SPECIAL, 0);
    Expect(BIND_SPECIAL|BIND_LEFT, OUT_LEFT);
    Expect(0, OUT_DOWN);
    Expect(0, OUT_DOWN|OUT_RIGHT); /* Later left cannot change selected fan. */
    Expect(0, OUT_RIGHT); Expect(0, OUT_LP); Expect(0, 0);

    Reset();
    Expect(BIND_SPECIAL|BIND_DOWN|IN_MK, OUT_DOWN|OUT_MK);
    Idle(); /* MK does not select charge for a non-fan. */
    Expect(0, OUT_RIGHT); Expect(0, OUT_DOWN);
    Expect(0, OUT_DOWN|OUT_RIGHT); Expect(0, OUT_HK); Expect(0, 0);

    Reset();
    Expect(IN_MK, OUT_MK);
    Expect(BIND_SPECIAL|IN_MK, OUT_MK);
    Expect(BIND_SPECIAL, 0);
    Expect(BIND_SPECIAL|IN_MK, OUT_DOWN); /* Fresh MK edge is now eligible. */
    CHECK(special_session.state == SPECIAL_CHARGING);

    Reset();
    Expect(BIND_SPECIAL|IN_LK, OUT_DOWN);
    Expect(BIND_SPECIAL, OUT_DOWN|OUT_RIGHT);
    Expect(BIND_SPECIAL|IN_MK, OUT_RIGHT);
    Expect(BIND_SPECIAL|IN_MK, OUT_LP|OUT_MP);
    Expect(IN_MK, OUT_MK); /* OD cannot become charge afterwards. */
    Idle();
}
static void TestCharge(void)
{
    unsigned i, release_frame, mirror;
    const OUT_STATE prefix[] = {OUT_DOWN, OUT_DOWN|OUT_RIGHT, OUT_RIGHT, OUT_LP};
    for (mirror = 0; mirror < 2; ++mirror)
    for (release_frame = 1; release_frame < 6; ++release_frame) {
        Reset(); facing_left = (uint8_t)mirror;
        for (i = 0; i < 6; ++i) {
            IN_STATE in = (i < release_frame ? IN_MK : 0) | (i == 0 ? BIND_SPECIAL : 0);
            OUT_STATE out = i < 4 ? prefix[i] : i < release_frame ? OUT_LP : 0;
            Expect(in, Mirror(out, mirror));
        }
        Expect(0, 0); Idle();
        CHECK(current_action.auto_lp_frames == (release_frame > 4 ? release_frame-3 : 1));
        CHECK(special_session.mk_hold_samples == release_frame);
    }
    Reset();
    Expect(BIND_SPECIAL|IN_MK, OUT_DOWN);
    Expect(IN_MK, OUT_DOWN|OUT_RIGHT); /* Special release cannot submit normal. */
    Expect(IN_MK|IN_HP, OUT_RIGHT); /* Prefix remains exclusive. */
    Expect(IN_MK|IN_HP|BIND_LEFT, OUT_LP|OUT_HP|OUT_LEFT);
    CHECK(current_action.holding);
    Expect(IN_MK|IN_LK|IN_MP|IN_HK|BIND_UP|BIND_DOWN|BIND_LEFT|BIND_RIGHT,
        OUT_LP|OUT_LK|OUT_MP|OUT_HK|OUT_UP|OUT_DOWN|OUT_LEFT|OUT_RIGHT);
    Expect(IN_MK|BIND_MEDIUM_PAIR, OUT_LP|OUT_MP|OUT_MK); /* Independent chord passes. */
    Expect(IN_LP|IN_HP, OUT_LP|OUT_HP); /* Remove auto LP, preserve physical LP. */
    Expect(IN_HP, OUT_HP); Idle();
    Expect(IN_MK, OUT_MK); /* A new MK alone cannot revive a finished fan. */

    Reset();
    Expect(BIND_SPECIAL|IN_MK, OUT_DOWN);
    Expect(BIND_SPECIAL|IN_MK|IN_LK, OUT_DOWN|OUT_RIGHT);
    Expect(BIND_SPECIAL|IN_MK|IN_LK, OUT_RIGHT);
    Expect(BIND_SPECIAL|IN_MK|IN_LK, OUT_LP|OUT_LK); /* Charge is version locked. */
    Expect(IN_MK|IN_LK, OUT_LP|OUT_LK);
    Expect(0, 0); Idle();

    Reset();
    Expect(BIND_SPECIAL|IN_MK, OUT_DOWN);
    Expect(IN_MK, OUT_DOWN|OUT_RIGHT); Expect(IN_MK, OUT_RIGHT);
    Expect(IN_MK, OUT_LP);
    for (i = 0; i < 70000; ++i) Expect(IN_MK, OUT_LP);
    CHECK(special_session.mk_hold_samples == UINT16_MAX);
    CHECK(current_action.auto_lp_frames == UINT16_MAX);
    Expect(0, 0); Idle();
}
static void TestQueueAndFacing(void)
{
    unsigned i;
    uint32_t first, second;
    const OUT_STATE super_down[] = {OUT_DOWN, OUT_DOWN|OUT_LEFT, OUT_LEFT,
        OUT_DOWN, OUT_DOWN|OUT_LEFT, OUT_LEFT, OUT_HP, 0};
    Reset();
    Expect(BIND_SPECIAL, 0);
    Expect(BIND_SPECIAL|BIND_SUPER, 0); CHECK(facing_left == 1);
    Expect(0, OUT_DOWN); Expect(0, OUT_DOWN|OUT_RIGHT); /* Prepared facing right. */
    Expect(0, OUT_RIGHT); Expect(0, OUT_LP);
    Reset();
    Expect(BIND_SPECIAL|BIND_SUPER|IN_LK, OUT_DOWN);
    CHECK(facing_left == 1);
    Expect(IN_LK, OUT_DOWN|OUT_LEFT); /* Same-frame toggle happens first. */
    Expect(IN_LK, OUT_LEFT); Expect(IN_LK, OUT_LP|OUT_MP); Expect(0, 0);

    Reset();
    Expect(BIND_SPECIAL|BIND_SUPER|BIND_DOWN|IN_LK, super_down[0]);
    for (i = 1; i < 8; ++i) Expect(0, super_down[i]);
    Expect(0, OUT_RIGHT); Expect(0, OUT_DOWN); /* SUPER before special. */
    Expect(0, OUT_DOWN|OUT_RIGHT); Expect(0, OUT_LK|OUT_MK); Expect(0, 0);

    Reset();
    Expect(BIND_SPECIAL|IN_MK, OUT_DOWN);
    Expect(IN_MK, OUT_DOWN|OUT_RIGHT); Expect(IN_MK, OUT_RIGHT); Expect(IN_MK, OUT_LP);
    Expect(IN_MK|BIND_SUPER|BIND_DOWN, OUT_LP|OUT_DOWN);
    CHECK(queue_head != queue_tail);
    for (i = 0; i < 20; ++i) Expect(IN_MK, OUT_LP); /* Queued macro stays paused. */
    for (i = 0; i < 8; ++i) Expect(0, super_down[i]);
    Expect(0, 0); Idle();

    /* Two charge requests behind SUPER: first MK release must remain final. */
    Reset();
    Expect(BIND_SUPER|BIND_DOWN, super_down[0]);
    Expect(BIND_SPECIAL|IN_MK, super_down[1]);
    first = special_session.held_charge_id; CHECK(first);
    Expect(0, super_down[2]);
    Expect(BIND_SPECIAL|IN_MK, super_down[3]);
    second = special_session.held_charge_id; CHECK(second && second != first);
    for (i = 4; i < 8; ++i) Expect(IN_MK, super_down[i]);
    Expect(IN_MK, OUT_DOWN); Expect(IN_MK, OUT_DOWN|OUT_RIGHT);
    Expect(IN_MK, OUT_RIGHT); Expect(IN_MK, OUT_LP);
    CHECK(!current_action.holding && current_action.charge_id == first);
    Expect(IN_MK, OUT_DOWN); Expect(IN_MK, OUT_DOWN|OUT_RIGHT);
    Expect(IN_MK, OUT_RIGHT); Expect(IN_MK, OUT_LP);
    CHECK(current_action.holding && current_action.charge_id == second);
    Expect(IN_MK, OUT_LP); Expect(0, 0); Idle();

    /* Released in queue, then an unrelated MK press: one LP frame only. */
    Reset();
    Expect(BIND_SUPER|BIND_DOWN, super_down[0]);
    Expect(BIND_SPECIAL|IN_MK, super_down[1]);
    Expect(0, super_down[2]);
    for (i = 3; i < 8; ++i) Expect(IN_MK, super_down[i]);
    Expect(IN_MK, OUT_DOWN); Expect(IN_MK, OUT_DOWN|OUT_RIGHT);
    Expect(IN_MK, OUT_RIGHT); Expect(IN_MK, OUT_LP|OUT_MK);
    Expect(IN_MK, OUT_MK); Idle();

    /* Prefix release/repress cannot extend old charge either. */
    Reset();
    Expect(BIND_SPECIAL|IN_MK, OUT_DOWN); Expect(BIND_SPECIAL, OUT_DOWN|OUT_RIGHT);
    Expect(BIND_SPECIAL|IN_MK, OUT_RIGHT); Expect(BIND_SPECIAL|IN_MK, OUT_LP|OUT_MK);
    Expect(IN_MK, OUT_MK); Idle();

    /* ID wrap skips a live queued request instead of reusing its identity. */
    Reset();
    CHECK(EnqueueCharge(3, 0, 1));
    special_session.next_charge_id = UINT32_MAX;
    Expect(BIND_SPECIAL|IN_MK, OUT_DOWN);
    CHECK(special_session.held_charge_id == 2);
    Expect(IN_MK, OUT_DOWN|OUT_RIGHT); Expect(IN_MK, OUT_RIGHT); Expect(IN_MK, OUT_LP);
    CHECK(!current_action.holding);
}
static void TestOverflow(void)
{
    unsigned version, i;
    const OUT_STATE super_down[] = {OUT_DOWN, OUT_DOWN|OUT_LEFT, OUT_LEFT,
        OUT_DOWN, OUT_DOWN|OUT_LEFT, OUT_LEFT, OUT_HP, 0};
    for (version = 0; version < 3; ++version) {
        IN_STATE modifier = version == 1 ? IN_LK : version == 2 ? IN_MK : 0;
        Reset();
        /* Three waiting slots, plus one playing SUPER. */
        Expect(BIND_SUPER|BIND_DOWN, OUT_DOWN);
        CHECK(EnqueueAction(ACT_SUPER, 0, 0));
        CHECK(EnqueueAction(ACT_SUPER, 0, 0));
        CHECK(EnqueueAction(ACT_SUPER, 0, 0));
        CHECK(!EnqueueAction(ACT_SUPER, 0, 0));
        Expect(BIND_SPECIAL|modifier, OUT_DOWN|OUT_LEFT);
        Expect(modifier, OUT_LEFT);
        CHECK(!special_session.held_charge_id);
        for (i = 3; i < 32; ++i) Expect(modifier, super_down[i % 8]);
        Expect(modifier, 0); /* No hidden fourth request after queued SUPERs. */
        Idle();
        CHECK(!current_action.holding);
        /* Failed modifier request is consumed until release, never a stray kick. */
        Expect(modifier, 0);
        Expect(0, 0);
        Expect(modifier, (OUT_STATE)modifier);
        Expect(0, 0);
        Expect(BIND_SPECIAL, 0); Expect(0, OUT_DOWN); /* Next session works. */
    }
    /* SUPER takes the last available queue slot before a same-frame special. */
    Reset();
    Expect(BIND_SPECIAL|IN_MK, OUT_DOWN);
    Expect(IN_MK, OUT_DOWN|OUT_RIGHT); Expect(IN_MK, OUT_RIGHT); Expect(IN_MK, OUT_LP);
    CHECK(EnqueueAction(ACT_SUPER, 0, 0)); CHECK(EnqueueAction(ACT_SUPER, 0, 0));
    Expect(IN_MK|BIND_SUPER|BIND_SPECIAL|BIND_RIGHT|IN_LK, OUT_LP|OUT_RIGHT);
    CHECK(action_queue[(queue_tail + QUEUE_LEN - 1) % QUEUE_LEN].type == ACT_SUPER);
    CHECK(special_session.state == SPECIAL_COMMITTED);
    CHECK(!EnqueueAction(ACT_SUPER, 0, 0));
    for (i = 0; i < 40; ++i) Logic_Process(0);
    Idle();
}
void TestMai(void)
{
    TestMoves(); TestEdges(); TestCharge(); TestQueueAndFacing(); TestOverflow();
    printf("PASS: Mai %lu checked frames; normal/OD, edges, charge, counters, queue, IDs, facing.\n", checked);
}
