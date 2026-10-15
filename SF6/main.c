#include "Board/board_io.h"
#include "Board/frame_clock.h"
#include "Core/controller_logic.h"

volatile IN_STATE debug_in = 0;

int main(void)
{
    GPIO_Init_All();
    SysTick_Init();
    while (1)
    {
        debug_in = ReadInputs();
        if (tick_flag)
        {
            IN_STATE in;
            OUT_STATE out;
            tick_flag = 0;
            in = ReadInputs();
            out = Logic_Process(in);
            Output_Update(out);
        }
    }
}
