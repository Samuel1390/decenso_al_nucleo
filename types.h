#ifndef TYPES_H
#define TYPES_H

#include <stdbool.h>
#include "constants.h"

// ==========================================
// 1. ENUMERACIONES
// ==========================================

typedef enum {
    TARGET_PLAYER,
    TARGET_ENEMY
} TargetType;

typedef enum {
    DMG_PHYSICAL,
    DMG_MAGICAL
} DamageType;

typedef enum {
    TYPE_CONSUMABLE,
    TYPE_WEAPON,
    TYPE_ARMOR
} ItemType;

// ==========================================
// 2. DECLARACIONES ADELANTADAS (Forward Declarations)
// ==========================================

typedef struct Character Character;
typedef struct Item      Item;
typedef struct ObjectData ObjectData;  // Forward declaration necesaria para Character

// ==========================================
// 3. PUNTEROS A FUNCIÓN
// ==========================================

typedef void (*ItemAction)(Character* target);

// ==========================================
// 4. ESTRUCTURAS PRINCIPALES
// ==========================================

struct Item {
    ItemType  type;
    char      name[MAX_STRING];
    char      description[MAX_STRING + 200];
    int       quantity;
    bool      can_use_outside_battle;
    TargetType target_type;
    int       id;
    ItemAction use_function;
};

typedef struct {
    Item  base_item;
    float damage;      // Amplifica el ataque (0 = sin bono, infinito = máximo)
    int   durability;  // Usos restantes; -1 = infinito
} Weapon;

typedef struct {
    Item  base_item;
    float resistance;  // Reducción de daño físico (0.0 a 1.0)
    int   durability;  // Usos restantes; -1 = infinito
} Armor;

// ObjectData se define ANTES de Character para que Character pueda usarla
struct ObjectData {
    ItemType type;
    union {
        Item   item;
        Weapon weapon;
        Armor  armor;
    } data;
};

// Character contiene el inventario directamente (válido para Player y Enemy)
struct Character {
    char       name[MAX_STRING];
    int        attack;
    int        health;
    int        hp_max;
    Armor*     defense;
    int        xp_level;
    int        xp_threshold;
    int        xp_points;
    Weapon*    weapon;
    ObjectData inventory[MAX_INVENTORY];
    int        inventory_count;
};

// ==========================================
// 5. TIPOS COMPUESTOS
// ==========================================

typedef struct {
    Character base_char;
} Player;

typedef struct {
    Character base_char;
    char rank;  // Rareza del enemigo: 'S' > 'A' > 'B' > 'C' > 'D'
} Enemy;

#endif // TYPES_H