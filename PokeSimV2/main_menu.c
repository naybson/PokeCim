#include "main_menu.h"
#include "game_context.h"
#include "constants.h"

//=====================================================================
// Function: show_pokedex_menu
// Purpose:
//    Displays and manages the Pokedex viewing system with scrollable search.
//
// Parameters:
//    context - Pointer to the game context which includes loaded Pokemon and moves.
//
// Notes:
//    - Uses a scrollable list display to view filtered Pokemon.
//    - Search supports filtering by name and type.
//    - ESC key exits the Pokedex view.
//    - The currently selected index and scroll start are preserved across redraws.
//=====================================================================
void show_pokedex_menu(GameContext* context)
{
    // Initial search setup (name-based search by default)
    SearchState pokedexSearch = {
        .mode = SEARCH_NAME,
        .name_filter = "",
        .type_filter = POKITYPE_NONE
    };

    int selectedIndex = 0;  // Tracks which Pokemon is selected/highlighted
    int scrollStart = 0;    // Tracks first visible entry on screen

    while (1) // Loop to keep the Pokedex menu active
    {
        // Draw the menu and return selected Pokemon index or -1 for exit
        int selected = display_menu(context->pokemons, context->totalPokemon, &pokedexSearch, &selectedIndex, &scrollStart);

        if (selected == -1)
            break; // Exit the Pokedex view if user hits ESC

        // If a valid Pokemon was selected, show its interactive detail view
        if (selected >= 0 && selected < context->totalPokemon)
        {
            print_pokemon_details_interactive(&context->pokemons[selected], context->moveList, context->totalMoves);
        }
    }
}


//=====================================================================
// Function: show_custom_creation_menu
// Purpose:
//    Guides the user through the process of creating custom Pokemon.
//    After each successful creation, the Pokemon list is reloaded.
//
// Parameters:
//    context - Pointer to the game context containing the Pokemon and move list.
//
// Notes:
//    - Uses _getch() to detect keystrokes.
//    - ESC exits back to main menu.
//    - ENTER allows continuous creation.
//    - Uses constants KEY_ESC, KEY_SPECIAL_0, KEY_SPECIAL_224 for key handling.
//=====================================================================
void show_custom_creation_menu(GameContext* context)
{
    while (1) // Loop to allow creating multiple custom Pokemon
    {
        // Launch the creation form and wait for user action
        int result = create_custom_pokemon(context->moveList, context->totalMoves);

        // Exit if user canceled or backed out
        if (result == 0)
            break;

        // Replace current Pokemon list with updated list from file
        free(context->pokemons);
        context->pokemons = load_pokemon_list("pokemon_list_combined.txt", &context->totalPokemon);

        // If loading failed, inform the user and exit loop
        if (!context->pokemons || context->totalPokemon == 0)
        {
            printf("Failed to reload Pokemon list.\n");
            _getch();
            break;
        }

        // Notify user of success and prompt for next action
        printf("\n Custom Pokemon added!\n");
        printf("Press ESC to return to menu or ENTER to create another.\n");

        // Wait for key input to decide what to do next
        int input = _getch();

        // If input is part of a special key combo, discard prefix
        if (input == KEY_SPECIAL_0 || input == KEY_SPECIAL_224)
            input = _getch();

        // ESC exits back to main menu
        if (input == KEY_ESC)
            break;
    }
}

//=====================================================================
// Function: main_menu
// Purpose:
//    Displays the main interactive menu of the program, allowing the user
//    to choose between viewing the Pokedex, creating a custom Pokemon,
//    or accessing a (WIP) battle simulator.
//
// Parameters:
//    context - Pointer to the global game context, containing all loaded data
//
// Notes:
//    - Uses _getch() to read input without requiring ENTER.
//    - Uses system("cls") to clear the terminal (Windows-only).
//    - Repeats the menu until the user presses ESC.
//    - KEY_ESC is used to detect ESC key (defined in constants.h).
//=====================================================================
void main_menu(GameContext* context)
{
    while (1) // Menu loop that runs until the user exits with ESC
    {
        system("cls"); // Clear the screen on each menu draw (Windows only)

        // === Print the main menu UI ===
        printf("=============================\n");
        printf("        PokeSim \n");
        printf("=============================\n");
        printf("[1] View Pokedex\n");
        printf("[2] Create Custom Pokemon (Demo)\n");
        printf("[3] View Credits\n");
        printf("[ESC] Exit\n");
        printf("=============================\n");
        printf("Select an option: ");

        int input = _getch(); // Read a single character from the user

        if (input == KEY_ESC) // Check for ESC key (exit trigger)
        {
            printf("\nExiting PokeSim...\n");
            break; // Exit the loop and return from main_menu
        }

        switch (input)
        {
        case '1':
            // View Pokedex menu
            show_pokedex_menu(context);
            break;

        case '2':
            // Launch the custom Pokemon creation menu
            show_custom_creation_menu(context);
            break;
        case '3':
            show_credits_screen();
            break;
        default:
            // Invalid key handling
            printf("\nInvalid selection.\n");
            _getch(); // Pause so the user sees the error
            break;
        }
    }
}


void show_credits_screen()
{
    system("cls"); // Clear screen on Windows

    printf("\x1B[1m\x1B[36m============================\n");
    printf("         PokeSim Credits       \n");
    printf("============================\x1B[0m\n\n");

    printf("\x1B[1mDeveloped by:\x1B[0m Niv Shsmesh\n");
    printf("\x1B[1mLanguage:\x1B[0m C\n");
    printf("\x1B[1mIDE:\x1B[0m Visual Studio 2022\n");
    printf("\x1B[1mTerminal Graphics:\x1B[0m ANSI Escape Sequences\n\n");

    printf("\x1B[1mPokemon data inspired by:\x1B[0m\n");
    printf("- PokeWiki and Bulbapedia\n");
    printf("- Sprite format adapted from:\n");
    printf("  https://gitlab.com/phoneybadger/pokemon-colorscripts\n");
    printf("  (Heavily modified to fit project requirements)\n\n");

    printf("\x1B[1mFusion Sprites:\x1B[0m\n");
    printf("- From Pokemon Infinite Fusion community project\n");
    printf("  https://pokemoneinfinitefusion.com/pokemon-infinite-fusion-sprite-packs/\n");
    printf("  (Full credit to all contributing artists)\n\n");

    printf("\x1B[1mSpecial Thanks:\x1B[0m\n");
    printf("- Nick – my cat and spiritual co-pilot,\n");
    printf("  \x1B[91mwho sat on my keyboard and deleted key files at least twice\x1B[0m\n");
    printf("- Zheypr (ChatGPT) – project co-designer, debugger, pixel whisperer\n");
    printf("- Pokemon and ANSI art communities\n");
    printf("- Open-source data warriors\n");
    printf("- \x1B[93mCoffee. Seriously. Thank you.\x1B[0m\n\n");

    printf("\x1B[1mCourse:\x1B[0m Introduction to Computer Science – HIT\n");
    printf("\x1B[1mLecturer:\x1B[0m Gilad Shamir\n\n");

    printf("\x1B[3mPress any key to return...\x1B[0m\n");
    getch();
}
