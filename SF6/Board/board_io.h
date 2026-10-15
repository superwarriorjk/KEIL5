#ifndef SF6_BOARD_IO_H
#define SF6_BOARD_IO_H
#include "../Core/controller_types.h"
void GPIO_Init_All(void);
IN_STATE ReadInputs(void);
void Output_Update(OUT_STATE out);
#endif
