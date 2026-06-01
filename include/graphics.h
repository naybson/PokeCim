#pragma once

#include <stddef.h> // for size_t
#include "logic.h"  // for Type enum (used in print_type_colored)


// ========== TERMINAL CONTROL ==========

// Clears the console screen
void clearScreen(void);

// Waits for a key press
void pauseUntilKey(void);

// Resets ANSI color to default
void reset_color(void);



// ========== TEXT RENDERING ==========

// Prints colored text using ANSI color strings
void printc(const char* color, const char* format, ...);

// Prints text in RGB color
void printRGB(const char* text, int r, int g, int b);

// Animated typewriter effect
void print_animated(const char* text, int delayMs);

// Prints a Pokemon type (e.g., Fire) in its type color
void print_type_colored(Type type, const char* label);



// ========== SPRITE / NAME DISPLAY ==========

// Shows a Pokemon's name (usually header style)
void printPokemon(const char* name);

// Prints a fully loaded ASCII sprite
void printSpriteBuffer(const char* spriteBuffer);

// Returns a string representing the HP bar
void get_hp_bar_string(int current, int max, int width, char* out, size_t outSize);
