#pragma once

#include "logic.h"    // for Pokemon, Move
#include "pokedex.h"  // for display_menu() and search state


// ========== MAIN MENU ==========

// Shows the main menu and dispatches to Pokedex, Creator, or Exit
void main_menu(GameContext* context);

void show_pokedex_menu(GameContext* context);

void show_custom_creation_menu(GameContext* context);

void show_credits_screen();