#ifndef SF6_CONTROLLER_TYPES_H
#define SF6_CONTROLLER_TYPES_H
#include <stdint.h>
typedef uint32_t IN_STATE;
typedef uint16_t OUT_STATE;
/* Legacy bit names are preserved; see input_bindings.h for actual roles. */
#define IN_UP (1U << 0)
#define IN_DOWN (1U << 1)
#define IN_RIGHT (1U << 3)
#define IN_LP (1U << 4)
#define IN_MP (1U << 5)
#define IN_HP (1U << 6)
#define IN_LK (1U << 7)
#define IN_MK (1U << 8)
#define IN_HK (1U << 9)
#define IN_SUPER (1U << 10)
#define IN_SP1 (1U << 11)
#define IN_SP2 (1U << 12)
#define IN_SP3 (1U << 13)
#define IN_SP4 (1U << 14)
#define IN_COMBO1 (1U << 16)
#define IN_COMBO2 (1U << 17)
#define IN_COMBO3 (1U << 18)
#define IN_X (1U << 15)
#define OUT_UP (1U << 0)
#define OUT_DOWN (1U << 1)
#define OUT_LEFT (1U << 2)
#define OUT_RIGHT (1U << 3)
#define OUT_LP (1U << 4)
#define OUT_MP (1U << 5)
#define OUT_HP (1U << 6)
#define OUT_LK (1U << 7)
#define OUT_MK (1U << 8)
#define OUT_HK (1U << 9)
#endif
