#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <conio.h>
#include <direct.h>   // for _mkdir
#include <io.h>       // for _findfirst, _finddata_t
#include <ctype.h>    // for tolower, toupper

#include "constants.h"
#include "logic.h"
#include "graphics.h"
#include "pokedex.h"        
#include "Custom_pokemon.h"
#include "main_menu.h"
#include "game_context.h"

// ========== SPRITE CREATION UI ==========

//=====================================================================
// Function: creature_selector_menu_with_id
// Purpose:
//    Provides an interactive terminal-based sprite creation menu for a given
//    Pokemon ID. Allows user to select sprite part, category, and save it.
//
// Parameters:
//    pokemonID - The unique ID of the Pokemon whose sprite is being created/edited
//
// Returns:
//    void (side effect: saves sprite file to disk)
//=====================================================================
void creature_selector_menu_with_id(const char* pokemonID)
{
    char* currentSprite = NULL;
    int lastCatIndex = -1;
    int lastCreatureIndex = -1;
    int catIndex = 0, creatureIndex = 0;
    int ch;

    // Load available sprite categories (folder names inside SPRITE_ASSET_FOLDER)
    char** categories;
    int categoryCount;
    load_category_list(&categories, &categoryCount);

    while (1)
    {
        system("cls");
        printf("==== SPRITE CREATOR ====\n\n");

        // Show currently selected sprite category
        printf("CATEGORY:  [%s]\n", categories[catIndex]);

        // Load creatures from the current category (e.g., body/eyes/limbs)
        char** creatures;
        int creatureCount;
        load_creature_list(categories[catIndex], &creatures, &creatureCount);

        // Show creature selection (if any)
        if (creatureCount > 0)
            printf("POKEMON:  [%d] %s\n", creatureIndex, strip_txt_extension(creatures[creatureIndex]));
        else
            printf("POKEMON:  (No creatures found)\n");

        printf("SUBMIT\n");

        printf("Press [1] to change BASE\n");
        printf("Press [2] to change TRAIT\n");
        printf("Press [3] or ENTER to SUBMIT\n");
        printf("Press ESC to CANCEL\n");

        // === Load sprite if needed ===
        if (creatureCount > 0 &&
            (catIndex != lastCatIndex || creatureIndex != lastCreatureIndex))
        {
            if (currentSprite) free(currentSprite);

            // Show loading bar
            printf("===============================\n");
            printf("Loading sprite: [");
            fflush(stdout);
            for (int i = 0; i < 20; i++)
            {
                Sleep(10);
                printf("#");
                fflush(stdout);
            }
            printf("]\n\n");

            currentSprite = load_ascii_creature(categories[catIndex], creatures[creatureIndex]);
            lastCatIndex = catIndex;
            lastCreatureIndex = creatureIndex;
        }
        else if (!currentSprite)
        {
            currentSprite = _strdup("(no sprite)");
        }

        // Show sprite
        printf("\nPreview:\n\n");
        while (_kbhit()) _getch(); // clear input buffer
        fwrite(currentSprite, sizeof(char), strlen(currentSprite), stdout);

        // Handle input
        ch = _getch();
        if (ch == 0 || ch == KEY_SPECIAL_224)
            ch = _getch(); // skip special key prefix

        switch (ch)
        {
        case '1':
        {
            int result = display_list_menu("Select Category", categories, categoryCount);
            if (result != -1)
            {
                catIndex = result;
                creatureIndex = 0; // Reset creature index when category changes
            }
            // else: ESC pressed, so stay in current category
            break;
        }

        case '2':
        {
            if (creatureCount > 0)
            {
                int result = display_list_menu("Select Creature", creatures, creatureCount);
                if (result != -1)
                    creatureIndex = result;
                // else: ESC pressed, stay on the same creature
            }
            break;
        }

        case '3':
        case KEY_ENTER:
            if (creatureCount > 0)
            {
                save_sprite_to_file(pokemonID, currentSprite);
                printf("\nSprite saved as %s! Press any key to continue...", pokemonID);
                _getch();
                goto cleanup;
            }
            break;

        case KEY_ESC:
            goto cleanup;
        }

        // Free creature list after each loop
        for (int i = 0; i < creatureCount; ++i)
            free(creatures[i]);
        free(creatures);
    }

cleanup:
    for (int i = 0; i < categoryCount; ++i)
        free(categories[i]);
    free(categories);
    if (currentSprite) free(currentSprite);
}

// ========== FILE SYSTEM LOGIC ==========

//=====================================================================
// Function: load_category_list
// Purpose:
//    Scans the SPRITE_ASSET_FOLDER directory for subfolders and returns a list
//    of folder names, which are interpreted as sprite categories.
//
// Parameters:
//    outFileList - Pointer to char** that will be allocated and filled with folder names
//    outCount    - Pointer to int that will store the number of folders found
//
// Returns:
//    int - 1 if at least one folder was found, 0 otherwise
//=====================================================================
int load_category_list(char*** outFileList, int* outCount)
{
    struct _finddata_t file; // Struct to hold file data during search

    // Build search path to match everything in the sprite asset folder
    char searchPath[128];
    snprintf(searchPath, sizeof(searchPath), "%s*", SPRITE_ASSET_FOLDER); // %s& means Match every file you see!

    // Begin directory search
    intptr_t handle = _findfirst(searchPath, &file);
    if (handle == -1)
    {
        // No folders found or error occurred
        *outFileList = NULL;
        *outCount = 0;
        return 0;
    }

    int capacity = 10; // Initial list capacity
    int count = 0;     // Folder counter
    char** list = malloc(capacity * sizeof(char*)); // Allocate memory for folder name list

    // Loop through search results
    int result = 0;
    while (result == 0)
    {
        // Include only directories and skip current/parent dir entries
        if ((file.attrib & _A_SUBDIR) && strcmp(file.name, ".") != 0 && strcmp(file.name, "..") != 0)
        {
            // Resize list if full
            if (count >= capacity)
            {
                capacity *= 2;
                list = realloc(list, capacity * sizeof(char*));
            }

            // Copy directory name into list
            list[count++] = _strdup(file.name);
        }

        result = _findnext(handle, &file); // Move to the next file in the search result
        // Returns 0 if another file was found, -1 if done
    }

    // ============================
    // Finalization and return
    // ============================

    _findclose(handle);            // Close the search handle to release system resources
    *outFileList = list;           // Output the list of category names to the caller
    *outCount = count;             // Output the number of found categories
    return 1;                      // Indicate success
}

//=====================================================================
// Function: load_ascii_creature
// Purpose:
//    Loads an ASCII sprite from a given category folder and filename.
//
// Parameters:
//    category - Name of the sprite category (e.g., "body", "eyes")
//    filename - Name of the .txt file containing the sprite
//
// Returns:
//    char* - Dynamically allocated string containing the ASCII sprite.
//           Caller is responsible for freeing the memory.
//=====================================================================
char* load_ascii_creature(const char* category, const char* filename)
{

    // Create a full file path string based on the folder and file name
    // Example: "sprites/custom_sprites_assets/body/round.txt"
    char fullPath[256];
    snprintf(fullPath, sizeof(fullPath), "%s%s/%s", SPRITE_ASSET_FOLDER, category, filename);

    // Try to open the file for reading
    FILE* file = fopen(fullPath, "r");
    if (!file)
    {
        // Return error message if the file couldn't be opened
        return _strdup("(Failed to load sprite)");
    }

    // Allocate memory for the sprite contents
    // This buffer will hold all lines of the sprite text
    char* buffer = malloc(MAX_SPRITE_SIZE);
    if (!buffer)
    {
        fclose(file); // Always close the file if something goes wrong
        return _strdup("(Allocation failed)");
    }

    size_t offset = 0;       // Current write position in the buffer
    char line[256];          // Temporary line buffer for reading each line of the file

    // Read each line from the file until end or buffer fills
    while (fgets(line, sizeof(line), file))
    {
        size_t len = strlen(line); // Measure the length of the current line

        // Check if the buffer has enough space left for this line
        if (offset + len < MAX_SPRITE_SIZE)
        {
            strncpy_s(buffer + offset, MAX_SPRITE_SIZE - offset, line, len); // Append line to buffer at offset
            offset += len;                 // Advance offset by length of the line
        }
        else
        {
            // Stop reading if the sprite is too large for the buffer
            break;
        }
    }

    fclose(file); // Done reading, close the file
    return buffer; // Return pointer to the loaded sprite
    // Caller must free this when done
}

//=====================================================================
// Function: load_creature_list
// Purpose:
//    Scans the given sprite category folder for all .txt creature files,
//    excluding the "manifest.txt" entry. Outputs a list of filenames.
//
// Parameters:
//    category     - Name of the folder under SPRITE_ASSET_FOLDER to scan
//    outFileList  - Output array of strings (allocated inside)
//    outCount     - Pointer to store number of files found
//
// Returns:
//    int - 1 on success, 0 if no files found or on failure
//=====================================================================
int load_creature_list(const char* category, char*** outFileList, int* outCount)
{
    // Construct the search path: e.g., sprites/custom_sprites_assets/body/*.txt
    char searchPath[128];
    snprintf(searchPath, sizeof(searchPath), "%s%s/*.txt", SPRITE_ASSET_FOLDER, category);

    // _finddata_t is used to hold data about each file found (name, size, etc.)
    struct _finddata_t file;

    // Begin search for files matching the search path
    // handle is used to continue the search with _findnext()
    intptr_t handle = _findfirst(searchPath, &file);

    // If no files are found, return failure with empty results
    if (handle == -1)
    {
        *outFileList = NULL;  // No files found, output pointer is NULL
        *outCount = 0;        // No files found, count is 0
        return 0;             // Return failure
    }

    // Allocate initial memory for up to 10 filenames
    int capacity = 10;
    int count = 0;
    char** fileList = malloc(capacity * sizeof(char*)); // fileList is a list of string pointers
    if (!fileList)
    {
        _findclose(handle);
        return 0; // Allocation failed
    }

    // Process files starting with the first found
    int result = 0;
    while (result == 0)
    {
        // Expand memory if the list is full
        if (count >= capacity)
        {
            capacity *= 2;
            fileList = realloc(fileList, capacity * sizeof(char*));
            if (!fileList)
            {
                _findclose(handle);
                return 0; // Reallocation failed
            }
        }

        // Duplicate the filename string and store it in the list
        fileList[count++] = _strdup(file.name);

        // Try to find the next file; exit loop if none found
        result = _findnext(handle, &file);
    }

    // Close the directory search handle
    _findclose(handle);

    // Set the output pointers with results
    *outFileList = fileList; // The list of file names
    *outCount = count;       // Number of valid files found

    return 1; // Success
}

void save_sprite_to_file(const char* id, const char* sprite)
{
    _mkdir(CUSTOM_SPRITE_FOLDER);
    char filename[128];
    snprintf(filename, sizeof(filename), "%s%s.txt", CUSTOM_SPRITE_FOLDER, id);

    FILE* f = fopen(filename, "w");
    if (!f)
    {
        return;
    }

    // Write the sprite without any color
    fprintf(f, "%s", sprite);
    fclose(f);
}

// ========== MISC HELPERS ==========

//============================================================
// STRIP TXT EXTENSION
// Returns a copy of the filename without the ".txt" extension.
// Capitalizes the first letter of the resulting string.
// Parameters:
//   - filename: Input filename string (e.g., "ghost.txt")
// Returns:
//   - A static buffer containing the cleaned name (e.g., "Ghost")
//============================================================
const char* strip_txt_extension(const char* filename) 
{
    static char label[64]; // Static buffer (caller must not free)

    // Copy the input filename into the label buffer safely
    strncpy_s(label, sizeof(label), filename, _TRUNCATE);
    label[sizeof(label) - 1] = '\0'; // Ensure null-termination

    // Find the last '.' and cut off the extension
    char* dot = strrchr(label, '.');
    if (dot) *dot = '\0';

    // Capitalize the first letter if it's not empty
    if (label[0]) label[0] = toupper(label[0]);

    return label;
}

int display_list_menu(const char* title, char** items, int count)
{
    int selected = 0;
    int ch;

    while (1)
    {
        system("cls");
        printf("=== %s ===\n\n", title);

        for (int i = 0; i < count; i++)
        {
            if (i == selected)
                printf(" > %s\n", strip_txt_extension(items[i]));
            else
                printf("   %s\n", strip_txt_extension(items[i]));
        }

        ch = _getch();
        if (ch == KEY_SPECIAL_0 || ch == KEY_SPECIAL_224) ch = _getch();

        switch (ch)
        {
        case KEY_ARROW_UP: // Up arrow
            selected = (selected - 1 + count) % count;
            break;
        case KEY_ARROW_DOWN: // Down arrow
            selected = (selected + 1) % count;
            break;
        case KEY_ENTER:
            return selected;
        case KEY_ESC:
            return -1;
        }
    }
}
