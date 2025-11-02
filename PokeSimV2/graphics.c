#include <stdio.h>           // Needed for printf and scanf
#include <stdlib.h>          // For system("cls") and other stuff
#include <windows.h>         // For Sleep() on Windows
#include <conio.h> // for _kbhit() and _getch()
#include <string.h>
#include <wchar.h>
#include <locale.h>
#include <ctype.h>

#include "graphics.h"
#include "pokemon.h"
#include "constants.h"
#include "game_context.h"
//=====================================================================
// TERMINAL CONTROL UTILITIES
//=====================================================================

// Clears the terminal screen using the appropriate command for the OS
void clearScreen()
{
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

// Waits until the user presses any key before continuing
void pauseUntilKey()
{
    printf("Press any key to continue...\n");
    _getch();
}


//=====================================================================
// TEXT RENDERING UTILITIES
//=====================================================================

// Prints text in a specified RGB color in the terminal
void printRGB(const char* text, int r, int g, int b)
{
    printf("\033[38;2;%d;%d;%dm%s\033[0m", r, g, b, text);
}

// Prints formatted text in a given ANSI color string, then resets color
void printc(const char* color, const char* format, ...)
{
    va_list args;
    va_start(args, format);

    printf("%s", color);        // Apply ANSI color
    vprintf(format, args);     // Print formatted string
    va_end(args);

    printf(COLOR_RESET);       // Reset to default terminal color
}

// Animates text output by printing characters one at a time
void print_animated(const char* text, int delayMs)
{
    for (int i = 0; text[i] != '\0'; i++)
    {
        putchar(text[i]);     // Print one character
        fflush(stdout);       // Force output to appear immediately
        Sleep(delayMs);       // Wait for specified milliseconds
    }
}

// Resets terminal color formatting to default
void reset_color()
{
    printf("\033[0m");
}


//=====================================================================
// TYPE-BASED COLOR RENDERING
//=====================================================================

// Prints a label with a color based on Pokemon type
void print_type_colored(Type type, const char* label)
{
    switch (type)
    {
    case POKITYPE_NORMAL:   printRGB(label, 169, 169, 169); break;  // Gray
    case POKITYPE_FIRE:     printRGB(label, 255, 80, 0); break;     // Orange-red
    case POKITYPE_WATER:    printRGB(label, 0, 120, 255); break;    // Blue
    case POKITYPE_GRASS:    printRGB(label, 0, 200, 0); break;      // Green
    case POKITYPE_ELECTRIC: printRGB(label, 255, 230, 0); break;    // Yellow
    case POKITYPE_ICE:      printRGB(label, 150, 255, 255); break;  // Light cyan
    case POKITYPE_FIGHTING: printRGB(label, 192, 48, 40); break;    // Red-brown
    case POKITYPE_POISON:   printRGB(label, 160, 64, 160); break;   // Purple
    case POKITYPE_GROUND:   printRGB(label, 210, 180, 100); break;  // Sandy brown
    case POKITYPE_FLYING:   printRGB(label, 135, 206, 235); break;  // Sky blue
    case POKITYPE_PSYCHIC:  printRGB(label, 255, 105, 180); break;  // Hot pink
    case POKITYPE_BUG:      printRGB(label, 150, 200, 0); break;    // Olive green
    case POKITYPE_ROCK:     printRGB(label, 184, 134, 11); break;   // Earthy yellow
    case POKITYPE_GHOST:    printRGB(label, 123, 104, 238); break;  // Indigo
    case POKITYPE_DRAGON:   printRGB(label, 106, 90, 205); break;   // Slate blue
    case POKITYPE_DARK:     printRGB(label, 60, 60, 60); break;     // Dark gray
    case POKITYPE_STEEL:    printRGB(label, 192, 192, 192); break;  // Silver
    case POKITYPE_FAIRY:    printRGB(label, 255, 182, 193); break;  // Light pink
    case POKITYPE_NONE:     printRGB(label, 255, 255, 255); break;  // White
    default:                printRGB(label, 255, 255, 255); break;  // Fallback: white
    }
}


