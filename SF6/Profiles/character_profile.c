#include "character_profile.h"
/* Fixed selection; runtime character switching is not assigned to a button. */
const CharacterProfile *CharacterProfile_GetActive(void) { return &mai_profile; }
