# PokeCim — Terminal Pokédex & Fusion Lab

A **terminal-based Pokédex application written in C** that runs entirely in the terminal.
Browse 1000+ Pokémon, search by name or type, render colorful ANSI sprites, and fuse Pokémon together to create new creatures — all without leaving the terminal.

> **Platform:** Windows & Linux — cross-platform C, no external dependencies.

---

## Features

| Feature | Description |
|---|---|
| Pokédex Browser | Browse 1000+ Pokémon from the National Pokédex |
| Search | Find Pokémon by name or type |
| Sprite Renderer | Render colorful ASCII sprites directly in the Windows terminal |
| Fusion Lab | Combine two Pokémon to create a new fused creature |
| Stats Display | View full stats, types, and movesets |
| Custom Pokémon | Create and save your own Pokémon via a text file |

---

## Technical Highlights

- **Modular C architecture** — `GameContext` struct centralizes state, avoiding global variables entirely
- **Windows Console API** — enables ANSI escape codes and UTF-8 for colored output on Windows
- **18-type system** — full type effectiveness chart (`float[19][19]`) loaded at startup
- **File-based data pipeline** — Pokémon and moves loaded from structured `.txt` files into dynamic arrays
- **Sprite system** — pre-rendered ASCII art sprites stored per-Pokémon, rendered frame-by-frame in the terminal
- **Memory management** — all heap allocations tracked and freed on exit

---

## Project Structure

```
PokeCim/
├── src/                    # All C source files
│   ├── main.c              # Entry point, console setup, data loading
│   ├── pokemon.c           # Pokémon data structures and utilities
│   ├── pokemon_loader.c    # File parser: loads pokemon_list_combined.txt
│   ├── graphics.c          # Terminal sprite rendering
│   ├── main_menu.c         # Interactive menu system
│   ├── Pokedex.c           # Pokédex browse & search logic
│   └── Pokemon_maker.c     # Fusion & custom Pokémon creation
│
├── include/                # Header files
│   ├── pokemon.h           # Core structs: Pokemon, Move, Type enum
│   ├── game_context.h      # GameContext: central state struct
│   ├── graphics.h          # Sprite rendering interface
│   ├── logic.h             # Type chart, move utilities
│   └── ...
│
├── assets/                 # Data files
│   ├── pokemon_list_combined.txt   # Full Pokédex data
│   ├── moves_list.txt              # Move database
│   └── custom_pokemon.txt          # User-created Pokémon
│
└── sprites/                # ASCII art sprite files (one per Pokémon)
```

---

## Build & Run

### Linux
```bash
git clone https://github.com/naybson/PokeCim.git
cd PokeCim
make
./pokecim
```
> Requires: `gcc`, `make`. Use a terminal with ANSI color support (any modern terminal).

### Windows — No Build Required
1. Open the `download/` folder
2. Double-click **`PokeCim.exe`** — runs straight from there, no install needed

> Works on Windows 10/11.

### Windows — Build from Source

**Requirements:** Visual Studio 2019 or later

```bash
git clone https://github.com/naybson/PokeCim.git
```

1. Open `PokeSimV2.sln` in Visual Studio
2. Set configuration to **Release x64**
3. Press **Ctrl+F5** to build and run

---

## Built With

- **C** (C99)
- Windows Console API (`windows.h`)
- ANSI escape codes for terminal colors
- No external libraries — pure C standard library

---

## Credits
- Base Pokémon sprites adapted from the [krabby](https://github.com/yannjor/krabby) colorscript library (GPL-3.0)
- Fusion sprites sourced from the Pokémon Infinite Fusion community project
- Pokémon is a trademark of Nintendo/Game Freak. This project is fan-made and not affiliated with or endorsed by Nintendo.
