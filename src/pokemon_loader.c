//============================================================
// 📥 POKeMON LOADER (core/pokemon_loader.c)
// Loads Pokemon data from file and builds the list.
//============================================================
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <locale.h>
#include <ctype.h>
#include <stdbool.h>
#include "platform.h"
#ifdef _WIN32
#include <io.h>
#include <direct.h>
#include <windows.h>
#else
#include <dirent.h>
#endif
#include "pokemon.h"
#include "logic.h"
#include "constants.h"
#include "game_context.h"

//=====================================================================
// Function: load_pokemon_list
// Purpose:
//    Loads Pokemon data from a text file into a dynamically allocated array,
//    using secure fscanf_s to prevent buffer overflow.
//
// Parameters:
//    filename - Path to the file containing Pokemon data.
//    outCount - Pointer to an integer where the number of loaded Pokemon will be stored.
//
// Returns:
//    Pointer to a dynamically allocated array of Pokemon structs on success,
//    or NULL on failure (e.g., file open error or memory allocation failure).
//=====================================================================
Pokemon* load_pokemon_list(const char* filename, int* outCount)
{
    // Open the file for reading ("r")
    FILE* file = fopen(filename, "r");
    if (!file)
    {
        perror("Failed to open file"); // Print error if file cannot be opened
        return NULL;
    }

    // Initial capacity for the dynamically allocated Pokemon array
    int capacity = INITIAL_CAPACITY;
    int count = 0; // Number of Pokemon successfully loaded

    // Allocate memory for Pokemon array
    Pokemon* pokemons = malloc(sizeof(Pokemon) * capacity);
    if (!pokemons)
    {
        printf("Failed to allocate Pokemon array\n");
        fclose(file); // Close file before returning
        return NULL;
    }

    // Loop until end of file is reached
    while (!feof(file))
    {
        Pokemon p;          // Temporary Pokemon struct to hold parsed data
        char type1[20] = { 0 }; // Buffers for Pokemon type names (strings)
        char type2[20] = { 0 };

        // fscanf_s reads formatted input safely, requires buffer sizes for strings
        int read = fscanf(file,
            "%7s %31s %19s %19s %d %d %d %d %d %d %19s %19s %19s %19s\n",
            p.idnumber,  // Read ID number string and buffer size
            p.name,          // Read Pokemon name string and buffer size
            type1,              // Read first type string and buffer size
            type2,              // Read second type string and buffer size
            &p.hp, &p.attack, &p.defense, &p.spAttack, &p.spDefence, &p.speed, // Read integer stats
            p.moves[0],  // Read move names and their buffer sizes (4 moves)
            p.moves[1],
            p.moves[2],
            p.moves[3]);

        // Check if all 18 input items were successfully read
        if (read != 14)
        {
            // Invalid or incomplete line in file, skip it and continue reading next line
            continue;
        }

        // Convert type strings to enum values used internally
        p.type1 = type_from_name(type1);
        p.type2 = type_from_name(type2);

        // If array is full, resize it to hold more Pokemon
        if (count >= capacity)
        {
            capacity *= 2; // Double capacity for amortized efficiency
            Pokemon* temp = realloc(pokemons, sizeof(Pokemon) * capacity);
            if (!temp)
            {
                // Memory reallocation failed, clean up and exit
                free(pokemons);
                fclose(file);
                return NULL;
            }
            pokemons = temp;
        }

        // Store the successfully parsed Pokemon into the array
        pokemons[count++] = p;
    }

    // Close the file handle to release resources
    fclose(file);

    // Store the final count of loaded Pokemon to output parameter
    *outCount = count;

    // Return pointer to dynamically allocated Pokemon array
    return pokemons;
}

// ========== SPRITE DISPLAY ==========

//=====================================================================
// Function: printPokemon
// Purpose:
//    Loads and prints the ASCII art sprite for a Pokemon by name.
//    The function sanitizes the input name (lowercases it), builds the
//    file path, reads the sprite file safely into a buffer, then prints it.
//
// Parameters:
//    name - The Pokemon name string to find the sprite for.
//
// Behavior:
//    - Assumes sprites are stored as lowercase text files under POKEMON_SPRITE_FOLDER.
//    - Reads the entire sprite file safely into a dynamically allocated buffer.
//    - Prints the sprite or an error message if file or memory fails.
//=====================================================================
void printPokemon(const char* name)
{
    // Ensure locale supports UTF-8 for proper character display
    setlocale(LC_ALL, "en_US.UTF-8");

    // Sanitize name by converting it to lowercase to match filename convention
    char sanitized[100];
    int i = 0;
    for (; name[i] && i < sizeof(sanitized) - 1; ++i)
    {
        sanitized[i] = tolower(name[i]);
    }
    sanitized[i] = '\0'; // Null-terminate the sanitized string

    // Build the full file path for the sprite file
    char path[120];
    snprintf(path, sizeof(path), POKEMON_SPRITE_FOLDER "%s.txt", sanitized);

    // Open the sprite file for reading
    FILE* file = fopen(path, "r");
    if (!file) 
    {
        printf("Could not find sprite for %s\n", name);
        return;
    }

    // Allocate a large buffer to hold the entire sprite contents
    char* spriteBuffer = malloc(MAX_SPRITE_SIZE);
    if (!spriteBuffer) 
    {
        printf("Memory allocation failed for sprite\n");
        fclose(file);
        return;
    }
    spriteBuffer[0] = '\0'; // Initialize buffer as empty string

    // Temporary buffer to read each line from file
    char line[64];
    size_t used = strlen(spriteBuffer);

    // Read file line by line and append to spriteBuffer, avoiding overflow
    while (fgets(line, sizeof(line), file)) {
        size_t len = strlen(line);

        // Check if there is enough space left in spriteBuffer for this line
        if (used + len >= MAX_SPRITE_SIZE - 1)
            break;

        // Safely copy the line into spriteBuffer at current offset
        strncpy_s(spriteBuffer + used, (size_t)(MAX_SPRITE_SIZE - used), line, (size_t)(MAX_SPRITE_SIZE - used) - 1);
        used += len;
    }

    fclose(file); // Close the sprite file

    // Print the entire sprite at once
    printf("%s", spriteBuffer);

    free(spriteBuffer); // Free the allocated buffer
}

//=====================================================================
// Function: loadPokemonSpriteByID
// Purpose:
//    Loads the ASCII art sprite of a Pokemon given its ID string.
//    Supports loading from custom sprites or default sprites folders.
//
// Parameters:
//    id - The Pokemon ID string. IDs starting with 'C' are considered custom.
//
// Returns:
//    Pointer to a dynamically allocated buffer containing the sprite text,
//    or a duplicated error message string if loading fails.
//
// Notes:
//    - The caller is responsible for freeing the returned buffer.
//    - Uses safe string functions to avoid buffer overflows.
//=====================================================================
char* loadPokemonSpriteByID(const char* id) {
    char path[128]; // Buffer to hold the full file path

    // Determine folder based on whether it's a custom Pokemon (ID starts with 'C')
    if (id[0] == 'C')
    {
        // Construct path to custom sprite folder
        snprintf(path, sizeof(path), CUSTOM_SPRITE_FOLDER "%s.txt", id);
    }
    else {
        // Construct path to standard Pokemon sprite folder
        snprintf(path, sizeof(path), POKEMON_SPRITE_FOLDER "%s.txt", id);
    }

    // Open the sprite file for reading
    FILE* f = fopen(path, "r");
    if (!f)
        return _strdup("(sprite missing)"); // Return error string if file missing

    // Allocate a buffer to hold the entire sprite file contents
    char* buffer = malloc(MAX_SPRITE_SIZE);
    if (!buffer)
    {
        fclose(f); // Close file before returning
        return _strdup("(memory error)"); // Return error string if allocation fails
    }
    buffer[0] = '\0'; // Initialize buffer as empty string

    char line[128];  // Temporary buffer to read each line (safely sized for colored lines)
    size_t used = 0; // Track how much of buffer is used

    // Read file line-by-line until EOF or buffer limit reached
    while (fgets(line, sizeof(line), f)) {
        size_t len = strlen(line);

        // Prevent buffer overflow: ensure enough space for the line + null terminator
        if (used + len >= MAX_SPRITE_SIZE - 1)
            break;

        // Safely append the line into buffer at current offset
        strncpy_s(buffer + used, (size_t)(MAX_SPRITE_SIZE - used), line, (size_t)(MAX_SPRITE_SIZE - used) - 1);
        used += len;
    }

    fclose(f); // Close the file handle

    return buffer; // Return the buffer containing the full sprite
}

//=====================================================================
// Function: printSpriteBuffer
// Purpose:
//    Prints the given ASCII sprite buffer to the console.
//
// Parameters:
//    spriteBuffer - Pointer to a null-terminated string containing the sprite.
//
// Behavior:
//    - If spriteBuffer is not NULL, prints its contents.
//    - If spriteBuffer is NULL, prints a placeholder message indicating missing sprite.
//=====================================================================
void printSpriteBuffer(const char* spriteBuffer)
{
    if (spriteBuffer)
        printf("%s", spriteBuffer);   // Print the sprite
    else
        printf("[Missing sprite]\n"); // Print fallback message if NULL
}

 //=====================================================================
 // Function: cleanup_orphaned_custom_sprites
 // Purpose:
 //    Deletes any custom sprite files (.txt) in CUSTOM_SPRITE_FOLDER that do not
 //    match a valid Pokemon ID in the master Pokemon list file. This helps keep
 //    the custom sprites folder clean and free of orphaned files.
 //
 // Behavior:
 //    1) Loads valid Pokemon IDs into memory.
 //    2) Scans custom sprite folder for files starting with 'C'.
 //    3) Deletes any sprite file whose ID isn't in the valid list.
 //    4) Prints out status messages during cleanup.
 // 
 // Notes:
 // strcmp - Compares two strings lexicographically. Returns 0 if equal.
 //
 // strrchr - Finds the last occurrence of a character in a string.
 // Returns a pointer to the character or NULL if not found.
 //
 // intptr_t - An integer type capable of storing a pointer value.Used here to store file search handles.
 //
 // struct _finddata_t - A Windows - specific struct that holds file metadata
 // (e.g., name, attributes) when performing directory searches with _findfirst / _findnext.
 //=====================================================================
void cleanup_orphaned_custom_sprites()
{
    printf("Running orphaned sprite cleanup...\n");

    // Step 1: Open the master Pokemon list file to read valid IDs
    FILE* f = fopen(POKEMON_LIST_FILE, "r");
    if (!f) {
        printf("Could not open Pokemon list file: %s\n", POKEMON_LIST_FILE);
        return; // Abort if we can't access the master list
    }

    int capacity = 128; // Initial array capacity for storing IDs
    int validCount = 0; // Number of valid IDs read so far
    char** validIDs = malloc(sizeof(char*) * capacity); // Allocate array of string pointers
    if (!validIDs) {
        fclose(f);
        printf("Memory allocation failed!\n");
        return;
    }

    char line[256];
    // Read each line from the Pokemon list file
    while (fgets(line, sizeof(line), f)) {
        char id[16]; // Buffer to hold a single Pokemon ID

        // Parse the first whitespace-separated token as ID
        if (sscanf_s(line, "%15s", id, (unsigned)sizeof(id)) == 1)
        {
            // Resize array if we exceed current capacity
            if (validCount >= capacity) 
            {
                capacity *= 2;
                char** temp = realloc(validIDs, sizeof(char*) * capacity);
                if (!temp) 
                {
                    printf("Memory reallocation failed!\n");
                    fclose(f);
                    // Free previously allocated IDs before returning
                    for (int i = 0; i < validCount; i++)
                        free(validIDs[i]);
                    free(validIDs);
                    return;
                }
                validIDs = temp;
            }
            validIDs[validCount++] = _strdup(id); // Duplicate and store ID string
        }
    }
    fclose(f); // Finished reading Pokemon list

    // Step 2: Prepare to scan the custom sprites folder for .txt files
    #ifdef _WIN32
    struct _finddata_t file;
    char searchPath[128];
    snprintf(searchPath, sizeof(searchPath), "%s*.txt", CUSTOM_SPRITE_FOLDER);
    intptr_t searchHandle = _findfirst(searchPath, &file);
    if (searchHandle == -1) { printf("No custom sprites to clean up.\n"); goto cleanup; }
    while (1) {
        char* dot = strrchr(file.name, '.'); if (dot) *dot = '\0';
        if (file.name[0] == 'C') {
            bool found = false;
            for (int i = 0; i < validCount; i++) if (strcmp(validIDs[i], file.name) == 0) { found = true; break; }
            if (!found) {
                char fullPath[256];
                snprintf(fullPath, sizeof(fullPath), "%s%s.txt", CUSTOM_SPRITE_FOLDER, file.name);
                if (remove(fullPath) == 0) printf("Deleted orphaned sprite: %s\n", file.name);
                else printf("Failed to delete: %s\n", file.name);
            }
        }
        if (_findnext(searchHandle, &file) != 0) break;
    }
    _findclose(searchHandle);
#else
    /* POSIX: opendir/readdir to scan CUSTOM_SPRITE_FOLDER for .txt files */
    DIR* dir = opendir(CUSTOM_SPRITE_FOLDER);
    if (!dir) { printf("No custom sprites to clean up.\n"); goto cleanup; }
    struct dirent* entry;
    while ((entry = readdir(dir)) != NULL) {
        char name[256];
        strncpy(name, entry->d_name, sizeof(name) - 1);
        name[sizeof(name)-1] = ' ';
        char* dot = strrchr(name, '.');
        if (!dot || strcmp(dot, ".txt") != 0) continue;
        *dot = '\0';
        if (name[0] == 'C') {
            bool found = false;
            for (int i = 0; i < validCount; i++) if (strcmp(validIDs[i], name) == 0) { found = true; break; }
            if (!found) {
                char fullPath[256];
                snprintf(fullPath, sizeof(fullPath), "%s%s.txt", CUSTOM_SPRITE_FOLDER, name);
                if (remove(fullPath) == 0) printf("Deleted orphaned sprite: %s\n", name);
                else printf("Failed to delete: %s\n", name);
            }
        }
    }
    closedir(dir);
#endif

cleanup: // Label used as a target for cleanup and exit to free allocated memory
    // Free all allocated Pokemon ID strings
    for (int i = 0; i < validCount; i++) 
    {
        free(validIDs[i]);
    }
    free(validIDs); // Free the array pointer itself
}

Move* load_move_list_dynamic(const char* filename, int* outCount)
{
    FILE* file = fopen(filename, "r");
    if (!file) {
        perror("Failed to open move list");
        *outCount = 0;
        return NULL;
    }

    int capacity = 50;
    int count = 0;
    Move* moves = malloc(sizeof(Move) * capacity);
    if (!moves) {
        perror("Memory allocation failed");
        fclose(file);
        *outCount = 0;
        return NULL;
    }

    char raw[256];
    while (fgets(raw, sizeof(raw), file))
    {
        char name[20], typeStr[20], catStr[20], accStr[10];
        char statTarget[20], statusEffect[20];
        int power, statChange;

        int read = sscanf(raw, "%19s %19s %19s %d %9s %19s %d %19s",
            name, typeStr, catStr, &power, accStr,
            statTarget, &statChange, statusEffect);

        if (read != 8)
        {
            printf("Skipping malformed line: %s", raw);
            continue;
        }

        if (count >= capacity)
        {
            capacity *= 2;
            moves = realloc(moves, sizeof(Move) * capacity);
            if (!moves) {
                perror("Reallocation failed");
                fclose(file);
                *outCount = 0;
                return NULL;
            }
        }

        // Strip quotes from string fields
        strip_quotes(statTarget);
        strip_quotes(statusEffect);

        Move* m = &moves[count++];
        strncpy_s(m->name, sizeof(m->name), name, sizeof(m->name) - 1);
        m->type = type_from_name(typeStr);
        m->category = string_to_category(catStr);
        m->power = power;
        strncpy_s(m->accuracy, sizeof(m->accuracy), accStr, sizeof(m->accuracy) - 1);
        strncpy_s(m->statTarget, sizeof(m->statTarget), statTarget, sizeof(m->statTarget) - 1);
        m->statChange = statChange;
        strncpy_s(m->statusEffect, sizeof(m->statusEffect), statusEffect, sizeof(m->statusEffect) - 1);
    }

    fclose(file);
    *outCount = count;
    return moves;
}

