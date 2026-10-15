#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "stm32f10x.h"
#include "Core/controller_logic.h"
#include "Core/action_player.h"
#include "Board/board_io.h"
#include "Board/frame_clock.h"

TestGPIO test_gpio_a, test_gpio_b;
TestRCC test_rcc;
TestAFIO test_afio;
TestSysTick test_systick;

typedef struct { uint8_t pressed, prev; uint16_t hold_frames; } KeyState;
extern KeyState sp1_key, sp2_key, sp3_key, sp4_key, super_key;
extern KeyState old_sp1_key, old_sp2_key, old_sp3_key, old_sp4_key, old_super_key;
extern Action old_action_queue[QUEUE_LEN];
extern uint8_t old_queue_head, old_queue_tail;
extern ActionState old_current_action;
extern int old_test_flag;
extern volatile uint8_t old_tick_flag;
OUT_STATE old_Logic_Process(IN_STATE in);
void old_EnqueueAction(uint8_t type, uint8_t dir);
void old_GPIO_Init_All(void);
IN_STATE old_ReadInputs(void);
void old_Output_Update(OUT_STATE out);
void old_SysTick_Init(void);
void old_SysTick_Handler(void);

static unsigned long frames_checked;
static uint32_t random_state = 0x6a09e667;
static uint32_t Random(void)
{
    random_state ^= random_state << 13;
    random_state ^= random_state >> 17;
    random_state ^= random_state << 5;
    return random_state;
}

#define REQUIRE(expr) do { if (!(expr)) { \
    fprintf(stderr, "FAIL line %d: %s (frame %lu)\n", __LINE__, #expr, frames_checked); \
    exit(1); } } while (0)

static void Reset(void)
{
    KeyState zero = {0};
    ActionState idle = {0};
    sp1_key = sp2_key = sp3_key = sp4_key = super_key = zero;
    old_sp1_key = old_sp2_key = old_sp3_key = old_sp4_key = old_super_key = zero;
    current_action = old_current_action = idle;
    queue_head = queue_tail = old_queue_head = old_queue_tail = 0;
    memset(action_queue, 0, sizeof(action_queue));
    memset(old_action_queue, 0, sizeof(old_action_queue));
    test_flag = old_test_flag = 10;
}

static OUT_STATE Step(IN_STATE in)
{
    OUT_STATE expected = old_Logic_Process(in);
    OUT_STATE actual = Logic_Process(in);
    frames_checked++;
    REQUIRE(actual == expected);
    REQUIRE(queue_head == old_queue_head && queue_tail == old_queue_tail);
    REQUIRE(memcmp(action_queue, old_action_queue, sizeof(action_queue)) == 0);
    REQUIRE(memcmp(&current_action, &old_current_action, sizeof(current_action)) == 0);
    REQUIRE(memcmp(&super_key, &old_super_key, sizeof(super_key)) == 0);
    REQUIRE(test_flag == old_test_flag);
    return actual;
}

static void Enqueue(uint8_t type, uint8_t dir)
{
    old_EnqueueAction(type, dir);
    EnqueueAction(type, dir);
}

static void TestLogic(void)
{
    unsigned int in, type, dir, mirror, frame, i;
    const uint8_t types[] = {1, 2, 3, 4, 5, 11, 12, 13, 14};
    /* Every 19-bit input combination from a released state. */
    for (in = 0; in < (1U << 19); ++in) { Reset(); Step(in); }

    /* All defined actions, all direction byte values, both mirror states.
     * Also exercise the dormant actions directly through the queue API. */
    for (type = 0; type < sizeof(types); ++type)
        for (dir = 0; dir < 256; ++dir)
            for (mirror = 0; mirror < 2; ++mirror) {
                Reset(); Enqueue(types[type], (uint8_t)dir);
                for (frame = 0; frame < 12; ++frame)
                    Step(IN_LP | (mirror ? IN_SP4 : 0));
            }

    /* A concrete user path, including the explicit release frame. */
    {
        const OUT_STATE expected[] = {OUT_DOWN, OUT_DOWN | OUT_LEFT, OUT_LEFT,
            OUT_DOWN, OUT_DOWN | OUT_LEFT, OUT_LEFT, OUT_HP, 0};
        Reset();
        for (i = 0; i < 8; ++i) REQUIRE(Step(IN_SP1 | IN_DOWN) == expected[i]);
        REQUIRE(Step(IN_SP1 | IN_DOWN) == OUT_DOWN); /* Held: no retrigger. */
    }

    Reset();
    for (i = 0; i < 20; ++i) Enqueue(ACT_SUPER, (uint8_t)(i % 4));
    REQUIRE(queue_tail == 3); /* Three waiting slots, overflow drops newest. */
    for (i = 0; i < 40; ++i) Step((i & 1) ? IN_SP4 : IN_LP);

    Reset();
    for (i = 0; i < 70000; ++i) Step(IN_SP1 | IN_RIGHT);
    Step(0); Step(IN_SP1 | IN_RIGHT); /* Release and retrigger after counter wrap. */

    Reset();
    for (i = 0; i < 200000; ++i) {
        if (i % 17 == 0) Enqueue(types[Random() % sizeof(types)], Random() % 4);
        Step(Random() & 0x7ffff); /* Queue wraparound, simultaneous inputs, live mirror. */
    }
}

static void TestHardware(void)
{
    unsigned int i, a;
    TestGPIO expected_a, expected_b;
    TestRCC expected_rcc;
    TestAFIO expected_afio;
    TestSysTick expected_tick;
    for (i = 0; i < 1000; ++i) {
        TestGPIO initial_a = {Random(), Random(), Random(), Random()};
        TestGPIO initial_b = {Random(), Random(), Random(), Random()};
        uint32_t initial_rcc = Random(), initial_afio = Random();
        test_gpio_a = initial_a; test_gpio_b = initial_b;
        test_rcc.APB2ENR = initial_rcc; test_afio.MAPR = initial_afio;
        old_GPIO_Init_All();
        expected_a = test_gpio_a; expected_b = test_gpio_b;
        expected_rcc = test_rcc; expected_afio = test_afio;
        test_gpio_a = initial_a; test_gpio_b = initial_b;
        test_rcc.APB2ENR = initial_rcc; test_afio.MAPR = initial_afio;
        GPIO_Init_All();
        REQUIRE(memcmp(&expected_a, &test_gpio_a, sizeof(expected_a)) == 0);
        REQUIRE(memcmp(&expected_b, &test_gpio_b, sizeof(expected_b)) == 0);
        REQUIRE(expected_rcc.APB2ENR == test_rcc.APB2ENR);
        REQUIRE(expected_afio.MAPR == test_afio.MAPR);
    }
    for (i = 0; i < 65536; ++i)
        for (a = 0; a < 8; ++a) {
            test_gpio_a.IDR = a << 10; test_gpio_b.IDR = i;
            REQUIRE(ReadInputs() == old_ReadInputs());
        }
    for (i = 0; i < 65536; ++i) {
        uint32_t initial = Random();
        test_gpio_a.ODR = initial; old_Output_Update((OUT_STATE)i);
        expected_a = test_gpio_a;
        test_gpio_a.ODR = initial; Output_Update((OUT_STATE)i);
        REQUIRE(expected_a.ODR == test_gpio_a.ODR);
    }
    old_SysTick_Init(); expected_tick = test_systick;
    memset(&test_systick, 0, sizeof(test_systick)); SysTick_Init();
    REQUIRE(memcmp(&expected_tick, &test_systick, sizeof(expected_tick)) == 0);
    old_tick_flag = tick_flag = 0;
    old_SysTick_Handler(); SysTick_Handler();
    REQUIRE(tick_flag == 1 && tick_flag == old_tick_flag);
}

int main(void)
{
    TestLogic(); TestHardware();
    printf("PASS: %lu logic frames; GPIO input/output, initialization and SysTick match baseline.\n",
        frames_checked);
    return 0;
}
