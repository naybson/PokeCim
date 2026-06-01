#pragma once

#include "logic.h"     // for Pokemon, Move, Type
#include "graphics.h"  // for colored printing, if needed
#include "game_context.h"
#include <stdbool.h>   // for bool types

// ========== STATE MANAGEMENT STRUCTS ==========

typedef enum {
    SEARCH_NAME,
    SEARCH_ID
} SearchMode;

typedef enum {
    FOCUS_FILTER,
    FOCUS_SEARCH,
    EDIT_FILTER,
    EDIT_SEARCH,
    FOCUS_LIST
} FocusState;

typedef struct {
    SearchMode mode;
    char name_filter[32];
    Type type_filter;
} SearchState;

// ========== MAIN MENU ==========
void main_menu(GameContext* context); // Added the missing closing parenthesis

// ========== POKEDEX MENU ==========
int display_menu(const Pokemon pokemons[], int count, SearchState* search, int* selected, int* start_index);

// ========== NAVIGATION + SELECTION ==========

void handle_navigation_keys(int ch, FocusState* focus, int* selected, int* start_index, int filtered_count, int page_end);
int  handle_enter(FocusState* focus, int* selected, int* start_index, const int* filtered_indices, int filtered_count);

// ========== FILTER + SEARCH BAR LOGIC ==========

int  build_filtered_list(const Pokemon* pokemons, int count, const SearchState* search, int* filtered_indices);
void handle_edit_search(int ch, SearchState* search, int* selected, int* start_index);
void handle_edit_filter(int ch, SearchState* search, int* selected, int* start_index);
char* strstr_case_insensitive(const char* haystack, const char* needle);

// ========== RENDERING UI ==========

void draw_search_bar(const SearchState* search, FocusState focus);
void draw_filtered_list(const Pokemon* pokemons, const int* filtered_indices, int start_index, int end_index, int selected, FocusState focus);

// ========== DETAIL VIEW ==========

void describe_move(const Move* m, char* outBuf, int bufSize);
int  print_pokemon_details_interactive(const Pokemon* p, const Move* allMoves, int totalMoves);

// ========== ASSET LOADING ==========
Pokemon* load_pokemon_list(const char* filename, int* outCount);
char* loadPokemonSpriteByID(const char* id); // used in details viewer
