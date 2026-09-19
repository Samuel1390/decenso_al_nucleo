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
// HELPERS PARA EQUIPAR ENEMIGOS
// ==========================================

void give_enemy_weapon(Enemy* enemy, Weapon* weapon, const char* name, float damage) {
    weapon->base_item.type               = TYPE_WEAPON;
    weapon->base_item.id                 = next_id++;
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
    armor->base_item.id                 = next_id++;
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
// TESTS DE DROPEO DE OBJETOS
// ==========================================

// void run_drop_tests(Player* player) {
//     printf("\n========================================================\n");
//     printf("        INICIANDO TESTS DE DROPEO DE OBJETOS            \n");
//     printf("========================================================\n\n");

//     // --- TEST 1: Enemigo con arma equipada (cave_goblin - ID 1) ---
//     printf("--- [TEST 1] Enemigo con arma equipada (cave_goblin) ---\n");
//     Enemy goblin = create_enemy("bestiario.txt", 1, next_id++, player);
//     Weapon goblin_dagger;
//     give_enemy_weapon(&goblin, &goblin_dagger, "Daga de goblin", 2.0f);

//     int count_before = player->base_char.inventory_count;
//     ObjectData dropped;
//     int has_dropped = drop_random_item(&goblin.base_char, &dropped);

//     assert(has_dropped == true);
//     assert(dropped.type == TYPE_WEAPON);
//     assert(strcmp(dropped.data.weapon.base_item.name, "Daga de goblin") == 0);

//     // El jugador toma el objeto dropeado
//     char* dropped_name = get_obj_name(&dropped);
//     added = add_item(&player->base_char, &dropped, dropped_name);
//     free(dropped_name);

//     assert(added == 1);
//     assert(player->base_char.inventory_count == count_before + 1);
//     printf("Resultado: OK -> El jugador tomo con exito '%s'. (Inventario: %d/%d)\n\n",
//            player->base_char.inventory[player->base_char.inventory_count - 1].data.weapon.base_item.name,
//            player->base_char.inventory_count, MAX_INVENTORY);

//     // --- TEST 2: Enemigo con armadura equipada (stone_gargoyle - ID 9) ---
//     printf("--- [TEST 2] Enemigo con armadura equipada (stone_gargoyle) ---\n");
//     Enemy gargoyle = create_enemy("bestiario.txt", 9, next_id++, player);
//     Armor gargoyle_skin;
//     give_enemy_armor(&gargoyle, &gargoyle_skin, "Piel de gargola petrificada", 0.45f);

//     count_before = player->base_char.inventory_count;
//     has_dropped = drop_random_item(&gargoyle.base_char, &dropped);

//     assert(has_dropped == true);
//     assert(dropped.type == TYPE_ARMOR);
//     assert(strcmp(dropped.data.armor.base_item.name, "Piel de gargola petrificada") == 0);

//     // El jugador toma el objeto dropeado
//     dropped_name = get_obj_name(&dropped);
//     added = add_item(&player->base_char, &dropped, dropped_name);
//     free(dropped_name);

//     assert(added == 1);
//     assert(player->base_char.inventory_count == count_before + 1);
//     printf("Resultado: OK -> El jugador tomo con exito '%s'. (Inventario: %d/%d)\n\n",
//            player->base_char.inventory[player->base_char.inventory_count - 1].data.armor.base_item.name,
//            player->base_char.inventory_count, MAX_INVENTORY);

//     // --- TEST 3: Enemigo con consumible en inventario (wandering_skeleton - ID 2) ---
//     printf("--- [TEST 3] Enemigo con objeto en inventario (wandering_skeleton) ---\n");
//     Enemy skeleton = create_enemy("bestiario.txt", 2, next_id++, player);
//     ObjectData bone_potion = create_health_potion(40, "de Hueso", 1);
//     give_enemy_item(&skeleton, &bone_potion);

//     count_before = player->base_char.inventory_count;
//     has_dropped = drop_random_item(&skeleton.base_char, &dropped);

//     assert(has_dropped == true);
//     assert(dropped.type == TYPE_CONSUMABLE);
//     assert(strcmp(dropped.data.item.name, "Poción de salud de Hueso") == 0);

//     // El jugador toma el objeto dropeado
//     dropped_name = get_obj_name(&dropped);
//     added = add_item(&player->base_char, &dropped, dropped_name);
//     free(dropped_name);

//     assert(added == 1);
//     assert(player->base_char.inventory_count == count_before + 1);
//     printf("Resultado: OK -> El jugador tomo con exito '%s'. (Inventario: %d/%d)\n\n",
//            player->base_char.inventory[player->base_char.inventory_count - 1].data.item.name,
//            player->base_char.inventory_count, MAX_INVENTORY);

//     // --- TEST 4: Enemigo sin objetos ni equipo (acid_slime - ID 7) ---
//     printf("--- [TEST 4] Enemigo sin ningun objeto ni equipo (acid_slime) ---\n");
//     Enemy slime = create_enemy("bestiario.txt", 7, next_id++, player);

//     count_before = player->base_char.inventory_count;
//     has_dropped = drop_random_item(&slime.base_char, &dropped);

//     assert(has_dropped == false);
//     assert(player->base_char.inventory_count == count_before);
//     printf("Resultado: OK -> No hubo dropeo (retorno false de forma segura sin abortar ni alterar el inventario).\n\n");

//     // --- TEST 5: Enemigo mixto con arma, armadura e inventario (zombie_knight - ID 4) ---
//     printf("--- [TEST 5] Múltiples tiradas en enemigo con inventario y equipo (zombie_knight) ---\n");
//     Enemy zombie = create_enemy("bestiario.txt", 4, next_id++, player);
//     Weapon zombie_sword;
//     Armor zombie_armor;
//     give_enemy_weapon(&zombie, &zombie_sword, "Mandoble oxidado", 3.0f);
//     give_enemy_armor(&zombie, &zombie_armor, "Cota herrumbrosa", 0.35f);
//     ObjectData zombie_pot = create_health_potion(70, "Antigua", 2);
//     give_enemy_item(&zombie, &zombie_pot);

//     count_before = player->base_char.inventory_count;
//     for (int i = 0; i < 3; i++) {
//         has_dropped = drop_random_item(&zombie.base_char, &dropped);
//         assert(has_dropped == true);
//         dropped_name = get_obj_name(&dropped);
//         printf("    Simulacion %d: dropeo '%s' (Tipo: %d)\n", i + 1, dropped_name, dropped.type);
//         // El jugador toma cada objeto dropeado
//         add_item(&player->base_char, &dropped, dropped_name);
//         free(dropped_name);
//     }
//     assert(player->base_char.inventory_count == count_before + 3);
//     printf("Resultado: OK -> El jugador tomo los 3 objetos generados. (Inventario: %d/%d)\n\n",
//            player->base_char.inventory_count, MAX_INVENTORY);

//     printf("========================================================\n");
//     printf("       TODOS LOS TESTS DE DROPEO PASARON CON ÉXITO      \n");
//     printf("========================================================\n\n");
// }

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
                   10, // xp_level
                   100, // xp_threshold
                   0, // xp_points
                   &iron_sword_data); // weapon

    // Agregar ítems iniciales al inventario del jugador
    add_item(&player.base_char, &iron_sword_obj,    iron_sword_obj.data.weapon.base_item.name);
    add_item(&player.base_char, &iron_armor_obj,    iron_armor_obj.data.armor.base_item.name);
    add_item(&player.base_char, &health_potion_I,   health_potion_I.data.item.name);
    add_item(&player.base_char, &health_potion_II,  health_potion_II.data.item.name);

    // --- Ejecutar tests automáticos de dropeo de objetos ---
    // run_drop_tests(&player);

    // --- Preparar enemigo para la mazmorra con equipo y drops ---
    Enemy enemy4 = create_enemy("bestiario.txt", 26, next_id++, &player);
    Weapon dragon_weapon;
    Armor dragon_armor;
    give_enemy_weapon(&enemy4, &dragon_weapon, "Garra de dragon ceniza", 5.0f);
    give_enemy_armor(&enemy4, &dragon_armor, "Escamas ignifugas", 0.5f);
    ObjectData dragon_potion = create_health_potion(70, "de Dragon", 1);
    give_enemy_item(&enemy4, &dragon_potion);

    int seed = 1;
    int n_flors = 10;
    int n_halls = 5;
    int curr_flor = 1;
    int curr_hall = 1;
    int curr_enemy_range = 'D';
    dungeon(seed, n_flors, n_halls, curr_flor, curr_hall, curr_enemy_range, &player, &enemy4);
    // show_enemy_stats(&enemy4);
    // combat(&player, &enemy4);

    return 0;
}