#pragma once

#include "pokemon.h"

// Type/category converters
const char* type_to_string(Type type);
Type type_from_name(const char* str);
const char* category_to_string(MoveCategory cat);
MoveCategory string_to_category(const char* str);

// Move details
void print_move_details(const Move* move);
const Move* find_move(const char* name, const Move* allMoves, int totalMoves);

// Loader


// Type system
void init_type_chart(void);

// Text cleaner
void strip_quotes(char* str);
