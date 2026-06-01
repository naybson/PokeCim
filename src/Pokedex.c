#include <stdio.h>
#include <ctype.h>
#include "platform.h"
#include <stdlib.h>
#include <string.h>
#ifndef _WIN32
#include <strings.h>
#define _strnicmp strncasecmp
#endif
#include <stdbool.h>

#include "constants.h"
#include "Pokedex.h"
#include "graphics.h"
#include "logic.h"
#include "main_menu.h"
#include "game_context.h"

//=====================================================================
// Function: display_menu
// Purpose:
//    Displays an interactive, scrollable and searchable Pokedex menu.
//    Supports filtering, paging, and selection of Pokemon.
//
// Parameters:
//    pokemons      - Array of Pokemon data to display
//    count         - Number of Pokemon in the array
//    search        - Pointer to the current search/filter state (will be updated)
//    selected      - Pointer to store current selected index (updated on return)
//    start_index   - Pointer to the current scroll start index (updated on return)
//
// Returns:
//    int - Index of selected Pokemon on ENTER, or -1 if user cancels (ESC)
//=====================================================================
int display_menu(const Pokemon pokemons[], int count, SearchState* search, int* selected, int* start_index)
{
    FocusState focus = FOCUS_LIST; // Which UI element is currently active/focused

    // Local copies to track current UI state inside the function
    int current_selected = *selected;
    int current_start_index = *start_index;
    SearchState current_search = *search;

    int ch; // Input char/key

    while (1)
    {
        clearScreen(); // Clear terminal for fresh redraw
        printf("==== Pokedex ====\n\n");

        // Allocate array to store filtered Pokemon indices
        int* filtered_indices = malloc(sizeof(int) * count);
        if (!filtered_indices) {
            perror("Failed to allocate filtered_indices");
            return -1;
        }

        // Build the filtered list based on current_search filters
        int filtered_count = build_filtered_list(pokemons, count, &current_search, filtered_indices);

        // Calculate end index of current page (do not overflow filtered_count)
        int page_end = current_start_index + PAGE_SIZE;
        if (page_end > filtered_count)
            page_end = filtered_count;

        // Draw the search/filter input bar and highlight focused element
        draw_search_bar(&current_search, focus);

        // Draw the filtered Pokemon list for the current page and highlight selected
        draw_filtered_list(pokemons, filtered_indices, current_start_index, page_end, current_selected, focus);

        // Calculate total pages and display page navigation info
        int total_pages = (filtered_count + PAGE_SIZE - 1) / PAGE_SIZE;
        printf("\n[Page %d of %d]  [^/v] Move  |  [</>] Page  |  [Enter] Select\n", (current_start_index / PAGE_SIZE) + 1, total_pages);
        printf("\n[Press ESC To return to main menu]");

        // Read key input from user
        ch = _getch();
        // If the key is a special key (arrow keys etc.), get the second part
        if (ch == KEY_SPECIAL_0 || ch == KEY_SPECIAL_224)
            ch = _getch();

        // Handle user pressing ENTER
        if (ch == KEY_ENTER)
        {
            int result = handle_enter(&focus, &current_selected, &current_start_index, filtered_indices, filtered_count);
            free(filtered_indices);
            if (result != -1)
            {
                // Save updated UI state before returning
                *selected = current_selected;
                *start_index = current_start_index;
                *search = current_search;
                return result; // Return the selected Pokemon index
            }
            continue; // If -1 returned, redraw menu
        }

        // If editing the filter input, handle filter input keys
        if (focus == EDIT_FILTER)
        {
            handle_edit_filter(ch, &current_search, &current_selected, &current_start_index);
            free(filtered_indices);
            continue; // Redraw menu after filter input
        }

        // If editing the search input, handle search input keys
        if (focus == EDIT_SEARCH)
        {
            handle_edit_search(ch, &current_search, &current_selected, &current_start_index);
            free(filtered_indices);
            continue; // Redraw menu after search input
        }

        // Handle navigation keys: arrows, page up/down etc.
        handle_navigation_keys(ch, &focus, &current_selected, &current_start_index, filtered_count, page_end);

        // Handle ESC key: change focus or exit menu
        if (ch == KEY_ESC)
        {
            if (focus == EDIT_FILTER)
                focus = FOCUS_FILTER; // Stop editing filter, go to filter focus
            else if (focus == EDIT_SEARCH)
                focus = FOCUS_SEARCH; // Stop editing search, go to search focus
            else
            {
                printf("\n[!] Returning to menu...\n");
                _getch();
                free(filtered_indices);
                return -1; // Exit menu (cancel)
            }
        }

        free(filtered_indices); // Free memory before next redraw loop
    }
}

// ========== NAVIGATION LOGIC ==========
//=====================================================================
// Function: handle_navigation_keys
// Purpose:
//    Processes user input for navigating the Pokedex menu using arrow keys.
//    Updates focus (which UI element is active), selected Pokemon index,
//    and scroll start index accordingly.
//
// Parameters:
//    ch             - The key code the user pressed.
//    focus          - Pointer to the current input focus state (filter, search, list).
//    selected       - Pointer to the currently highlighted Pokemon index.
//    start_index    - Pointer to the index of the first Pokemon shown on the current page.
//    filtered_count - Total number of Pokemon after filtering.
//    page_end       - Index of the end of the current visible page.
//
// Notes:
//    - Focus cycles between filter, search bar, and list.
//    - Arrow keys move selection and switch focus as appropriate.
//    - Left/Right arrows switch pages when focus is on the list.
//
//=====================================================================
void handle_navigation_keys(int ch, FocusState* focus, int* selected, int* start_index, int filtered_count, int page_end)
{
    switch (ch)
    {
        // === KEY UP ===
    case KEY_ARROW_UP:

        switch (*focus)
        {
        case FOCUS_LIST:
            // If at top of list, move focus up to search bar
            if (*selected == 0)
                *focus = FOCUS_SEARCH;
            else
                (*selected)--; // Otherwise, move selection up one

            break;

        case FOCUS_SEARCH:
            // Move focus up to filter input
            *focus = FOCUS_FILTER;
            break;

        case FOCUS_FILTER:
            // Already at top, no action
            break;

        default:
            break;
        }
        break;

        // === KEY DOWN ===
    case KEY_ARROW_DOWN:

        switch (*focus)
        {
        case FOCUS_FILTER:
            // Move focus down to search bar
            *focus = FOCUS_SEARCH;
            break;

        case FOCUS_SEARCH:
            // Move focus down to list
            *focus = FOCUS_LIST;
            break;

        case FOCUS_LIST:
            // Move selection down within page if possible
            if (*selected < (page_end - *start_index - 1))
                (*selected)++;
            break;

        default:
            break;
        }
        break;

        // === KEY LEFT ===
    case KEY_ARROW_LEFT:
        // If focused on list and not on first page, go back one page
        if (*focus == FOCUS_LIST && *start_index >= PAGE_SIZE)
        {
            *start_index -= PAGE_SIZE;
            *selected = 0; // Reset selection to first item on new page
        }
        break;

        // === KEY RIGHT ===
    case KEY_ARROW_RIGHT:
        // If focused on list and not on last page, go forward one page
        if (*focus == FOCUS_LIST && *start_index + PAGE_SIZE < filtered_count)
        {
            *start_index += PAGE_SIZE;
            *selected = 0; // Reset selection to first item on new page
        }
        break;

    default:
        // Other keys ignored by navigation handler
        break;
    }
}

//=====================================================================
// Function: handle_enter
// Purpose:
//    Handles the user pressing ENTER depending on the current focus in the Pokedex menu.
//    Transitions between editing modes and returns the selected Pokemon index if applicable.
//
// Parameters:
//    focus           - Pointer to the current input focus (filter, search, list, edit states).
//    selected        - Pointer to the current highlighted Pokemon index within the page.
//    start_index     - Pointer to the scroll start index (page offset).
//    filtered_indices - Array of indices corresponding to filtered Pokemon list.
//    filtered_count  - Number of Pokemon in the filtered list.
//
// Returns:
//    int - Returns the absolute index of the selected Pokemon from filtered_indices
//          if selection is made in the list; otherwise returns -1 (no selection).
//
// Behavior:
//    - If focus is on FILTER or SEARCH, ENTER switches to edit mode.
//    - If focus is editing FILTER or SEARCH and ENTER is pressed, focus returns to the list,
//      selection resets to the first item on the first page.
//    - If focus is on the LIST and ENTER is pressed, returns the actual selected Pokemon index.
//=====================================================================
int handle_enter(FocusState* focus, int* selected, int* start_index, const int* filtered_indices, int filtered_count)
{
    switch (*focus)
    {
    case FOCUS_FILTER:
        // User pressed ENTER while filter is focused  go into edit mode
        *focus = EDIT_FILTER;
        break;

    case FOCUS_SEARCH:
        // User pressed ENTER while search is focused  go into edit mode
        *focus = EDIT_SEARCH;
        break;

    case EDIT_FILTER:
        // User finished editing filter  return focus to list and reset view
        *focus = FOCUS_LIST;
        *selected = 0;       // Reset selection to first Pokemon
        *start_index = 0;    // Reset page to top
        break;

    case EDIT_SEARCH:
        // User finished editing search  return focus to list and reset view
        *focus = FOCUS_LIST;
        *selected = 0;       // Reset selection to first Pokemon
        *start_index = 0;    // Reset page to top
        break;

    case FOCUS_LIST:
        // User is in the list and presses ENTER  attempt to select current Pokemon
        if (filtered_count == 0)
        {
            // Nothing to select
            return -1;
        }

        // Return the actual Pokemon index from the filtered list
        return filtered_indices[*start_index + *selected];

    default:
        // Unknown state  do nothing
        break;
    }

    // If no valid selection was made, return -1 to stay in menu
    return -1;
}

// ========== SEARCH + FILTER LOGIC ==========

//=====================================================================
// Function: build_filtered_list
// Purpose:
//    Filters the given Pokemon array based on the current search criteria,
//    populating filtered_indices with indices of Pokemon that match.
//    Returns the count of filtered Pokemon.
//
// Parameters:
//    pokemons         - Array of all Pokemon to filter through
//    count            - Total number of Pokemon in the array
//    search           - Pointer to current search/filter criteria
//    filtered_indices - Output array to store indices of Pokemon passing filters
//
// Returns:
//    int - Number of Pokemon matching the filter criteria
// Notes:
// // strcmp:
// Compares two strings.
// Returns 0 if they are exactly the same, non-zero otherwise.
// 
// strstr:
// Finds the first occurrence of a substring inside another string.
// Returns pointer to substring or NULL if not found.
//=====================================================================
int build_filtered_list(const Pokemon* pokemons, int count, const SearchState* search, int* filtered_indices)
{
    int filtered_count = 0; // Tracks how many Pokemon pass the filter

    // Iterate through all Pokemon to check if they match the filters
    for (int i = 0; i < count; ++i)
    {
        const Pokemon* p = &pokemons[i]; // Current Pokemon pointer

        bool match = true; // Assume it matches initially

        // === FILTER BY NAME OR ID ===
        // If the user typed something in the name/id filter box
        if (search->name_filter[0] != '\0')
        {
            if (search->mode == SEARCH_NAME)
            {
                // Case-insensitive substring check in Pokemon name
                // Returns NULL if name_filter is not found inside p->name
                if (strstr_case_insensitive(p->name, search->name_filter) == NULL)
                {
                    match = false; // Does not match filter
                }
            }
            else if (search->mode == SEARCH_ID)
            {
                // Compare directly the Pokemon's ID string with the filter text
                if (strcmp(p->idnumber, search->name_filter) != 0)
                {
                    match = false; // ID does not match
                }
            }
        }

        // === FILTER BY TYPE ===
        // If user selected a specific type filter (not NONE)
        if (search->type_filter != POKITYPE_NONE)
        {
            // Pokemon must have the type as either type1 or type2 to match
            if (p->type1 != search->type_filter && p->type2 != search->type_filter)
            {
                match = false; // Type does not match filter
            }
        }

        // If the Pokemon passed all filters, add its index to the output list
        if (match)
        {
            filtered_indices[filtered_count++] = i;
        }
    }

    return filtered_count; // Return number of Pokemon that matched the filter
}

//=====================================================================
// Function: handle_edit_search
// Purpose:
//    Processes user input when editing the search bar in the Pokedex menu.
//    Handles printable characters, backspace, and updates search mode.
//
// Parameters:
//    ch          - The input character pressed by the user.
//    search      - Pointer to the SearchState, updated with new input.
//    selected    - Pointer to the current selected Pokemon index (reset on input).
//    start_index - Pointer to the current scroll start index (reset on input).
//
// Notes:
//    - Ignores arrow keys so they dont get added to the search string.
//    - Limits input length to prevent buffer overflow.
//    - Automatically switches search mode to SEARCH_ID if only digits are typed,
//      otherwise defaults to SEARCH_NAME.
// 
// // strlen explanation:
//    The standard C function `strlen(const char* str)` returns the length of a
//    null-terminated string, excluding the terminating null character.
//    For example, strlen("Hello") returns 5.
//=====================================================================
void handle_edit_search(int ch, SearchState* search, int* selected, int* start_index)
{
    // Ignore arrow keys during search text input (navigation handled elsewhere)
    if (ch == KEY_ARROW_UP || ch == KEY_ARROW_DOWN || ch == KEY_ARROW_LEFT || ch == KEY_ARROW_RIGHT)
        return;

    // If character is printable ASCII (letters, digits, symbols)
    if (ch >= CHAR_PRINTABLE_MIN && ch <= CHAR_PRINTABLE_MAX)
    {
        size_t len = strlen(search->name_filter);

        // Ensure theres room for new character plus null terminator
        if (len < sizeof(search->name_filter) - 1)
        {
            // Append new character at the end
            search->name_filter[len] = (char)ch;
            // Null-terminate the string
            search->name_filter[len + 1] = '\0';
        }
    }
    // If backspace is pressed, remove last character if string is not empty
    else if (ch == KEY_BACKSPACE)
    {
        size_t len = strlen(search->name_filter);
        if (len > 0)
        {
            // Remove last character by moving null terminator back one position
            search->name_filter[len - 1] = '\0';
        }
    }

    // Determine if the current search string contains only digits
    bool is_digits_only = true;
    for (size_t i = 0; i < strlen(search->name_filter); ++i)
    {
        if (search->name_filter[i] < ASCII_DIGIT_MIN || search->name_filter[i] > ASCII_DIGIT_MAX)
        {
            is_digits_only = false;
            break;
        }
    }

    // Set search mode based on input:
    // If empty string, default to name search
    if (search->name_filter[0] == '\0')
    {
        search->mode = SEARCH_NAME;
    }
    // If only digits, switch to ID search mode
    else if (is_digits_only)
    {
        search->mode = SEARCH_ID;
    }
    // Otherwise, use name search mode
    else
    {
        search->mode = SEARCH_NAME;
    }

    // Reset selection and scroll to start of filtered list on input change
    *selected = 0;
    *start_index = 0;
}

//=====================================================================
// Function: handle_edit_filter
// Purpose:
//    Processes LEFT and RIGHT arrow key input to cycle through Pokemon type filters.
//    Adjusts the filter type and resets selection and scroll to keep the list valid.
//
// Parameters:
//    ch          - The input character (key code) from the user.
//    search      - Pointer to the current SearchState, updated with new filter type.
//    selected    - Pointer to the currently selected Pokemon index, reset on filter change.
//    start_index - Pointer to the current scroll start index, reset on filter change.
//
// Behavior:
//    - LEFT arrow decrements the type filter, wrapping around to the last type if at zero.
//    - RIGHT arrow increments the type filter, wrapping around back to zero at the end.
//    - Resets the selected Pokemon index and scroll position to the top after filter change,
//      ensuring the UI shows the start of the new filtered list.
//=====================================================================
void handle_edit_filter(int ch, SearchState* search, int* selected, int* start_index)
{
    // Handle LEFT arrow: move to previous filter type with wrap-around
    if (ch == KEY_ARROW_LEFT)
    {
        if (search->type_filter == 0)
        {
            // Wrap around to last valid type index
            search->type_filter = POKITYPE_COUNT - 1;
        }
        else
        {
            // Move one type backward
            search->type_filter--;
        }

        // Reset selection and scroll to start of list for new filter
        *selected = 0;
        *start_index = 0;
    }

    // Handle RIGHT arrow: move to next filter type with wrap-around
    else if (ch == KEY_ARROW_RIGHT)
    {
        // Increment filter type modulo total count to wrap around
        search->type_filter = (search->type_filter + 1) % POKITYPE_COUNT;

        // Reset selection and scroll to top for consistency
        *selected = 0;
        *start_index = 0;
    }
}

//=====================================================================
// Function: strstr_case_insensitive
// Purpose:
//    Finds the first occurrence of substring `search_substring` within string `main_string`,
//    ignoring case differences.
//
// Parameters:
//    main_string       - The string to search within.
//    search_substring  - The substring to search for.
//
// Returns:
//    Pointer to the first occurrence of `search_substring` in `main_string` (case-insensitive),
//    or NULL if `search_substring` is not found.
//
// Notes:
//    - Uses _strnicmp (Windows-specific) for case-insensitive comparison of fixed length.
//      _strnicmp compares two strings up to a given length ignoring letter case.
//    - Uses strlen to find length of `search_substring` once for efficiency.
//    - Loops through `main_string` one character at a time to check for match starting at that position.
//=====================================================================
char* strstr_case_insensitive(const char* main_string, const char* search_substring)
{
    size_t search_substring_length = strlen(search_substring); // Cache substring length once

    // Loop through each character in main_string
    for (; *main_string; ++main_string)
    {
        // Compare substring starting at main_string with search_substring, ignoring case
        if (_strnicmp(main_string, search_substring, search_substring_length) == 0)
            return (char*)main_string; // Match found, return pointer to start
    }

    return NULL; // No match found
}

// ========== RENDERING ==========

//=====================================================================
// Function: draw_search_bar
// Purpose:
//    Displays the filter and search input bars at the top of the Pokedex menu,
//    highlighting the currently focused element and showing editing hints.
//
// Parameters:
//    search - Pointer to current SearchState containing filter and search strings.
//    focus  - Current UI focus state to determine which input is active or being edited.
//
// Behavior:
//    - Highlights the filter or search bar when focused.
//    - Shows different messages depending on whether input is being edited or just focused.
//    - Uses colored printing functions for better visual feedback.
//=====================================================================
void draw_search_bar(const SearchState* search, FocusState focus)
{
    // Labels for the two input lines
    const char* filter_label = "Filter: ";
    const char* search_label = "Search: ";

    // === Draw Filter Line ===
    if (focus == FOCUS_FILTER || focus == EDIT_FILTER)
    {
        // Highlight line with yellow color and prefix ">"
        printc(COLOR_YELLOW, "> %s", "");

        // Show filter label and current type filter name (e.g. "Fire", "Water")
        printc(COLOR_YELLOW, "%s[%s] ", filter_label, type_to_string(search->type_filter));

        if (focus == FOCUS_FILTER)
        {
            // Show hint for how to edit filter
            printc(COLOR_YELLOW, " [PRESS ENTER TO SELECT]", "");
        }

        if (focus == EDIT_FILTER)
        {
            // Show editing state in white color
            printc(COLOR_WHITE, "  [EDITING]", "");
        }

        printf("\n"); // New line after filter line
    }
    else
    {
        // When filter not focused, print normally without highlights
        printf("  %s[%s]\n", filter_label, type_to_string(search->type_filter));
    }

    // === Draw Search Line ===
    if (focus == FOCUS_SEARCH || focus == EDIT_SEARCH)
    {
        // Highlight line with yellow color and prefix ">"
        printc(COLOR_YELLOW, "> ", "");

        // Show search label and current search string input
        printc(COLOR_YELLOW, "%s%s ", search_label, search->name_filter);

        if (focus == FOCUS_SEARCH)
        {
            // Show hint to enter edit mode on search
            printc(COLOR_YELLOW, " [PRESS ENTER TO SELECT]", "");
        }

        if (focus == EDIT_SEARCH)
        {
            // Show editing confirmation hint in bright yellow
            printc(COLOR_BRIGHT_YELLOW, "  [PRESS ENTER TO TO CONFIRM]", "");
        }

        printf("\n"); // New line after search line
    }
    else
    {
        // When search not focused, print normally without highlights
        printf("  %s%s\n", search_label, search->name_filter);
    }

    printf("\n"); // Add spacing before the rest of the Pokedex list
}

//=====================================================================
// Function: draw_filtered_list
// Purpose:
//    Displays a paginated list of Pokemon entries based on the filtered indices,
//    highlighting the currently selected Pokemon if the list has focus.
//
// Parameters:
//    pokemons        - Array of all Pokemon data.
//    filtered_indices - Array of indices representing Pokemon matching current filters.
//    start_index     - Index in filtered_indices where the current page starts.
//    end_index       - Index in filtered_indices where the current page ends (exclusive).
//    selected        - Index of the currently highlighted Pokemon on the current page (0-based).
//    focus           - Current UI focus state (used to decide whether to highlight selection).
//
// Behavior:
//    - Iterates from start_index to end_index in filtered_indices.
//    - Retrieves the actual Pokemon from pokemons using filtered_indices.
//    - Prints each Pokemon's ID and name.
//    - Highlights the selected Pokemon with a yellow color and an arrow if the list has focus.
//=====================================================================
void draw_filtered_list(const Pokemon* pokemons, const int* filtered_indices, int start_index, int end_index, int selected, FocusState focus)
{
    // Loop over each Pokemon to display on the current page
    for (int i = start_index; i < end_index; ++i)
    {
        // Retrieve the actual Pokemon object by mapping filtered index to the original pokemons array
        const Pokemon* p = &pokemons[filtered_indices[i]];

        // Calculate local position of Pokemon on current page (0 to page size - 1)
        int local_index = i - start_index;

        // If this Pokemon is currently selected and the list is focused,
        // highlight it with yellow color and a '>' arrow prefix
        if (local_index == selected && focus == FOCUS_LIST)
        {
            printc(COLOR_YELLOW, "> [%s] %s\n", p->idnumber, p->name);
        }
        else
        {
            // Otherwise, print normally without highlight or arrow
            printf("  [%s] %s\n", p->idnumber, p->name);
        }
    }
}

// ========== DETAILS + HELPERS ==========

//=====================================================================
// Function: describe_move
// Purpose:
//    Creates a human-readable description string for a given Pokemon move,
//    including power, category, accuracy, and any special effects.
//
// Parameters:
//    m        - Pointer to the Move struct containing move details.
//    outBuf   - Buffer to write the description string into.
//    bufSize  - Size of the output buffer to prevent overflow.
//
// Behavior:
//    - If move has power and a status effect (not "0"), describes damage + status effect.
//    - If move has power only, describes damage and accuracy.
//    - If move changes enemy stats (statTarget not empty), describes stat change and accuracy.
//    - If move has only a status effect, describes that effect and accuracy.
//    - Otherwise, states "No effect."
//=====================================================================
void describe_move(const Move* m, char* outBuf, int bufSize)
{
    // Case 1: Move deals damage and may cause a status effect
    if (strcmp(m->statusEffect, "0") != 0 && m->power > 0)
    {
        snprintf(outBuf, bufSize,
            "Deals %d %s damage. Accuracy: %s%%. May cause %s.",
            m->power, category_to_string(m->category), m->accuracy, m->statusEffect);
    }
    // Case 2: Move deals damage only (no status effect)
    else if (m->power > 0)
    {
        snprintf(outBuf, bufSize,
            "Deals %d %s damage. Accuracy: %s%%.",
            m->power, category_to_string(m->category), m->accuracy);
    }
    // Case 3: Move changes enemy stats (statTarget non-empty)
    else if (strcmp(m->statTarget, "") != 0)
    {
        snprintf(outBuf, bufSize,
            "Lowers enemy %s by %d stage(s). Accuracy: %s%%.",
            m->statTarget, m->statChange, m->accuracy);
    }
    // Case 4: Move only causes a status effect (no damage, no stat change)
    else if (strcmp(m->statusEffect, "0") != 0)
    {
        snprintf(outBuf, bufSize,
            "May cause %s. Accuracy: %s%%.",
            m->statusEffect, m->accuracy);
    }
    // Case 5: Move has no effect
    else
    {
        snprintf(outBuf, bufSize, "No effect.");
    }
}

//=====================================================================
// Function: print_pokemon_details_interactive
// Purpose:
//    Displays detailed information about a selected Pokemon in the terminal,
//    including stats, types, moves (with descriptions), and its ASCII sprite.
//    Waits for the user to press ESC to return to the previous menu.
//
// Parameters:
//    p          - Pointer to the Pokemon to display.
//    allMoves   - Array of all available moves (to look up move details).
//    totalMoves - Total number of moves in allMoves.
//
// Returns:
//    int - Always returns 0 after user presses ESC to exit this view.
//=====================================================================
int print_pokemon_details_interactive(const Pokemon* p, const Move* allMoves, int totalMoves)
{
    // The Pokemon's move names (array of strings)
    const char (*moveNames)[20] = p->moves;

    // Load the Pokemon's ASCII art sprite based on its ID number
    char* spriteBuffer = loadPokemonSpriteByID(p->idnumber);

    clearScreen(); // Clear terminal for fresh display

    // === HEADER ===
    printf("===========================\n");
    printf("        %s\n", p->name); // Pokemon name

    printf("     ");
    // Print primary type in its color
    print_type_colored(p->type1, type_to_string(p->type1));

    // Print secondary type if it exists
    if (p->type2 != POKITYPE_NONE) {
        printf(" / ");
        print_type_colored(p->type2, type_to_string(p->type2));
    }

    printf("\n===========================\n");

    // Display base stats of the Pokemon
    printf("HP:         %d\n", p->hp);
    printf("ATTACK:     %d\n", p->attack);
    printf("SP ATTACK:  %d\n", p->spAttack);
    printf("DEFENSE:    %d\n", p->defense);
    printf("SP DEFENSE: %d\n", p->spDefence);
    printf("SPEED:      %d\n", p->speed);

    printf("===========================\n");
    printf("Moves:\n");

    // Loop through all moves (MAX_MOVES is typically 4)
    for (int i = 0; i < MAX_MOVES; i++) {
        const char* mname = moveNames[i]; // move name string
        const Move* m = find_move(mname, allMoves, totalMoves); // look up move details

        // If move not found or it's a placeholder "NOMOVE"
        if (!m || strcmp(mname, "NOMOVE") == 0) {
            printf("  %-12s -\n", mname); // Just print the move name with a dash
        }
        else {
            char desc[128]; // buffer for move description
            describe_move(m, desc, sizeof(desc)); // generate move description string

            printf("  ");
            printc(COLOR_WHITE, "%-12s ", m->name); // Print move name in white, padded
            print_type_colored(m->type, type_to_string(m->type)); // Print move type colored
            printf("  ");
            printc(COLOR_BRIGHT_YELLOW, "[%s]\n", desc); // Print description in bright yellow
        }
    }

    printf("===========================\n");
    printf("\n[Press ESC To return to PokeDex]\n");

    // Print the ASCII sprite art
    printSpriteBuffer(spriteBuffer);

    // Free the allocated sprite buffer memory
    free(spriteBuffer);

    // Wait indefinitely until user presses ESC (ASCII 27)
    int ch;
    while (1) 
    {
        ch = _getch();
        if (ch == KEY_ESC) {
            return 0; // Return to caller on ESC
        }
    }
}

