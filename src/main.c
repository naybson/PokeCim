#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "platform.h"
#include <locale.h>    // For setting UTF-8 locale
#include <time.h>      // For time()

#include "logic.h"
#include "constants.h"
#include "game_context.h"
#include "main_menu.h"

Move* load_move_list_dynamic(const char* filename, int* outCount);

// Global type effectiveness chart, indexed by [attacker][defender]

//=====================================================================
// Function: enable_ansi_escape_codes
// Purpose:
//    Enables ANSI escape code processing on Windows console to support
//    colored and formatted text output.
//
// How:
//    - Sets console output code page to UTF-8 (65001)
//    - Sets locale to UTF-8 for proper character handling
//    - Enables ENABLE_VIRTUAL_TERMINAL_PROCESSING mode for ANSI codes
//=====================================================================
void enable_ansi_escape_codes()
{
    #ifdef _WIN32
    SetConsoleOutputCP(65001);         // Switch console output to UTF-8 codepage
    setlocale(LC_ALL, ".UTF8");        // Set locale to UTF-8 for Unicode handling

    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);  // Get console output handle
    DWORD dwMode = 0;
    GetConsoleMode(hOut, &dwMode);                    // Get current console mode
    dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;    // Enable ANSI escape sequences
    SetConsoleMode(hOut, dwMode);                     // Apply updated console mode
#endif /* _WIN32 */
}

//=====================================================================
// Function: main
// Purpose:
//    Entry point of the program. Sets up console, loads game data,
//    cleans up orphaned sprites, and launches main menu loop.
//
// Steps:
//    1) Enable UTF-8 and ANSI codes for console output.
//    2) Initialize random seed for game randomness.
//    3) Initialize Pokemon type chart.
//    4) Load Pokemon list from file.
//    5) Load move list from file.
//    6) Clean up any orphaned custom sprites from previous runs.
//    7) Enter the main menu loop to interact with the user.
//    8) Cleanup allocated resources before exiting.
//
//=====================================================================
int main()
{
    // Setup console for UTF-8 and ANSI codes (colors, special chars)
    enable_ansi_escape_codes();

    // Seed random number generator with current time for randomness
    srand((unsigned int)time(NULL));

    // Initialize the main game context structure (zero-initialize)
    GameContext context = { 0 };

    // Load the combined Pokemon list file into memory
    context.pokemons = load_pokemon_list(POKEMON_LIST_FILE, &context.totalPokemon);
    if (!context.pokemons || context.totalPokemon == 0) 
    {
        printf("No Pokemon loaded.\n");
        return 1;  // Exit with failure if loading fails
    }

    // Load the list of all moves from the moves list file
    context.moveList = load_move_list_dynamic(MOVE_LIST_FILE, &context.totalMoves);
    if (!context.moveList || context.totalMoves == 0) {
        printf("Failed to load moves.\n");
        free(context.pokemons);  // Free already loaded Pokemon data
        return 1;  // Exit with failure
    }

    // Cleanup any custom sprites on disk that no longer correspond to valid Pokemon
    cleanup_orphaned_custom_sprites();

    // Start the main interactive menu (blocks until user exits)
    main_menu(&context);

    // Free dynamically allocated Pokemon and move data before exiting
    free(context.pokemons);
    free(context.moveList);

    return 0; // Program exited successfully
}
