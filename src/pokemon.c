#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <conio.h>
#include "pokemon.h"
#include "graphics.h"
#include "game_context.h"

// Global type effectiveness chart
// type_chart[attacking][defending] = multiplier
float type_chart[18][18];

//=====================================================================
// TYPE UTILITIES
//=====================================================================

// Converts a Type enum to a string label for display or parsing
const char* type_to_string(Type type)
{
    switch (type)
    {
    case POKITYPE_NORMAL:   return "NORMAL";
    case POKITYPE_FIRE:     return "FIRE";
    case POKITYPE_WATER:    return "WATER";
    case POKITYPE_GRASS:    return "GRASS";
    case POKITYPE_ELECTRIC: return "ELECTRIC";
    case POKITYPE_ICE:      return "ICE";
    case POKITYPE_FIGHTING: return "FIGHTING";
    case POKITYPE_POISON:   return "POISON";
    case POKITYPE_GROUND:   return "GROUND";
    case POKITYPE_FLYING:   return "FLYING";
    case POKITYPE_PSYCHIC:  return "PSYCHIC";
    case POKITYPE_BUG:      return "BUG";
    case POKITYPE_ROCK:     return "ROCK";
    case POKITYPE_GHOST:    return "GHOST";
    case POKITYPE_DRAGON:   return "DRAGON";
    case POKITYPE_DARK:     return "DARK";
    case POKITYPE_STEEL:    return "STEEL";
    case POKITYPE_FAIRY:    return "FAIRY";
    case POKITYPE_NONE:     return "NONE";
    default:                return "???"; // fallback for unknown
    }
}

// Converts a string name to its corresponding Type enum
Type type_from_name(const char* str)
{
    if (strcmp(str, "NORMAL") == 0)   return POKITYPE_NORMAL;
    if (strcmp(str, "FIRE") == 0)     return POKITYPE_FIRE;
    if (strcmp(str, "WATER") == 0)    return POKITYPE_WATER;
    if (strcmp(str, "GRASS") == 0)    return POKITYPE_GRASS;
    if (strcmp(str, "ELECTRIC") == 0) return POKITYPE_ELECTRIC;
    if (strcmp(str, "ICE") == 0)      return POKITYPE_ICE;
    if (strcmp(str, "FIGHTING") == 0) return POKITYPE_FIGHTING;
    if (strcmp(str, "POISON") == 0)   return POKITYPE_POISON;
    if (strcmp(str, "GROUND") == 0)   return POKITYPE_GROUND;
    if (strcmp(str, "FLYING") == 0)   return POKITYPE_FLYING;
    if (strcmp(str, "PSYCHIC") == 0)  return POKITYPE_PSYCHIC;
    if (strcmp(str, "BUG") == 0)      return POKITYPE_BUG;
    if (strcmp(str, "ROCK") == 0)     return POKITYPE_ROCK;
    if (strcmp(str, "GHOST") == 0)    return POKITYPE_GHOST;
    if (strcmp(str, "DRAGON") == 0)   return POKITYPE_DRAGON;
    if (strcmp(str, "DARK") == 0)     return POKITYPE_DARK;
    if (strcmp(str, "STEEL") == 0)    return POKITYPE_STEEL;
    if (strcmp(str, "FAIRY") == 0)    return POKITYPE_FAIRY;
    if (strcmp(str, "NONE") == 0)     return POKITYPE_NONE;

    // Print error and return default if the type string is unknown
    printf(" Unknown TYPE string: %s\n", str);
    return POKITYPE_NONE;
}


//=====================================================================
// CATEGORY UTILITIES
//=====================================================================
// Converts a string to a MoveCategory enum type (case-insensitive)
// Uses _stricmp for case-insensitive string comparison
// This allows inputs like "buff" or "BUFF" to be recognized correctly
MoveCategory string_to_category(const char* str)
{
    if (_stricmp(str, "PHYSICAL") == 0) return MOVE_PHYSICAL;
    if (_stricmp(str, "SPECIAL") == 0)  return MOVE_SPECIAL;
    if (_stricmp(str, "BUFF") == 0)     return MOVE_BUFF;
    if (_stricmp(str, "DEBUFF") == 0)   return MOVE_DEBUFF;
    if (_stricmp(str, "STATUS") == 0)   return MOVE_STATUS;
    if (_stricmp(str, "CHARGE") == 0)   return MOVE_PHYSICAL; // fallback

    printf("Unknown category '%s', defaulting to PHYSICAL\n", str);
    return MOVE_PHYSICAL;
}

// Converts a MoveCategory enum to its display string
const char* category_to_string(MoveCategory cat)
{
    switch (cat) {
    case MOVE_PHYSICAL: return "Physical";
    case MOVE_SPECIAL:  return "Special";
    case MOVE_BUFF:     return "Buff";
    case MOVE_DEBUFF:   return "Debuff";
    case MOVE_STATUS:   return "Status";
    default:            return "Unknown";
    }
}


//=====================================================================
// TEXT CLEANING
//=====================================================================
// Removes wrapping double quotes from a string (e.g., "Buff" -> Buff)
void strip_quotes(char* str)
{
    size_t len = strlen(str);
    if (len >= 2 && str[0] == '"' && str[len - 1] == '"')
    {
        memmove(str, str + 1, len - 2); // shift all chars left by 1
        str[len - 2] = '\0';           // terminate string early to cut last quote
    }
}


//=====================================================================
// MOVE SEARCH
//=====================================================================
// Looks for a move by name in the full move list (case-insensitive)
// Uses _stricmp to search for a move name case-insensitively
// Ensures "tackle" and "TACKLE" match the same move
const Move* find_move(const char* name, const Move* allMoves, int totalMoves)
{
    for (int i = 0; i < totalMoves; i++) {
        if (_stricmp(name, allMoves[i].name) == 0) {
            return &allMoves[i];
        }
    }
    return NULL; // move not found
}

