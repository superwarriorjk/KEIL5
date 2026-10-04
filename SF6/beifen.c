/*#include "stm32f10x.h"
#include <stdint.h>


typedef uint32_t IN_STATE;
typedef uint16_t OUT_STATE;

volatile IN_STATE debug_in = 0;

// ???
#define IN_UP         (1U << 0)
#define IN_DOWN       (1U << 1)
//#define IN_LEFT       (1U << 2)
#define IN_RIGHT      (1U << 3)

// ???
#define IN_LP         (1U << 4)
#define IN_MP         (1U << 5)
#define IN_HP         (1U << 6)
#define IN_LK         (1U << 7)
#define IN_MK         (1U << 8)
#define IN_HK         (1U << 9)

// ???
#define IN_SUPER      (1U << 10)

// SP ??
#define IN_SP1        (1U << 11)
#define IN_SP2        (1U << 12)
#define IN_SP3        (1U << 13)
#define IN_SP4        (1U << 14)

// COMBO ??
#define IN_COMBO1     (1U << 16)
#define IN_COMBO2     (1U << 17)
#define IN_COMBO3     (1U << 18)
//#define IN_LA1        (1u << 19)

// X ???
#define IN_X          (1U << 15)

#define OUT_UP        (1U << 0)
#define OUT_DOWN      (1U << 1)
#define OUT_LEFT      (1U << 2)
#define OUT_RIGHT     (1U << 3)
#define OUT_LP        (1U << 4)
#define OUT_MP        (1U << 5)
#define OUT_HP        (1U << 6)
#define OUT_LK        (1U << 7)
#define OUT_MK        (1U << 8)
#define OUT_HK        (1U << 9)

#define FPS 60
volatile uint8_t tick_flag = 0;

void SysTick_Init(void)
{
    // STM32F103C8T6 ???? 72 MHz
    // SysTick ??????? HCLK = 72 MHz
    // ?? = 1/60 ? = 16.6667 ms
    // SysTick ???? = 72 MHz * 0.0166667 ˜ 1200000

    SysTick->LOAD  = 1200000 - 1; // ????
    SysTick->VAL   = 0;           // ?????
    SysTick->CTRL  = SysTick_CTRL_CLKSOURCE_Msk |  // HCLK
                     SysTick_CTRL_TICKINT_Msk   |  // ????
                     SysTick_CTRL_ENABLE_Msk;      // ??
}

// SysTick ??????
void SysTick_Handler(void)
{
    tick_flag = 1; // ????
}
void GPIO_Init_All(void)
{
    RCC->APB2ENR |= RCC_APB2ENR_AFIOEN;

    // ?? JTAG,?? SWD
    AFIO->MAPR |= AFIO_MAPR_SWJ_CFG_1;

    // ??? GPIO ??
    
    RCC->APB2ENR |= RCC_APB2ENR_IOPAEN;
    RCC->APB2ENR |= RCC_APB2ENR_IOPBEN;

    

 GPIOA->CRL &= ~(0xFFFFFFFF);   // ??
    GPIOA->CRL |= 0x77777777;      // ??????,50MHz

    // PA8~PA9
    GPIOA->CRH &= ~(0xFF);         // ?? PA8~PA9 ??
    GPIOA->CRH |= 0x77;            // ??????,50MHz

    // ??????
    GPIOA->ODR |= 0x03FF;          // PA0~PA9 ???(???)

    ************ ?? PB0~PB15(????) ************
    // PB0~PB7
    GPIOB->CRL &= ~(0xFFFFFFFF);
    GPIOB->CRL |= 0x88888888;      // ????

    // PB8~PB15
    GPIOB->CRH &= ~(0xFFFFFFFF);
    GPIOB->CRH |= 0x88888888;      // ????

    // ??????
    GPIOB->ODR |= 0xFFFF;

    ************ ?? PA10~PA12(????,?????) ************
    // PA10~PA12 ? CRH ?
   GPIOA->CRH &= ~((0xF << 8) | (0xF << 12) | (0xF << 16));
GPIOA->CRH |=  (0x8 << 8) | (0x8 << 12) | (0x8 << 16);
GPIOA->ODR |=  (1<<10) | (1<<11) | (1<<12);
}


IN_STATE ReadInputs(void)
{
   IN_STATE state = 0;

    uint32_t pb = ~GPIOB->IDR; // ???1
    uint32_t pa = ~GPIOA->IDR;

    // PB0~PB15
    state |= (pb & 0xFFFF);      // PB0~PB15

    // PA10~PA12 -> ??? IN_COMBO1~3 ??? 16~18
    state |= ((pa >> 10) & 0x7) << 16;

    return state;


}
#define MAX_FRAMES      4
#define HOLD_THRESHOLD  10      // 10?˜167ms(60FPS)
#define QUEUE_LEN       4

// ----------------- ???? -----------------
#define ACT_SP1        1
#define ACT_SP2        2
#define ACT_SP3        3
#define ACT_SP4        4
#define ACT_SUPER      5

#define ACT_SP1_HOLD   11
#define ACT_SP2_HOLD   12
#define ACT_SP3_HOLD   13
#define ACT_SP4_HOLD   14

// ----------------- ???? -----------------
typedef struct {
    uint8_t pressed;
    uint8_t prev;
    uint16_t hold_frames;
} KeyState;

KeyState sp1_key = {0};
KeyState sp2_key = {0};
KeyState sp3_key = {0};
KeyState sp4_key = {0};
KeyState super_key = {0};

// ----------------- ???? -----------------
typedef struct {
    uint8_t active;
    uint8_t type;
    uint8_t frame;
    uint8_t dir;     // 0=? 1=? 2=?
} ActionState;

typedef struct {
    uint8_t type;
    uint8_t dir;
} Action;

Action action_queue[QUEUE_LEN] = {0};
uint8_t queue_head = 0;
uint8_t queue_tail = 0;

ActionState current_action = {0,0,0,0};
static void UpdateKey(KeyState *k, uint8_t is_pressed)
{
    k->prev = k->pressed;
    k->pressed = is_pressed;

    if(k->pressed)
        k->hold_frames++;
    else
        k->hold_frames = 0;
}

void EnqueueAction(uint8_t type, uint8_t dir)
{
    uint8_t next = (queue_tail + 1) % QUEUE_LEN;

    if(next != queue_head)   // ????
    {
        action_queue[queue_tail].type = type;
        action_queue[queue_tail].dir  = dir;
        queue_tail = next;
    }
}

static void MirrorDirection(uint32_t *left, uint32_t *right, uint8_t mirror)
{
    if(mirror)
    {
        uint32_t tmp = *left;
        *left  = *right;
        *right = tmp;
    }
}

int test_flag=10;
OUT_STATE Logic_Process(IN_STATE in)
{
    OUT_STATE out = 0;
    uint8_t mirror = (in & IN_X) ? 1 : 0;

    // ----------------- ?????? -----------------
   UpdateKey(&sp1_key, (in & IN_SP1) ? 1 : 0);
	UpdateKey(&sp2_key, (in & IN_SP2) ? 1 : 0);
	UpdateKey(&sp3_key, (in & IN_SP3) ? 1 : 0);
	UpdateKey(&sp4_key, (in & IN_SP4) ? 1 : 0);
	UpdateKey(&super_key, (in & IN_SUPER) ? 1 : 0);
    

    // ----------------- ???? -----------------
    uint8_t dir = 3;
    if(in & IN_DOWN) dir = 0;
   // else if(in & IN_LEFT) dir = 1;
    else if(in & IN_RIGHT) dir = 2;
	
	
	
	if(in & IN_SP1)
{
    test_flag = 1;
}
else
{
    test_flag = 0;
}
	
	
	
	

    // ----------------- ????? -----------------
    if(!sp1_key.prev && sp1_key.pressed)
        EnqueueAction(
            (sp1_key.hold_frames >= HOLD_THRESHOLD) ? ACT_SP1_HOLD : ACT_SP1,
            dir
        );

    if(!sp2_key.prev && sp2_key.pressed)
        EnqueueAction(
            (sp2_key.hold_frames >= HOLD_THRESHOLD) ? ACT_SP2_HOLD : ACT_SP2,
            dir
        );

    if(!sp3_key.prev && sp3_key.pressed)
        EnqueueAction(
            (sp3_key.hold_frames >= HOLD_THRESHOLD) ? ACT_SP3_HOLD : ACT_SP3,
            dir
        );

    if(!sp4_key.prev && sp4_key.pressed)
        EnqueueAction(
            (sp4_key.hold_frames >= HOLD_THRESHOLD) ? ACT_SP4_HOLD : ACT_SP4,
            dir
        );

    if(!super_key.prev && super_key.pressed)
        EnqueueAction(ACT_SUPER, dir);

    // ----------------- ????? -----------------
    if(!current_action.active && queue_head != queue_tail)
    {
        current_action.active = 1;
        current_action.type   = action_queue[queue_head].type;
        current_action.dir    = action_queue[queue_head].dir;
        current_action.frame  = 0;

        queue_head = (queue_head + 1) % QUEUE_LEN;
    }

    // ----------------- ???? -----------------
    if(current_action.active)
    {
        uint32_t down  = OUT_DOWN;
        uint32_t left  = OUT_LEFT;
        uint32_t right = OUT_RIGHT;

        MirrorDirection(&left, &right, mirror);

        switch(current_action.type)
        {
            // ================= SP NORMAL =================
            // ================= SP1 =================
case ACT_SP1:
{
    switch(current_action.frame)
    {
        case 0: out |= down; break;
        case 1: out |= (down | left); break;
        case 2: out |= left; break;
        case 3:
            switch(current_action.dir)
            {
                case 0: out |= OUT_MP; break;
                case 1: out |= OUT_LP; break;
                case 2: out |= OUT_MP | OUT_HP; break;
                default: out |= OUT_HP; break;
            }
            break;
    }
}
break;

// ================= SP2 =================
case ACT_SP2:
{
    switch(current_action.frame)
    {
        case 0: out |= right; break;
        case 1: out |= down; break;
        case 2: out |= (down | right);break;
        case 3:
            switch(current_action.dir)
            {
                case 0: out |= OUT_MK; break;
                case 1: out |= OUT_LK; break;
                case 2: out |= OUT_MK | OUT_HK;break;
                default: out |= OUT_HK; break;
            }
            break;
    }
}
break;

// ================= SP3 =================
case ACT_SP3:
{
    switch(current_action.frame)
    {
        case 0: out |= down; break;
        case 1: out |= (down | left); break;
        case 2: out |= left; break;
        case 3:
            switch(current_action.dir)
            {
                case 0: out |= OUT_MK; break;
                case 1: out |= OUT_LK; break;
                case 2: out |= OUT_HK; break;
                default: out |= OUT_MK | OUT_HK; break;
            }
            break;
    }
}
break;

// ================= SP4 =================
case ACT_SP4:
{
    switch(current_action.frame)
    {
        case 0: out |= down; break;
        case 1: out |= (down | right); break;
        case 2: out |= right; break;
        case 3:
            switch(current_action.dir)
            {
                case 0: out |= OUT_MK; break;
                case 1: out |= OUT_LK; break;
                case 2: out |= OUT_HK; break;
                default: out |= OUT_MK | OUT_HK; break;
            }
            break;
    }
}
break;

            // ================= SP HOLD ?? =================
            case ACT_SP1_HOLD:
            case ACT_SP2_HOLD:
            case ACT_SP3_HOLD:
            case ACT_SP4_HOLD:
            {
                uint8_t reverse = (current_action.type==ACT_SP2_HOLD || current_action.type==ACT_SP4_HOLD);

                switch(current_action.frame)
                {
                    case 0: out |= down; break;
                    case 1: out |= reverse ? (down|right):(down|left); break;
                    case 2: out |= reverse ? right:left; break;
                    case 3:
                        switch(current_action.dir)
                        {
                            case 0: out |= OUT_MP|OUT_MK; break;
                            case 1: out |= OUT_LP|OUT_LK; break;
                            case 2: out |= OUT_HP|OUT_HK; break;
                            default: out |= OUT_MP|OUT_HP|OUT_MK; break;
                        }
                        break;
                }
                break;
            }

            // ================= SUPER =================
            case ACT_SUPER:
{
    // ? 0~4 ?????
    if(current_action.frame <= 6)
{
    switch(current_action.dir)
    {
        case 0:
        {
            switch(current_action.frame)
            {
                case 0: out |= down; break;
                case 1: out |= (down | left); break;
                case 2: out |= left; break;
                case 3: out |= down; break;
                case 4: out |= (down | left); break;
                case 5: out |= left; break;
                case 6: out |= OUT_HP; break;
            }
        }
        break;

        case 2:
        {
            switch(current_action.frame)
            {
                case 0: out |= down; break;
                case 1: out |= (down | right); break;
                case 2: out |= right; break;
                case 3: out |= down; break;
                case 4: out |= (down | right); break;
                case 5: out |= right; break;
                case 6: out |= OUT_HP; break;
            }
        }
        break;
				case 1:
        {
            switch(current_action.frame)
            {
                case 0: out |= down; break;
                case 1: out |= (down | right); break;
                case 2: out |= right; break;
                case 3: out |= down; break;
                case 4: out |= (down | right); break;
                case 5: out |= right; break;
                case 6: out |= OUT_HK; break;
            }
        }
        break;
    }
}
    
}
break;
        }

        current_action.frame++;

        // -------- ???? --------
        if(current_action.type>=1 && current_action.type<=4 && current_action.frame>=MAX_FRAMES)
{
    current_action.active = 0;
    current_action.frame  = 0;
}
else if(current_action.type>=11 && current_action.type<=14 && current_action.frame>=MAX_FRAMES)
{
    current_action.active = 0;
    current_action.frame  = 0;
}
else if(current_action.type==ACT_SUPER && current_action.frame>=7)
{
    current_action.active = 0;
    current_action.frame  = 0;
}

        return out;
    }

    // ----------------- ?????? -----------------
    if(in & IN_UP)    out |= OUT_UP;
    if(in & IN_DOWN)  out |= OUT_DOWN;
    //if(in & IN_LEFT)  out |= OUT_LEFT;
    if(in & IN_RIGHT) out |= OUT_RIGHT;

    if(in & IN_LP) out |= OUT_LP;
    if(in & IN_MP) out |= OUT_MP;
    if(in & IN_HP) out |= OUT_HP;
    if(in & IN_LK) out |= OUT_LK;
    if(in & IN_MK) out |= OUT_MK;
    if(in & IN_HK) out |= OUT_HK;
		if(in & IN_COMBO1) out |= OUT_MP|OUT_MK;
		//if(in & IN_SUPER) out |= OUT_DOWN|OUT_LEFT|OUT_LP;

    return out;
}
void Output_Update(OUT_STATE out)
{
    GPIOA->ODR |= 0x03FF;          // 0x03FF = PA0~PA9

    // ????????????
    GPIOA->ODR &= ~(out & 0x03FF); // out ????1 ? ??
}


int main(void)
{
    GPIO_Init_All();
    SysTick_Init();  // ??? 60FPS SysTick

    while(1)
    {
       

debug_in = ReadInputs();

			if(tick_flag)  // ????
        {
            tick_flag = 0;

            IN_STATE in  = ReadInputs();
					
            OUT_STATE out = Logic_Process(in);
            Output_Update(out);
        }
    }
}
*/
