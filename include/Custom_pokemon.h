#pragma once

#include "pokemon.h"
#include "constants.h"

// ========== ENUMS ==========

typedef enum {
    MOVE_FOCUS_FILTER,
    MOVE_FOCUS_SEARCH,
    MOVE_FOCUS_LIST,
    MOVE_EDIT_FILTER,
    MOVE_EDIT_SEARCH
} MoveFocusState;

typedef enum  {
    FIELD_NAME, FIELD_TYPE1, FIELD_TYPE2,
    FIELD_HP, FIELD_ATTACK, FIELD_DEFENSE,
    FIELD_SPATK, FIELD_SPDEF, FIELD_SPEED,
    FIELD_MOVE_1, FIELD_MOVE_2, FIELD_MOVE_3, FIELD_MOVE_4,
    FIELD_SUBMIT, FIELD_SPRITE_MAKER,
    FIELD_TOTAL_COUNT
}Field;


// ========== STRUCTS ==========

typedef struct {
    bool canSubmit;
    bool fieldValid[FIELD_TOTAL_COUNT];
    char messages[10][128];
    int messageCount;
} ValidationResult;

typedef struct {
    char name_filter[32];
    Type type_filter;
} MoveSearchState;


// ========== CORE ENTRY ==========

int create_custom_pokemon(Move* moveList, int moveCount);


// ========== VALIDATION / UTILS ==========

ValidationResult validate_custom_pokemon(const char* name, Type type1, Type type2, int hp, int atk, int def, int spAtk, int spDef, int speed, char moveNames[4][20], const Pokemon* existingList, int totalExisting);
int calculate_total_stats(int hp, int atk, int def, int spAtk, int spDef, int speed);
const char* grade_from_total(int total, const char** color);
void to_uppercase(char* dest, const char* src, size_t maxLen);
bool sprite_exists(const char* pokemonID);


// ========== INPUT HANDLING ==========

void handle_input(int ch, int* selected, Type* type1, Type* type2, const char* pokemonID);
bool edit_field(int selected, char* name, int* hp, int* atk, int* def, int* spAtk, int* spDef, int* speed, char moveNames[4][20], Move* moveList, int moveCount, const char* pokemonID);


// ========== MOVE MENU ==========

void select_single_move(char* targetMove, char moveNames[4][20], int slot, Move* moveList, int moveCount);
int build_filtered_move_list(const Move* moves, int count, const MoveSearchState* search, int* filtered_indices);
int display_move_menu(const Move* moves, int total, const char moveNames[4][20], int slot);


// ========== UI RENDERING ==========

void render_form(const char* name, Type type1, Type type2, int hp, int atk, int def, int spAtk, int spDef, int speed, char moveNames[4][20], int selected, const ValidationResult* validation, Move* moveList, int moveCount, const char* pokemonID);
void render_messages(const ValidationResult* validation);
void render_header(int totalStats, const char* grade, const char* color);
void draw_move_search_bar(const MoveSearchState* search, MoveFocusState focus);

void render_name_field(const char* name, const char* arrow, const char* marker);
void render_type_field(const char* label, Type type, const char* arrow, const char* marker);
void render_stat_field(const char* label, int value, const char* arrow, const char* marker);
void render_move_field(int index, const char* moveName, const char* arrow, const char* marker, Move* moveList, int moveCount);
void render_submit_field(bool canSubmit, const char* arrow);


// ========== MAIN SPRITE CREATION INTERFACE ==========

void creature_selector_menu_with_id(const char* pokemonID);

// ========== COLOR PICKER ==========

void color_selection_menu(int* r, int* g, int* b);

// ========== FILE SYSTEM LOADERS ==========

int load_category_list(char*** outFileList, int* outCount);
int load_creature_list(const char* category, char*** outFileList, int* outCount);
char* load_ascii_creature(const char* category, const char* filename);
void save_sprite_to_file(const char* id, const char* sprite);

// ========== MISC HELPERS ==========

const char* strip_txt_extension(const char* filename);