#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>
#include "utils.h"
#include "types.h"
#include "constants.h"

// ==========================================
// CONTADOR GLOBAL DE IDs
// ==========================================

static int next_id = 1;

// ==========================================
// FUNCIONES DE USO DE ÍTEMS
// (deben ser funciones normales, no anidadas,
//  porque las funciones anidadas de GCC no
//  soportan closures reales en MinGW/Windows)
// ==========================================

// --- Pociones de salud ---

void use_health_potion_40(Character* target) {
    float pct = 0.40f;
    int recovered = (int)(target->hp_max * pct);
    target->health = (int)min_f((float)(target->health + recovered), (float)target->hp_max);
    printf("%s ha recuperado %d puntos de salud\n", target->name, recovered);
    draw_progress_bar(target->health, target->hp_max, "HP");
    enter_to_continue();
}

void use_health_potion_70(Character* target) {
    float pct = 0.70f;
    int recovered = (int)(target->hp_max * pct);
    target->health = (int)min_f((float)(target->health + recovered), (float)target->hp_max);
    printf("%s ha recuperado %d puntos de salud\n", target->name, recovered);
    draw_progress_bar(target->health, target->hp_max, "HP");
    enter_to_continue();
}

// --- Equipar espada de hierro ---

void equip_iron_sword(Character* c) {
    // NOTA: el arma real debe ser pasada por contexto externo.
    // Esta función marca visualmente el equipamiento; la asignación
    // de c->weapon se hace directamente desde main/create_iron_sword.
    printf("%s ha equipado la Espada de hierro.\n", c->name);
    enter_to_continue();
}

// --- Equipar armadura de hierro ---

void equip_iron_armor(Character* c) {
    printf("%s ha equipado la Armadura de hierro.\n", c->name);
    enter_to_continue();
}

// ==========================================
// FACTORÍAS DE ÍTEMS
// ==========================================

ObjectData create_iron_sword(Weapon* out_weapon) {
    Weapon w;
    w.base_item.type               = TYPE_WEAPON;
    w.base_item.id                 = next_id++;
    strncpy(w.base_item.name,        "Espada de hierro", MAX_STRING - 1);
    strncpy(w.base_item.description, "Arma cuerpo a cuerpo muy popular y eficaz", MAX_STRING + 199);
    w.base_item.quantity           = 1;
    w.base_item.can_use_outside_battle = false;
    w.base_item.target_type        = TARGET_ENEMY;
    w.base_item.use_function       = equip_iron_sword;
    w.damage                       = 3.0f;
    w.durability                   = 100;

    *out_weapon = w;           // Escribir en el buffer del llamador

    ObjectData obj;
    obj.type         = TYPE_WEAPON;
    obj.data.weapon  = w;
    return obj;
}

ObjectData create_iron_armor(Armor* out_armor) {
    Armor a;
    a.base_item.type               = TYPE_ARMOR;
    a.base_item.id                 = next_id++;
    strncpy(a.base_item.name,        "Armadura de hierro", MAX_STRING - 1);
    strncpy(a.base_item.description, "Armadura que protege el cuerpo de ataques físicos", MAX_STRING + 199);
    a.base_item.quantity           = 1;
    a.base_item.can_use_outside_battle = false;
    a.base_item.target_type        = TARGET_PLAYER;
    a.base_item.use_function       = equip_iron_armor;
    a.resistance                   = 0.5f;   // 50% reducción de daño físico
    a.durability                   = 100;

    *out_armor = a;

    ObjectData obj;
    obj.type        = TYPE_ARMOR;
    obj.data.armor  = a;
    return obj;
}

ObjectData create_health_potion(int health_percentage, const char* suffix, int quantity) {
    ObjectData potion;
    potion.type = TYPE_CONSUMABLE;
    potion.data.item.type  = TYPE_CONSUMABLE;
    potion.data.item.id    = next_id++;

    // Construir nombre y descripción
    snprintf(potion.data.item.name,        MAX_STRING,
             "Poción de salud %s",         suffix);
    snprintf(potion.data.item.description, MAX_STRING + 200,
             "Restaura en un %d%% la salud del personaje", health_percentage);

    potion.data.item.quantity              = (quantity > 0) ? quantity : 1;
    potion.data.item.can_use_outside_battle = true;
    potion.data.item.target_type           = TARGET_PLAYER;

    // Seleccionar función de uso según el porcentaje
    if (health_percentage <= 40) {
        potion.data.item.use_function = use_health_potion_40;
    } else {
        potion.data.item.use_function = use_health_potion_70;
    }

    return potion;
}

// ==========================================
// MAIN
// ==========================================

int main(void) {
    srand((unsigned int)time(NULL));
    Weapon iron_sword_data;
    Armor  iron_armor_data;

    ObjectData iron_sword_obj = create_iron_sword(&iron_sword_data);
    ObjectData iron_armor_obj = create_iron_armor(&iron_armor_data);

    ObjectData health_potion_I  = create_health_potion(40, "I",  2);
    ObjectData health_potion_II = create_health_potion(70, "II", 1);

    // --- Crear jugador ---
    Player player;
    init_character(&player.base_char, "Caballero",
                    80, // attack
                   100, // health
                   100, // hp_max
                   &iron_armor_data, // defense
                   1, // xp_level
                   100, // xp_threshold
                   0, // xp_points
                   &iron_sword_data); // weapon

    // Agregar ítems al inventario del jugador
    add_item(&player.base_char, &iron_sword_obj,    iron_sword_obj.data.weapon.base_item.name);
    add_item(&player.base_char, &iron_armor_obj,    iron_armor_obj.data.armor.base_item.name);
    add_item(&player.base_char, &health_potion_I,   health_potion_I.data.item.name);
    add_item(&player.base_char, &health_potion_II,  health_potion_II.data.item.name);

    // --- Crear enemigos ---
    Enemy enemy1, enemy2, enemy3;

    init_character(&enemy1.base_char, "Goblin",
                     20, // attack
                     80, // health
                     80, // hp_max
                     &iron_armor_data, // defense
                     1, // xp_level
                     100, // xp_threshold
                     0, // xp_points
                     &iron_sword_data); // weapon
    enemy1.rank = 'D';
    add_item(&enemy1.base_char, &health_potion_I, health_potion_I.data.item.name);

    init_character(&enemy2.base_char, "Goblin",
                     30, // attack
                     90, // health
                     90, // hp_max
                     &iron_armor_data, // defense
                     4, // xp_level
                     140, // xp_threshold
                     50, // xp_points
                     &iron_sword_data); // weapon
    enemy2.rank = 'C';
    add_item(&enemy2.base_char, &health_potion_I, health_potion_I.data.item.name);

    init_character(&enemy3.base_char, "Esqueleto",
                     32, // attack
                     90, // health
                     90, // hp_max
                     &iron_armor_data, // defense
                     10, // xp_level
                     240, // xp_threshold
                     50, // xp_points
                     &iron_sword_data); // weapon
    enemy3.rank = 'B';
    add_item(&enemy3.base_char, &health_potion_I, health_potion_I.data.item.name);

    // --- Combates ---
    combat(&player, &enemy3);
    show_player_stats(&player);
    combat(&player, &enemy2);
    show_player_stats(&player);

    return 0;
}