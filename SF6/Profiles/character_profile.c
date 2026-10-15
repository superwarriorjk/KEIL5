#include "character_profile.h"

/* Default remains the user's working configuration. Future profiles belong
 * in separate files; a future selector should switch only when queue/player
 * are idle so queued actions cannot silently change character semantics. */
const CharacterProfile *CharacterProfile_GetActive(void) { return &current_profile; }
