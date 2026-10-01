#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>
#include <assert.h>
#include "utils.h"
#include "types.h"
#include "dungeon_fn.h"
#include "constants.h"
#include "enemies.h"


static int next_id = 1;

// ==========================================
// FUNCIONES DE USO DE ÍTEMS
// (deben ser funciones normales, no anidadas,
//  porque las funciones anidadas de GCC no
//  soportan closures reales en MinGW/Windows)
// ==========================================

// --- Equipar espada de hierro ---

void equip_iron_sword(const struct Item* item, struct Character* user, struct Character* target) {
    (void)item; (void)target;
    printf("%s ha equipado la Espada de hierro.\n", user->name);
    enter_to_continue();
}

// --- Equipar armadura de hierro ---

void equip_iron_armor(const struct Item* item, struct Character* user, struct Character* target) {
    (void)item; (void)target;
    printf("%s ha equipado la Armadura de hierro.\n", user->name);
    enter_to_continue();
}


ObjectData create_iron_sword(Weapon* out_weapon) {
    Weapon w;
    w.base_item.type               = TYPE_WEAPON;
    w.base_item.global_id                 = next_id++;
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
    a.base_item.global_id                 = next_id++;
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
    potion.data.item.global_id    = next_id++;
    potion.data.item.function = health_percentage;

    snprintf(potion.data.item.name, MAX_STRING, "Poción de salud %s", suffix);
    snprintf(potion.data.item.description, MAX_STRING + 200,
             "Restaura en un %d%% la salud del personaje", health_percentage);

    potion.data.item.quantity = (quantity > 0) ? quantity : 1;
    potion.data.item.can_use_outside_battle = true;
    potion.data.item.target_type = TARGET_PLAYER;

    // Asignar la función genérica a todas las pociones
    potion.data.item.use_function = use_generic_item;

    return potion;
}
void give_enemy_weapon(Enemy* enemy, Weapon* weapon, const char* name, float damage) {
    weapon->base_item.type               = TYPE_WEAPON;
    weapon->base_item.global_id                 = next_id++;
    strncpy(weapon->base_item.name, name, MAX_STRING - 1);
    weapon->base_item.name[MAX_STRING - 1] = '\0';
    strncpy(weapon->base_item.description, "Arma empuñada por el enemigo", MAX_STRING + 199);
    weapon->base_item.quantity           = 1;
    weapon->base_item.can_use_outside_battle = false;
    weapon->base_item.target_type        = TARGET_ENEMY;
    weapon->base_item.use_function       = NULL;
    weapon->damage                       = damage;
    weapon->durability                   = 100;
    enemy->base_char.weapon              = weapon;
}

void give_enemy_armor(Enemy* enemy, Armor* armor, const char* name, float resistance) {
    armor->base_item.type               = TYPE_ARMOR;
    armor->base_item.global_id                 = next_id++;
    strncpy(armor->base_item.name, name, MAX_STRING - 1);
    armor->base_item.name[MAX_STRING - 1] = '\0';
    strncpy(armor->base_item.description, "Armadura que protege al enemigo", MAX_STRING + 199);
    armor->base_item.quantity           = 1;
    armor->base_item.can_use_outside_battle = false;
    armor->base_item.target_type        = TARGET_PLAYER;
    armor->base_item.use_function       = NULL;
    armor->resistance                   = resistance;
    armor->durability                   = 100;
    enemy->base_char.defense            = armor;
}

void give_enemy_item(Enemy* enemy, ObjectData* item) {
    char* item_name = get_obj_name(item);
    add_item(&enemy->base_char, item, item_name);
    free(item_name);
}


// ==========================================
// MAIN
// ==========================================

int main(void) {
    srand(15);

    const char* main_menu_options[] = {
        "Continuar partida guardada (match_data.dat)",
        "Iniciar nueva partida",
        "Salir"
    };

    printf("¿Que deseas hacer guerrero?\n");
    int op = menu(main_menu_options, 3);

    Player player;
    char username[MAX_STRING] = "";
    int curr_flor = 1;
    int curr_hall = 1;
    int seed = 1;
    int n_flors = 10;
    int n_halls = 5;
    int curr_enemy_range = 'D';

    if (op == 1) {
        if (!load_game_data("match_data.dat", &player, username, &curr_flor, &curr_hall)) {
            printf("Error al cargar la partida guardada.\n");
            return 1;
        }
        printf("\n¡Bienvenido de nuevo, %s!\n", username);
        printf("Reanudando partida desde el Piso %d, Sala %d...\n", curr_flor, curr_hall);
        enter_to_continue();
        dungeon(seed, n_flors, n_halls, curr_flor, curr_hall, curr_enemy_range, &player, username);
    } else if (op == 2) {
        Weapon iron_sword_data;
        Armor  iron_armor_data;

        ObjectData iron_sword_obj = create_iron_sword(&iron_sword_data);
        ObjectData iron_armor_obj = create_iron_armor(&iron_armor_data);

        ObjectData health_potion_I  = create_health_potion(40, "I",  2);
        ObjectData health_potion_II = create_health_potion(70, "II", 1);

        init_character(&player.base_char, "Caballero",
                        20, // attack
                       35, // health
                       35, // hp_max
                       &iron_armor_data, // defense
                       1, // xp_level
                       100, // xp_threshold
                       0, // xp_points
                       &iron_sword_data); // weapon

        add_item(&player.base_char, &iron_sword_obj,    iron_sword_obj.data.weapon.base_item.name);
        add_item(&player.base_char, &iron_armor_obj,    iron_armor_obj.data.armor.base_item.name);
        add_item(&player.base_char, &health_potion_I,   health_potion_I.data.item.name);
        add_item(&player.base_char, &health_potion_II,  health_potion_II.data.item.name);

        dungeon(seed, n_flors, n_halls, curr_flor, curr_hall, curr_enemy_range, &player, "");
    } else {
        printf("Partida finalizada.\n");
    }

    return 0;
}