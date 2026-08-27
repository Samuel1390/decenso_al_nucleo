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
// Permiten romper el bucle de dependencia circular
// ==========================================

typedef struct Character Character;
typedef struct Item Item;

// 3. PUNTEROS A FUNCIÓN

typedef void (*ItemAction)(Character* target, Item* item);

// 4. ESTRUCTURAS PRINCIPALES

struct Item {
    ItemType type;
    char name[MAX_STRING];
    char description[MAX_STRING];
    int quantity;
    bool can_use_outside_battle;
    TargetType target_type; // Define qué entidad se ve afectada
    int id; // Identificador único del objeto
    ItemAction use_function; // Puntero a función para aplicar el efecto
};

typedef struct {
    Item base_item;
    float damage; // Numero entre 0 e infinito, amplifica el ataque
    int durability; // Usos restantes antes de romperse, si es -1 es infinito
} Weapon;

typedef struct {
    Item base_item;
    float resistance; // Para recibir ataques físicos, entre 0 y 1 (0% a 100%)
    int durability; // Usos restantes antes de romperse, si es -1 es infinito
} Armor;

struct Character {
    char name[MAX_STRING];
    int attack;
    int health;
    int hp_max;
    Armor* defense;
    int xp_level;
    int xp_threshold;
    int xp_points;
    Weapon* weapon;
};

typedef struct {
    ItemType type;
    int id;
    union {
        Item item;
        Weapon weapon;
        Armor armor;
    } data;
} ObjectData;

typedef struct {
    Character base_char;
    ObjectData inventory[MAX_INVENTORY];
    int inventory_count;
} Player;

typedef struct {
    Character base_char;
    ObjectData inventory[MAX_ENEMY_INVENTORY];
    int inventory_count;
    char range; // Rango del enemigo: 'S', 'A', 'B', 'C', 'D'
} Enemy;

#endif // TYPES_H