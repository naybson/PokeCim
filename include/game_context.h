#pragma once

#include "pokemon.h"

//============================================================
// GameContext
// A central struct to avoid global variables and pass all core game state.
//============================================================
typedef struct {
    Pokemon* pokemons;      // Loaded Pokemon list (official + custom)
    int totalPokemon;

    Move* moveList;         // All loaded moves
    int totalMoves;

    // You can expand this as needed:
    // int currentMenu;
    // BattleSettings battleConfig;
    // SaveData saveFile;

} GameContext;
void cleanup_orphaned_custom_sprites(void);
