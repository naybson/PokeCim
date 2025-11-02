// NOTE: This is the **refactored version** of your create_custom_pokemon system.
// All helper logic is split into small focused functions for maintainability and sanity.

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <conio.h>
#include "pokemon.h"
#include "graphics.h"
#include "constants.h"
#include "game_context.h"
#include "Pokedex.h"
#include "Custom_pokemon.h"

// ========== MAIN ENTRY ==========

//=====================================================================
// Function: create_custom_pokemon
// Purpose:
//    Allows the user to create a custom Pokemon via an interactive terminal UI.
//    Users input name, stats, types, and moves, which are validated and saved.
//
// Parameters:
//    moveList   - A pointer to the list of available moves
//    moveCount  - The total number of moves in the moveList
//
// Returns:
//    CREATE_SUCCESS (1)   - If the Pokemon is successfully created and saved
//    CREATE_CANCELLED (0) - If the user cancels the creation process
//=====================================================================
int create_custom_pokemon(Move* moveList, int moveCount)
{
    // Generate a unique custom ID (e.g., C123)
    char pokemonID[10];
    snprintf(pokemonID, sizeof(pokemonID), "C%03d", rand() % 900 + 100);

    // Initialize custom Pokemon fields with default values
    char name[32] = "";
    int hp = 0, atk = 0, def = 0, spAtk = 0, spDef = 0, speed = 0;
    Type type1 = POKITYPE_NORMAL, type2 = POKITYPE_NONE;
    char moveNames[4][20] = { "NOMOVE", "NOMOVE", "NOMOVE", "NOMOVE" };

    // Load the current list of Pokemon for validation (e.g., checking for duplicates)
    int totalExisting = 0;
    Pokemon* existingList = load_pokemon_list(POKEMON_LIST_FILE, &totalExisting);
    if (!existingList) {
        printf(" ERROR: Failed to load existing Pokemon list!\n");
        exit(1);
    }

    int selected = 0; // Tracks which field is currently focused in the form
    int ch;           // Stores the current key input

    // === Main input loop ===
    while (1)
    {
        // Validate the current Pokemon state
        ValidationResult validation = validate_custom_pokemon(
            name, type1, type2, hp, atk, def, spAtk, spDef, speed,
            moveNames, existingList, totalExisting);

        // Calculate total stat score and get visual grade
        int totalStats = calculate_total_stats(hp, atk, def, spAtk, spDef, speed);
        const char* color;
        const char* grade = grade_from_total(totalStats, &color);

        // === UI Rendering ===
        system("cls");
        render_header(totalStats, grade, color); // Shows stat summary at top
        printf("POKEMON ID: %s\n", pokemonID);   // Show generated ID

        // Renders the main form with current values and validation info
        render_form(name, type1, type2, hp, atk, def, spAtk, spDef, speed,
            moveNames, selected, &validation, moveList, moveCount, pokemonID);

        render_messages(&validation); // Print validation messages under the form

        // Instructions to user
        printf("\n[UP/DOWN] to navigate, [LEFT/RIGHT] to change types, [ENTER] to edit/select/submit\n");
        printf("\n[Press ESC To return to main menu]");

        // === Handle user input ===
        ch = _getch();
        if (ch == KEY_SPECIAL_0 || ch == KEY_SPECIAL_224) ch = _getch();

        // --- ENTER pressed ---
        if (ch == KEY_ENTER) {
            // If user selects the SUBMIT field and everything is valid
            if (selected == FIELD_SUBMIT && validation.canSubmit) {
                FILE* f = fopen(POKEMON_LIST_FILE, "a");
                if (f) {
                    fprintf(f, "\n%s %s %s %s %d %d %d %d %d %d %s %s %s %s",
                        pokemonID, name,
                        type_to_string(type1), type_to_string(type2),
                        hp, atk, def, spAtk, spDef, speed,
                        moveNames[0], moveNames[1], moveNames[2], moveNames[3]);
                    fclose(f);
                }
                printf("\n[V] Pokemon saved! Press any key to return.\n");
                _getch();
                break; // Exit loop and return success
            }
            else if (selected != FIELD_SUBMIT) {
                // Otherwise, allow editing the currently selected field
                bool shouldExit = !edit_field(selected, name, &hp, &atk, &def, &spAtk, &spDef, &speed,
                    moveNames, moveList, moveCount, pokemonID);
                if (shouldExit) break; // User cancelled mid-edit
            }
        }
        else {
            // Any other input = navigation or type adjustment
            handle_input(ch, &selected, &type1, &type2, pokemonID);
        }

        // --- ESC pressed ---
        if (ch == KEY_ESC) {
            printf("\n[!] Creation cancelled. Returning to menu...\n");
            _getch();
            return CREATE_CANCELLED;
        }
    }

    // Clean up dynamically allocated memory
    if (existingList) free(existingList);
    return CREATE_SUCCESS;
}

// ========== INPUT + STATE HANDLING ==========

//=====================================================================
// Function: handle_input
// Purpose:
//    Processes key presses for navigation and type selection during
//    custom Pokemon creation. Updates the selected field and type values.
//
// Parameters:
//    ch         - The key code entered by the user
//    selected   - Pointer to the currently selected field index
//    type1      - Pointer to the first Pokemon type
//    type2      - Pointer to the second Pokemon type
//    pokemonID  - (Currently unused, reserved for future functionality)
//
// Returns:
//    void (modifies values through pointer arguments)
//=====================================================================
void handle_input(int ch, int* selected, Type* type1, Type* type2, const char* pokemonID)
{
    // Navigate upward in the field list
    if (ch == KEY_ARROW_UP)
    {
        (*selected)--;

        // Prevent moving above the first field
        if (*selected < 0)
        {
            *selected = 0;
        }
    }

    // Navigate downward in the field list
    else if (ch == KEY_ARROW_DOWN)
    {
        (*selected)++;

        // Prevent moving beyond the last field
        if (*selected >= FIELD_COUNT)
        {
            *selected = FIELD_COUNT - 1;
        }
    }

    // Handle left/right input for type selection
    else if (*selected == FIELD_TYPE1 || *selected == FIELD_TYPE2)
    {
        // Cycle type left (decrease value, wrap around)
        if (ch == KEY_ARROW_LEFT)
        {
            if (*selected == FIELD_TYPE1)
            {
                if (*type1 == 0)
                {
                    *type1 = POKITYPE_COUNT - 1;
                }
                else
                {
                    (*type1)--;
                }
            }
            else // FIELD_TYPE2
            {
                if (*type2 == 0)
                {
                    *type2 = POKITYPE_COUNT - 1;
                }
                else
                {
                    (*type2)--;
                }
            }
        }

        // Cycle type right (increase value, wrap around)
        else if (ch == KEY_ARROW_RIGHT)
        {
            if (*selected == FIELD_TYPE1)
            {
                *type1 = (*type1 + 1) % POKITYPE_COUNT;
            }
            else // FIELD_TYPE2
            {
                *type2 = (*type2 + 1) % POKITYPE_COUNT;
            }
        }
    }
}

//=====================================================================
// Function: edit_field
// Purpose:
//    Handles editing of each individual field during custom Pokemon creation.
//    Depending on the selected field, prompts user for input or opens submenus.
//
// Parameters:
//    selected     - Index of the currently selected field
//    name         - Buffer to store the Pokemon's name
//    hp, atk, def, spAtk, spDef, speed - Pointers to stat values to be edited
//    moveNames    - Array of move name strings
//    moveList     - Array of available Move structs
//    moveCount    - Number of moves in moveList
//    pokemonID    - Unique ID of the Pokemon being edited (used for sprite saving)
//
// Returns:
//    true         - Always returns true except when user should exit editing
//=====================================================================
bool edit_field(int selected, char* name, int* hp, int* atk, int* def, int* spAtk, int* spDef, int* speed,
    char moveNames[4][20], Move* moveList, int moveCount, const char* pokemonID)
{
    switch (selected)
    {
    case FIELD_NAME:
    {
        printf("\nEnter Name: ");
        fgets(name, 32, stdin);

        // Remove newline character if present
        name[strcspn(name, "\n")] = 0;
        break;
    }

    case FIELD_HP:
    {
        printf("Enter HP: ");
        scanf_s("%d", hp);
        getchar();
        break;
    }

    case FIELD_ATTACK:
    {
        printf("Enter Attack: ");
        scanf_s("%d", atk);
        getchar();
        break;
    }

    case FIELD_DEFENSE:
    {
        printf("Enter Defense: ");
        scanf_s("%d", def);
        getchar();
        break;
    }

    case FIELD_SPATK:
    {
        printf("Enter Sp. Atk: ");
        scanf_s("%d", spAtk);
        getchar();
        break;
    }

    case FIELD_SPDEF:
    {
        printf("Enter Sp. Def: ");
        scanf_s("%d", spDef);
        getchar();
        break;
    }

    case FIELD_SPEED:
    {
        printf("Enter Speed: ");
        scanf_s("%d", speed);
        getchar();
        break;
    }

    case FIELD_MOVE_1:
    {
        select_single_move(moveNames[0], moveNames, 0, moveList, moveCount);
        break;
    }

    case FIELD_MOVE_2:
    {
        select_single_move(moveNames[1], moveNames, 1, moveList, moveCount);
        break;
    }

    case FIELD_MOVE_3:
    {
        select_single_move(moveNames[2], moveNames, 2, moveList, moveCount);
        break;
    }

    case FIELD_MOVE_4:
    {
        select_single_move(moveNames[3], moveNames, 3, moveList, moveCount);
        break;
    }

    case FIELD_SPRITE_MAKER:
    {
        // Launch sprite creator UI using the Pokemon's unique ID
        creature_selector_menu_with_id(pokemonID);
        break;
    }

    case FIELD_SUBMIT:
    {
        // SUBMIT is a signal to exit editing and try saving
        return true;
    }
    }

    // In all other cases, continue editing
    return true;
}

// ========== RENDERING ==========
//=====================================================================
// Function: render_form
// Purpose:
//    Renders the entire interactive form used for custom Pokemon creation.
//    Highlights the selected field, shows validation markers, and renders
//    appropriate subcomponents like types, stats, moves, and sprite preview.
//
// Parameters:
//    name        - The Pokemon's name
//    type1       - The primary Pokemon type
//    type2       - The secondary Pokemon type
//    hp,...speed - Base stat values
//    moveNames   - 2D array of move names (4 total)
//    selected    - The index of the currently selected field
//    validation  - Pointer to validation result data (for visual feedback)
//    moveList    - Pointer to the move database
//    moveCount   - Number of available moves
//    pokemonID   - ID used to check for saved sprite file
//
// Returns:
//    void (prints to screen)
//=====================================================================
void render_form(const char* name, Type type1, Type type2, int hp, int atk, int def, int spAtk, int spDef, int speed,
    char moveNames[4][20], int selected, const ValidationResult* validation, Move* moveList, int moveCount, const char* pokemonID)
{
    for (int i = 0; i < FIELD_COUNT; i++)
    {
        // Determine the arrow symbol: ">" if selected, otherwise blank
        const char* arrow;
        if (i == selected)
        {
            arrow = "> ";
        }
        else
        {
            arrow = "  ";
        }

        // Determine the validation marker: red * if invalid, otherwise blank
        const char* marker;
        if (validation->fieldValid[i])
        {
            marker = "  ";
        }
        else
        {
            marker = "\x1b[91m* \x1b[0m";
        }

        switch (i)
        {
        case FIELD_NAME:
        {
            render_name_field(name, arrow, marker);
            break;
        }

        case FIELD_TYPE1:
        {
            render_type_field("MAIN TYPE", type1, arrow, marker);
            break;
        }

        case FIELD_TYPE2:
        {
            render_type_field("SECONDARY TYPE", type2, arrow, marker);
            break;
        }

        case FIELD_HP:
        {
            render_stat_field("HP", hp, arrow, marker);
            break;
        }

        case FIELD_ATTACK:
        {
            render_stat_field("ATTACK", atk, arrow, marker);
            break;
        }

        case FIELD_DEFENSE:
        {
            render_stat_field("DEFENSE", def, arrow, marker);
            break;
        }

        case FIELD_SPATK:
        {
            render_stat_field("SP. ATK", spAtk, arrow, marker);
            break;
        }

        case FIELD_SPDEF:
        {
            render_stat_field("SP. DEF", spDef, arrow, marker);
            break;
        }

        case FIELD_SPEED:
        {
            render_stat_field("SPEED", speed, arrow, marker);
            break;
        }

        case FIELD_MOVE_1:
        case FIELD_MOVE_2:
        case FIELD_MOVE_3:
        case FIELD_MOVE_4:
        {
            // Render the move field (index 0 to 3)
            int moveIndex = i - FIELD_MOVE_1;
            render_move_field(moveIndex, moveNames[moveIndex], arrow, marker, moveList, moveCount);
            break;
        }

        case FIELD_SPRITE_MAKER:
        {
            // Build the path to the sprite file
            char spritePath[128];
            snprintf(spritePath, sizeof(spritePath), "%s%s.txt", CUSTOM_SPRITE_FOLDER, pokemonID);

            FILE* spriteFile = fopen(spritePath, "r");

            // Show status if the sprite exists or not
            if (spriteFile)
            {
                printf("%s%s[EDIT SPRITE] \x1b[92m [ submitted]\x1b[0m\n", arrow, marker);
                fclose(spriteFile);
            }
            else
            {
                printf("%s%s[EDIT SPRITE] \x1b[90m [no sprite]\x1b[0m\n", arrow, marker);
            }

            break;
        }

        case FIELD_SUBMIT:
        {
            // Render the final submit button with validation color
            render_submit_field(validation->canSubmit, arrow);
            printf("=============================\n");
            break;
        }
        }
    }
}

//=====================================================================
// Function: render_name_field
// Purpose:
//    Renders the name input field in the custom Pokemon creation form.
//    Displays the arrow for selection, marker for validation, and current name.
//
// Parameters:
//    name    - The name of the Pokemon being edited
//    arrow   - The prefix arrow string ("> " or "  ") indicating selection
//    marker  - A visual marker string for field validation (e.g., red "*" or blank)
//
// Returns:
//    void (prints directly to the terminal)
//=====================================================================
void render_name_field(const char* name, const char* arrow, const char* marker)
{
    // Draw top border
    printf("=============================");

    // Draw the name field label with arrow and marker, then show the current name
    printf("\n%s%sNAME:           %s", arrow, marker, name);

    // Draw bottom border
    printf("\n=============================\n");
}

//=====================================================================
// Function: render_type_field
// Purpose:
//    Renders a single type selection field (primary or secondary) with label,
//    selection arrow, validation marker, and current type with color.
//
// Parameters:
//    label   - Field label (e.g., "MAIN TYPE" or "SECONDARY TYPE")
//    type    - The Type enum value to display
//    arrow   - Visual arrow ("> " if selected)
//    marker  - Validation indicator (e.g., red * if invalid)
//
// Returns:
//    void (prints to terminal)
//=====================================================================
void render_type_field(const char* label, Type type, const char* arrow, const char* marker)
{
    // Print the field's label with selection arrow and validation marker
    printf("%s%s%-17s:  <", arrow, marker, label);

    // Print the type name with appropriate color
    print_type_colored(type, type_to_string(type));

    // Close the field's display bracket
    printf(">\n");

    // strcmp returns 0 if strings match exactly
    // We're using it here to check if the label is "SECONDARY TYPE"
    // so we can print a separator line only under the secondary type field.
    if (strcmp(label, "SECONDARY TYPE") == 0)
    {
        printf("=============================\n");
    }
}

//=====================================================================
// Function: render_stat_field
// Purpose:
//    Renders a single stat input field (HP, ATK, DEF, etc.) with its label,
//    value, arrow for selection, and validation marker.
//
// Parameters:
//    label   - The label for the stat (e.g., "HP", "ATTACK")
//    value   - The numeric value of the stat
//    arrow   - Visual arrow ("> " if selected)
//    marker  - Validation indicator (e.g., red * if invalid)
//
// Returns:
//    void (prints to terminal)
//=====================================================================
void render_stat_field(const char* label, int value, const char* arrow, const char* marker)
{
    // Print the label, arrow, marker, and value (right-aligned)
    printf("%s%s%-12s %3d\n", arrow, marker, label, value);

    // strcmp returns 0 if the two strings are exactly equal
    // We use it here to print a bottom divider ONLY if the stat is SPEED
    if (strcmp(label, "SPEED") == 0)
    {
        printf("=============================\n");
    }
}

//=====================================================================
// Function: render_move_field
// Purpose:
//    Renders one of the four Pokemon move slots with its name, type color,
//    selection arrow, and a validation marker.
//
// Parameters:
//    index     - Move slot index (0-3)
//    moveName  - Name of the move in this slot
//    arrow     - Arrow prefix ("> " if selected)
//    marker    - Marker for validation (e.g., red "*" or blank)
//    moveList  - List of all available moves
//    moveCount - Number of moves in moveList
//
// Returns:
//    void (prints to terminal)
//=====================================================================
void render_move_field(int index, const char* moveName, const char* arrow, const char* marker, Move* moveList, int moveCount)
{
    // Create uppercase version of the move name for consistent display
    char upperName[32];
    to_uppercase(upperName, moveName, sizeof(upperName));

    // Try to find the move in the database to retrieve type info
    const Move* m = find_move(moveName, moveList, moveCount);

    // If the move is set to NOMOVE, display it as an empty move slot
    if (strcmp(upperName, "NOMOVE") == 0)
    {
        printf("%s%sMOVE %-2d     [--]\n", arrow, marker, index + 1);
    }
    else
    {
        // Print the move name label
        printf("%s%sMOVE %-2d     [", arrow, marker, index + 1);

        // If the move exists in the move database, color it by type
        if (m)
        {
            print_type_colored(m->type, upperName);
        }
        else
        {
            // If not found, print name in plain text
            printf("%s", upperName);
        }

        // Close the bracket for move display
        printf("]\n");
    }

    // If this is the last move slot, print the bottom divider
    if (index == 3)
    {
        printf("=============================\n");
    }
}

//=====================================================================
// Function: render_submit_field
// Purpose:
//    Displays the [SUBMIT] button in the Pokemon creation form. The color
//    changes based on whether submission is currently allowed.
//
// Parameters:
//    canSubmit - Whether the form is valid and can be submitted
//    arrow     - The arrow prefix ("> " if selected)
//
// Returns:
//    void (prints to terminal)
//=====================================================================
void render_submit_field(bool canSubmit, const char* arrow)
{
    // If the form is valid, show [SUBMIT] in bright green
    if (canSubmit)
    {
        printf("%s%s[SUBMIT]%s\n", arrow, COLOR_BRIGHT_GREEN, COLOR_RESET);
    }
    else
    {
        // Otherwise, show [SUBMIT] in dim gray to indicate it's inactive
        printf("%s%s[SUBMIT]%s\n", arrow, COLOR_GRAY, COLOR_RESET);
    }
}

//=====================================================================
// Function: render_header
// Purpose:
//    Renders the top banner and grading display for the Pokemon form.
//    The grade is color-coded, and the total stat score is shown beside it.
//
// Parameters:
//    totalStats - The sum of all base stats for the Pokemon
//    grade      - A letter or symbol representing stat quality (e.g., A, B, C)
//    color      - ANSI color string used to display the grade
//
// Returns:
//    void (prints to terminal)
//=====================================================================
void render_header(int totalStats, const char* grade, const char* color)
{
    // Draw header border and title
    printf("============================\n");
    printf("  CREATE CUSTOM POKEMON\n");
    printf("============================\n\n");

    // Show the stat grade in color and total stats
    printf("%sGRADE: %s%s (Total Stats: %d)\n\n", color, grade, COLOR_RESET, totalStats);
}

//=====================================================================
// Function: render_messages
// Purpose:
//    Displays validation error messages in red when the form cannot be submitted.
//    Loops through all messages in the ValidationResult and prints them as bullets.
//
// Parameters:
//    validation - Pointer to a ValidationResult struct containing error flags
//                 and human-readable validation messages.
//
// Returns:
//    void (prints messages to terminal)
//=====================================================================
void render_messages(const ValidationResult* validation)
{
    // Only show messages if submission is invalid and there are messages to show
    if (!validation->canSubmit && validation->messageCount > 0)
    {
        // Start red text for error messages
        printf("\n%s", COLOR_RED);

        // Print each validation message with a bullet prefix
        for (int i = 0; i < validation->messageCount; i++)
        {
            printf("%s %s\n", BULLET, validation->messages[i]);
        }

        // Reset terminal color
        printf("%s", COLOR_RESET);
    }
}

//=====================================================================
// Function: draw_move_search_bar
// Purpose:
//    Renders the top portion of the move selection UI, including the
//    current filter and search string with highlight indicators and
//    input mode feedback.
//
// Parameters:
//    search - Pointer to the current MoveSearchState holding filter and search input
//    focus  - Enum indicating which field (filter/search) is selected or being edited
//
// Returns:
//    void (prints formatted search/filter UI to terminal)
//=====================================================================
void draw_move_search_bar(const MoveSearchState* search, MoveFocusState focus)
{
    const char* fLabel = "Filter: ";
    const char* sLabel = "Search: ";

    // === FILTER FIELD ===
    if (focus == MOVE_FOCUS_FILTER || focus == MOVE_EDIT_FILTER)
    {
        // Show arrow and highlight color for selected/active filter field
        printc(COLOR_YELLOW, "> %s", "");
        printc(COLOR_YELLOW, "%s[%s]", fLabel, type_to_string(search->type_filter));

        // Show context text depending on whether we're in focus or edit mode
        if (focus == MOVE_FOCUS_FILTER)
        {
            printc(COLOR_YELLOW, " [ENTER to edit]");
        }
        if (focus == MOVE_EDIT_FILTER)
        {
            printc(COLOR_WHITE, " [EDITING]");
        }
        printf("\n");
    }
    else
    {
        // If not selected, show dimmed/inactive line
        printf("  %s[%s]\n", fLabel, type_to_string(search->type_filter));
    }

    // === SEARCH FIELD ===
    if (focus == MOVE_FOCUS_SEARCH || focus == MOVE_EDIT_SEARCH)
    {
        // Show arrow and search term
        printc(COLOR_YELLOW, "> ");
        printc(COLOR_YELLOW, "%s%s", sLabel, search->name_filter);

        // Show editing hint
        if (focus == MOVE_FOCUS_SEARCH)
        {
            printc(COLOR_YELLOW, " [ENTER to edit]");
        }
        if (focus == MOVE_EDIT_SEARCH)
        {
            printc(COLOR_WHITE, " [EDITING]");
        }
        printf("\n");
    }
    else
    {
        // If not selected, show inactive line
        printf("  %s%s\n", sLabel, search->name_filter);
    }

    // Extra spacing between search bar and result list
    printf("\n");
}
// ========== MOVE PICKER ==========

//=====================================================================
// Function: display_move_menu
// Purpose:
//    Opens a searchable, scrollable move selection menu for the player to
//    choose a move from a full list. Supports search by name and filter by type.
//
// Parameters:
//    moves      - Pointer to the full list of available moves
//    total      - Total number of moves in the move list
//    moveNames  - 2D array of the Pokemon's current move names
//    slot       - The slot index (0–3) being edited
//
// Returns:
//    int - Index of the selected move in the original move list
//          or -1 if the user cancels the selection
//=====================================================================
int display_move_menu(const Move* moves, int total, const char moveNames[4][20], int slot)
{
    int selected = 0;
    int start_index = 0;
    int ch;

    MoveFocusState focus = MOVE_FOCUS_LIST;
    MoveSearchState search = { .name_filter = "", .type_filter = POKITYPE_NONE };

    // Allocate array to hold filtered move indices
    int* filtered_indices = malloc(sizeof(int) * total);
    if (!filtered_indices)
    {
        return -1;
    }

    int result = -1;

    while (1)
    {
        system("cls");

        // Title and instructions
        printf("Select a move for slot %d\n", slot + 1);
        printf("Use arrow keys, ENTER to confirm, ESC to cancel\n\n");

        // Build filtered list based on current search state
        int filtered_count = build_filtered_move_list(moves, total, &search, filtered_indices);

        int page_end = start_index + MOVE_PAGE_SIZE;
        if (page_end > filtered_count)
        {
            page_end = filtered_count;
        }

        // Draw the filter/search bar
        draw_move_search_bar(&search, focus);

        // Render visible page of moves
        for (int i = start_index; i < page_end; i++)
        {
            const Move* m = &moves[filtered_indices[i]];

            char upperName[MAX_MOVE_NAME];
            to_uppercase(upperName, m->name, sizeof(upperName));

            const char* arrow = (i == start_index + selected && focus == MOVE_FOCUS_LIST) ? ">" : " ";

            printf("%s [%2d] %-15s ", arrow, i + 1, upperName);
            print_type_colored(m->type, type_to_string(m->type));

            // Show move description if selected
            if (i == start_index + selected && focus == MOVE_FOCUS_LIST)
            {
                char desc[MAX_LOG_LINE];
                describe_move(m, desc, sizeof(desc));
                printc(COLOR_BRIGHT_YELLOW, "  %s", desc);
            }

            printf("\n");
        }

        // Input handling
        ch = _getch();
        if (ch == KEY_SPECIAL_0 || ch == KEY_SPECIAL_224) ch = _getch();

        // Navigate up
        if (ch == KEY_ARROW_UP)
        {
            if (focus == MOVE_FOCUS_LIST && selected == 0) focus = MOVE_FOCUS_SEARCH;
            else if (focus == MOVE_FOCUS_SEARCH) focus = MOVE_FOCUS_FILTER;
            else if (focus == MOVE_FOCUS_LIST) selected--;
        }
        // Navigate down
        else if (ch == KEY_ARROW_DOWN)
        {
            if (focus == MOVE_FOCUS_FILTER) focus = MOVE_FOCUS_SEARCH;
            else if (focus == MOVE_FOCUS_SEARCH) focus = MOVE_FOCUS_LIST;
            else if (focus == MOVE_FOCUS_LIST && selected < (page_end - start_index - 1)) selected++;
        }
        // Change filter type left/right
        else if (ch == KEY_ARROW_LEFT && focus == MOVE_EDIT_FILTER)
        {
            if (search.type_filter == 0)
            {
                search.type_filter = POKITYPE_COUNT - 1;
            }
            else
            {
                search.type_filter--;
            }
        }
        else if (ch == KEY_ARROW_RIGHT && focus == MOVE_EDIT_FILTER)
        {
            search.type_filter = (search.type_filter + 1) % POKITYPE_COUNT;
        }
        // Handle ENTER key
        else if (ch == KEY_ENTER)
        {
            if (focus == MOVE_FOCUS_FILTER) focus = MOVE_EDIT_FILTER;
            else if (focus == MOVE_FOCUS_SEARCH) focus = MOVE_EDIT_SEARCH;
            else if (focus == MOVE_EDIT_FILTER || focus == MOVE_EDIT_SEARCH) focus = MOVE_FOCUS_LIST;
            else if (focus == MOVE_FOCUS_LIST)
            {
                result = filtered_indices[start_index + selected];
                break;
            }
        }
        // Handle ESC key
        else if (ch == KEY_ESC)
        {
            if (focus == MOVE_EDIT_FILTER) focus = MOVE_FOCUS_FILTER;
            else if (focus == MOVE_EDIT_SEARCH) focus = MOVE_FOCUS_SEARCH;
            else
            {
                result = -1;
                break;
            }
        }
        // Handle character input for name search
        else if (focus == MOVE_EDIT_SEARCH)
        {
            // Ignore arrow keys and navigation keys
            if (ch == KEY_ARROW_UP || ch == KEY_ARROW_DOWN || ch == KEY_ARROW_LEFT || ch == KEY_ARROW_RIGHT)
                continue;

            if (ch >= CHAR_PRINTABLE_MIN && ch <= CHAR_PRINTABLE_MAX)
            {
                size_t len = strlen(search.name_filter);
                if (len < sizeof(search.name_filter) - 1)
                {
                    search.name_filter[len] = (char)ch;
                    search.name_filter[len + 1] = '\0';
                }
            }
            else if (ch == KEY_BACKSPACE && strlen(search.name_filter) > 0)
            {
                search.name_filter[strlen(search.name_filter) - 1] = '\0';
            }
        }

    }

    // Cleanup
    free(filtered_indices);
    return result;
}

//=====================================================================
// Function: select_single_move
// Purpose:
//    Allows the user to select a move from the move menu and assigns it to
//    the specified slot, preventing duplicate selections.
//
// Parameters:
//    targetMove - Pointer to the destination buffer for the selected move name
//    moveNames  - Current move names (to check for duplicates)
//    slot       - Index of the slot being edited (0–3)
//    moveList   - List of all possible moves
//    moveCount  - Total number of moves in moveList
//
// Returns:
//    void (modifies targetMove by side effect)
//=====================================================================
void select_single_move(char* targetMove, char moveNames[4][20], int slot, Move* moveList, int moveCount)
{
    // Show the move menu and get the selected move index
    int index = display_move_menu(moveList, moveCount, moveNames, slot);

    // If the user cancelled selection, do nothing
    if (index == -1)
    {
        return;
    }

    // Prevent selecting a move that is already chosen in another slot
    for (int j = 0; j < 4; j++)
    {
        if (j != slot && strcmp(moveNames[j], moveList[index].name) == 0)
        {
            printf("\n[X] Already selected! Press a key to try again.\n");
            _getch();
            return;
        }
    }

    // Copy the selected move into the designated target slot
    strcpy_s(targetMove, 20, moveList[index].name);
}

//=====================================================================
// Function: build_filtered_move_list
// Purpose:
//    Filters the list of moves based on the current search state (name substring
//    and type match). Stores matching indices in the output array.
//
// Parameters:
//    moves             - Pointer to the full list of moves
//    count             - Total number of moves
//    search            - Pointer to the MoveSearchState containing filter criteria
//    filtered_indices  - Output array to hold indices of matching moves
//
// Returns:
//    int - Number of matching moves found and stored in filtered_indices
//=====================================================================
int build_filtered_move_list(const Move* moves, int count, const MoveSearchState* search, int* filtered_indices)
{
    int filtered_count = 0;

    for (int i = 0; i < count; i++)
    {
        const Move* m = &moves[i];
        bool match = true;

        // Filter by name substring (case-insensitive)
        if (search->name_filter[0] && !strstr_case_insensitive(m->name, search->name_filter))
        {
            match = false;
        }

        // Filter by type
        if (search->type_filter != POKITYPE_NONE && m->type != search->type_filter)
        {
            match = false;
        }

        // If move passed all filters, include its index
        if (match)
        {
            filtered_indices[filtered_count++] = i;
        }
    }

    return filtered_count;
}

//=====================================================================
// Function: validate_custom_pokemon
// Purpose:
//    Performs a full set of validations on a custom Pokemon configuration,
//    including stat bounds, name rules, type rules, and duplicate checking.
//    Collects error messages and marks individual fields as valid/invalid.
//
// Parameters:
//    name         - The name of the custom Pokemon
//    type1, type2 - The Pokemon's primary and secondary types
//    hp,...speed  - All six base stats
//    moveNames    - Array of up to 4 move names ("NOMOVE" means unused)
//    existingList - Pointer to array of all existing Pokemon
//    totalExisting- Number of Pokemon in existingList
//
// Returns:
//    ValidationResult - Struct containing error messages, per-field flags,
//                       and overall validity flag (canSubmit)
//=====================================================================
ValidationResult validate_custom_pokemon(
    const char* name,
    Type type1, Type type2,
    int hp, int atk, int def, int spAtk, int spDef, int speed,
    char moveNames[4][20],
    const Pokemon* existingList,
    int totalExisting)
{
    ValidationResult result = { 0 };
    result.canSubmit = true;

    // By default, all fields are valid
    for (int i = 0; i < FIELD_COUNT; i++)
    {
        result.fieldValid[i] = true;
    }

    int totalStats = calculate_total_stats(hp, atk, def, spAtk, spDef, speed);
    const char* gradeColor;
    const char* grade = grade_from_total(totalStats, &gradeColor);

    // Reject if the stat total is too low or too high (unacceptable grade)
    if (strcmp(grade, "TRAGIC") == 0 || strcmp(grade, "FORBIDDEN") == 0)
    {
        result.canSubmit = false;
        for (int i = FIELD_HP; i <= FIELD_SPEED; i++)
        {
            result.fieldValid[i] = false;
        }
        strcpy_s(result.messages[result.messageCount++], sizeof(result.messages[0]), "Stat total too low or too high. Grade invalid.");
    }

    // Check that all stats are above zero
    if (hp <= 0 || atk <= 0 || def <= 0 || spAtk <= 0 || spDef <= 0 || speed <= 0)
    {
        result.canSubmit = false;
        if (hp <= 0) result.fieldValid[FIELD_HP] = false;
        if (atk <= 0) result.fieldValid[FIELD_ATTACK] = false;
        if (def <= 0) result.fieldValid[FIELD_DEFENSE] = false;
        if (spAtk <= 0) result.fieldValid[FIELD_SPATK] = false;
        if (spDef <= 0) result.fieldValid[FIELD_SPDEF] = false;
        if (speed <= 0) result.fieldValid[FIELD_SPEED] = false;
        strcpy_s(result.messages[result.messageCount++], sizeof(result.messages[0]), "All stats must be above 0.");
    }

    // Name must be at least 3 characters
    if (strlen(name) < 3)
    {
        result.canSubmit = false;
        result.fieldValid[FIELD_NAME] = false;
        strcpy_s(result.messages[result.messageCount++], sizeof(result.messages[0]), "Name must be at least 3 characters long.");
    }

    // Name must contain only English letters and no spaces
    for (int i = 0; name[i] != '\0'; i++)
    {
        if (!(name[i] >= 'A' && name[i] <= 'Z') &&
            !(name[i] >= 'a' && name[i] <= 'z'))
        {
            result.canSubmit = false;
            result.fieldValid[FIELD_NAME] = false;
            strcpy_s(result.messages[result.messageCount++], sizeof(result.messages[0]), "Name must contain only English letters (A-Z, a-z) with no spaces.");
            break;
        }
    }

    // Name must not already exist (case-insensitive)
    for (int i = 0; i < totalExisting; i++)
    {
        if (_stricmp(existingList[i].name, name) == 0)
        {
            result.canSubmit = false;
            result.fieldValid[FIELD_NAME] = false;
            strcpy_s(result.messages[result.messageCount++], sizeof(result.messages[0]), "Name is already taken.");
            break;
        }
    }

    // At least one type must be selected
    if (type1 == POKITYPE_NONE && type2 == POKITYPE_NONE)
    {
        result.canSubmit = false;
        result.fieldValid[FIELD_TYPE1] = false;
        result.fieldValid[FIELD_TYPE2] = false;
        strcpy_s(result.messages[result.messageCount++], sizeof(result.messages[0]), "At least one type must be selected.");
    }

    // If both types are set, they must not be the same
    if (type1 != POKITYPE_NONE && type2 != POKITYPE_NONE && type1 == type2)
    {
        result.canSubmit = false;
        result.fieldValid[FIELD_TYPE1] = false;
        result.fieldValid[FIELD_TYPE2] = false;
        strcpy_s(result.messages[result.messageCount++], sizeof(result.messages[0]), "Types must be different.");
    }

    // At least one move must be selected
    bool hasValidMove = false;
    for (int i = 0; i < 4; i++)
    {
        if (strcmp(moveNames[i], "NOMOVE") != 0)
        {
            hasValidMove = true;
            break;
        }
    }
    if (!hasValidMove)
    {
        result.canSubmit = false;
        for (int i = FIELD_MOVE_1; i <= FIELD_MOVE_4; i++)
        {
            result.fieldValid[i] = false;
        }
        strcpy_s(result.messages[result.messageCount++], sizeof(result.messages[0]), "At least one move must be selected.");
    }

    return result;
}

//=====================================================================
// Function: calculate_total_stats
// Purpose:
//    Returns the sum of all base stats for a Pokemon, used to grade strength.
//
// Parameters:
//    hp, atk, def, spAtk, spDef, speed - The six core stat values
//
// Returns:
//    int - The total combined stat value
//=====================================================================
int calculate_total_stats(int hp, int atk, int def, int spAtk, int spDef, int speed)
{
    return hp + atk + def + spAtk + spDef + speed;
}

//=====================================================================
// Function: grade_from_total
// Purpose:
//    Assigns a grade and corresponding ANSI color code based on total stat value.
//    Used to visually rank a Pokemon's power level.
//
// Parameters:
//    total - The total of all stat values
//    color - Output pointer for the ANSI color string associated with the grade
//
// Returns:
//    const char* - The grade name (e.g., "VETERAN", "TRAGIC")
//=====================================================================
const char* grade_from_total(int total, const char** color)
{
    if (total >= 720)
    {
        *color = COLOR_FORBIDDEN;
        return "FORBIDDEN";
    }
    else if (total >= 680)
    {
        *color = COLOR_LEGENDARY;
        return "LEGENDARY";
    }
    else if (total >= 600)
    {
        *color = COLOR_CHAMPION;
        return "CHAMPION";
    }
    else if (total >= 500)
    {
        *color = COLOR_VETERAN;
        return "VETERAN";
    }
    else if (total >= 400)
    {
        *color = COLOR_CONTENDER;
        return "CONTENDER";
    }
    else if (total >= 300)
    {
        *color = COLOR_TRAINEE;
        return "TRAINEE";
    }
    else if (total >= 200)
    {
        *color = COLOR_FEEBLE;
        return "FEEBLE";
    }
    else
    {
        *color = COLOR_TRAGIC;
        return "TRAGIC";
    }
}
// ========== UTILITIES (Temporary) ==========

//=====================================================================
// Function: sprite_exists
// Purpose:
//    Checks whether a sprite file exists for a given custom Pokemon ID.
//    Used to conditionally render sprite status indicators.
//
// Parameters:
//    pokemonID - The unique ID of the Pokemon (e.g., "C101")
//
// Returns:
//    bool - true if the file exists, false otherwise
//=====================================================================
bool sprite_exists(const char* pokemonID)
{
    char path[128];

    // Construct the expected sprite file path
    snprintf(path, sizeof(path), "%s%s.txt", CUSTOM_SPRITE_FOLDER, pokemonID);

    // Attempt to open the file in read mode
    FILE* f = fopen(path, "r");

    // If it fails to open, file doesn't exist
    if (!f)
    {
        return false;
    }

    // File opened successfully — close it and return true
    fclose(f);
    return true;
}

//=====================================================================
// Function: to_uppercase
// Purpose:
//    Converts a string to uppercase, character by character, storing
//    the result in a destination buffer.
//
// Parameters:
//    dest   - Output buffer to store the uppercase string
//    src    - Source string to convert
//    maxLen - Maximum number of characters to copy, including null terminator
//
// Returns:
//    void (modifies dest in-place)
//=====================================================================
void to_uppercase(char* dest, const char* src, size_t maxLen)
{
    size_t i;

    // Loop through characters up to maxLen - 1 (leave space for \0)
    for (i = 0; i < maxLen - 1 && src[i]; i++)
    {
        dest[i] = toupper((unsigned char)src[i]);
    }

    // Null-terminate the destination string
    dest[i] = '\0';
}


                           