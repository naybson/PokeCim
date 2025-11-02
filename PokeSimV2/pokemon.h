#pragma once
#define ORIGINAL_POKEMON_NUM 1000
#define MAX_MOVES 4
#define TYPE_COUNT 18

// ========== TYPE + CATEGORY ENUMS ==========
typedef enum {
    POKITYPE_NORMAL,
    POKITYPE_FIRE,
    POKITYPE_WATER,
    POKITYPE_GRASS,
    POKITYPE_ELECTRIC,
    POKITYPE_ICE,
    POKITYPE_FIGHTING,
    POKITYPE_POISON,
    POKITYPE_GROUND,
    POKITYPE_FLYING,
    POKITYPE_PSYCHIC,
    POKITYPE_BUG,
    POKITYPE_ROCK,
    POKITYPE_GHOST,
    POKITYPE_DRAGON,
    POKITYPE_DARK,
    POKITYPE_STEEL,
    POKITYPE_FAIRY,
    POKITYPE_NONE,
    POKITYPE_COUNT
} Type;

typedef enum {
    PHYSICAL,
    SPECIAL,
    STATUS
} MoveCategory;

// ========== CORE STRUCTS ==========

typedef struct {
    char name[32];
    Type type;
    int power;
    char accuracy[8];
    MoveCategory category;
    char statusEffect[32];
    char statTarget[32];
    int statChange;
} Move;

typedef struct {
    char name[32];
    char idnumber[10];
    Type type1, type2;
    int hp, attack, defense, spAttack, spDefence, speed;
    char moves[4][20];  // list of move names
} Pokemon;

typedef enum {
    MOVE_PHYSICAL,
    MOVE_SPECIAL,
    MOVE_BUFF,
    MOVE_DEBUFF,
    MOVE_STATUS,
    MOVE_UNKNOWN   // fallback or invalid value
} MoveCategory;