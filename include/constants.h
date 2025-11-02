#pragma once

//--------------------------------------
// 📦 SYSTEM KEYS
//--------------------------------------
#define KEY_ESC 27
#define KEY_ENTER 13
#define KEY_BACKSPACE 8
#define KEY_ARROW_UP 72
#define KEY_ARROW_DOWN 80
#define KEY_ARROW_LEFT 75
#define KEY_ARROW_RIGHT 77
#define KEY_SPECIAL_0    0
#define KEY_SPECIAL_224  224 
//--------------------------------------
// 🔡 CHARACTER CODES
//--------------------------------------
#define CHAR_PRINTABLE_MIN 32
#define CHAR_PRINTABLE_MAX 126
#define ASCII_DIGIT_MIN '0'
#define ASCII_DIGIT_MAX '9'
//--------------------------------------
// 🎨 COLORS (for compatibility)
//--------------------------------------
// Normal colors
#define COLOR_BLACK "\033[0;30m"
#define COLOR_RED "\033[0;31m"
#define COLOR_GREEN "\033[0;32m"
#define COLOR_YELLOW "\033[0;33m"
#define COLOR_BLUE "\033[0;34m"
#define COLOR_MAGENTA "\033[0;35m"
#define COLOR_CYAN "\033[0;36m"
#define COLOR_WHITE "\033[0;37m"
#define COLOR_GRAY "\033[1;30m"

// Bright (bold) colors
#define COLOR_BRIGHT_RED "\033[1;31m"
#define COLOR_BRIGHT_GREEN "\033[1;32m"
#define COLOR_BRIGHT_YELLOW "\033[1;33m"
#define COLOR_BRIGHT_BLUE "\033[1;34m"
#define COLOR_BRIGHT_MAGENTA "\033[1;35m"
#define COLOR_BRIGHT_CYAN "\033[1;36m"
#define COLOR_BRIGHT_WHITE "\033[1;37m"

#define COLOR_FORBIDDEN   "\x1b[41;97m" // Red background, white text
#define COLOR_LEGENDARY   "\x1b[95m"    // Bright magenta
#define COLOR_CHAMPION    "\x1b[96m"    // Cyan
#define COLOR_VETERAN     "\x1b[92m"    // Light green
#define COLOR_CONTENDER   "\x1b[93m"    // Yellow
#define COLOR_TRAINEE     "\x1b[91m"    // Light red
#define COLOR_FEEBLE      "\x1b[31m"    // Red
#define COLOR_TRAGIC      "\x1b[90m"    // Dark gray
#define COLOR_RESET       "\x1b[0m"     // Reset to default


// Reset
#define COLOR_RESET "\033[0m"
#define BULLET "X"

//--------------------------------------
// 🧠 GAME LOGIC LIMITS
//--------------------------------------
#define MAX_MOVES           4
#define MAX_POKEMON_NAME   32
#define MAX_MOVE_NAME      32
#define MAX_TYPE_NAME      16
#define MAX_LOG_LINE      256
#define MAX_SPRITE_SIZE  163840
#define MAX_STAT_STAGE     6
#define MIN_STAT_STAGE    -6
#define PAGE_SIZE 10

//--------------------------------------
// 📄 FILE PATHS
//--------------------------------------
#define POKEMON_LIST_FILE     "pokemon_list_combined.txt"
#define MOVE_LIST_FILE        "moves_list.txt"
#define SPRITE_FOLDER_PATH       "sprites/"
#define CUSTOM_SPRITE_FOLDER     "sprites/custom_sprites/"
#define SPRITE_ASSET_FOLDER      "sprites/custom_sprites_assets/"
#define SPRITE_MANIFEST_FOLDER   "sprites/custom_sprites_assets/manifests/"
#define POKEMON_SPRITE_FOLDER "sprites/pokemon_sprites/"


//--------------------------------------
// ⚙️ OTHER CONSTANTS
//--------------------------------------
#define INITIAL_POKEMON_CAPACITY 128
#define MAX_SIMULATIONS         20000
#define FIELD_COUNT FIELD_TOTAL_COUNT
#define MOVE_PAGE_SIZE 10
#define CREATE_SUCCESS 1
#define CREATE_CANCELLED 0
#define MAX_SPRITE_SIZE 84000 
#define INITIAL_CAPACITY 128
#define MAX_ID_LEN 32