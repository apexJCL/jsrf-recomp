/*
 * recomp_env_game.h - this game's own environment keys, added to the
 * toolkit's table (xboxrecomp src/platform/recomp_env.h, which includes this
 * file when RECOMP_ENV_HAVE_GAME_KEYS is defined; CMakeLists.txt sets it on
 * the recomp_env target). Same row format: X(ID, TIER, NAME, LEGACY, HELP).
 * Inventory with meanings: docs/env.md.
 */
#ifndef RECOMP_ENV_GAME_H
#define RECOMP_ENV_GAME_H

#define RECOMP_ENV_GAME_KEYS(X) \
    X(FIX_DJ_STAGE_WORD, CONFIG, "RECOMP_FIX_DJ_STAGE_WORD", NULL, "fixes: fix.dj_stage_word, the stage switch after the DJ event uses the word cached at setup when the freed record no longer holds a stage word, on (default) or off (1/0, true/false, yes/no)") \
    X(FIX_MISSION_BUFFER, CONFIG, "RECOMP_FIX_MISSION_BUFFER", NULL, "fixes: fix.mission_buffer, a loader release does not free the mission buffer a type-0 entry still uses, on (default) or off (1/0, true/false, yes/no)")

#endif /* RECOMP_ENV_GAME_H */
