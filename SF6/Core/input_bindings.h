#ifndef SF6_INPUT_BINDINGS_H
#define SF6_INPUT_BINDINGS_H
#include "controller_types.h"
/* Current firmware roles, not a claim about physical button positions. */
#define BIND_UP IN_SP2
#define BIND_DOWN IN_DOWN
#define BIND_LEFT IN_SUPER
#define BIND_RIGHT IN_RIGHT
#define BIND_SPECIAL IN_SP1 /* Reserved; no move assigned yet. */
#define BIND_SUPER IN_X
#define BIND_DIRECTIONS (BIND_UP | BIND_DOWN | BIND_LEFT | BIND_RIGHT)
#define BIND_MEDIUM_PAIR IN_COMBO1
#endif
