#ifndef UTILS_H
#define UTILS_H

#include "types.h"
#include "constants.h"
#include "getters.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// ==========================================
// UTILIDADES GENERALES
// ==========================================

float get_random(float lo, float hi) {
    float scale = (float)rand() / (float)RAND_MAX;
    return lo + scale * (hi - lo);
}

float min_f(float a, float b) { return (a < b) ? a : b; }
float max_f(float a, float b) { return (a > b) ? a : b; }

void enter_to_continue() {
    printf("[Enter] para continuar...\n");
    while (getchar() != '\n');
}

// Validación robusta de entrada de enteros
int get_int(const char* prompt) {
    char buffer[256];
    int value;
    while (1) {
        printf("%s", prompt);
        if (fgets(buffer, sizeof(buffer), stdin) != NULL) {
            if (sscanf(buffer, "%d", &value) == 1) return value;
        }
        printf("Entrada inválida. Por favor, ingresa un número válido.\n");
    }
}

// Menú dinámico y robusto
int menu(const char* options[], int num_options) {
    printf("\n\nEscribe el número de la opción para seleccionarla:\n");
    for (int i = 0; i < num_options; i++) {
        printf("%d. %s\n", i + 1, options[i]);
    }
    int selected;
    while (1) {
        selected = get_int("> ");
        if (selected >= 1 && selected <= num_options) return selected;
        printf("Opción fuera de rango. Selecciona un número entre 1 y %d.\n", num_options);
    }
}

void draw_progress_bar(float current_value, float max_value, char label[]) {
    int filled = (int)(current_value / max_value * 50);
    printf("[");
    for (int i = 0; i < filled;  i++) printf("#");
    for (int i = filled; i < 50; i++) printf("-");
    printf("] %s %d/%d\n", label, (int)current_value, (int)max_value);
}

// ==========================================
// INVENTARIO (operan sobre Character*)
// ==========================================

// Elimina un objeto del inventario desplazando el arreglo
void remove_item(Character* c, int index) {
    for (int i = index; i < c->inventory_count - 1; i++) {
        c->inventory[i] = c->inventory[i + 1];
    }
    c->inventory_count--;
}

// Agrega un objeto al inventario del personaje
int add_item(Character* c, ObjectData* obj, const char* obj_name) {
    if (c->inventory_count < MAX_INVENTORY) {
        c->inventory[c->inventory_count] = *obj;
        c->inventory_count++;
        printf("%s ha obtenido %s\n", c->name, obj_name);
        return 1;
    }
    printf("Inventario lleno\n");
    return 0;
}

// Busca un objeto por ID; retorna su índice o -1 si no existe
int get_idx_from_id(Character* c, int id) {
    for (int i = 0; i < c->inventory_count; i++) {
        switch (c->inventory[i].type) {
            case TYPE_CONSUMABLE:
                if (c->inventory[i].data.item.id == id) return i;
                break;
            case TYPE_WEAPON:
                if (c->inventory[i].data.weapon.base_item.id == id) return i;
                break;
            case TYPE_ARMOR:
                if (c->inventory[i].data.armor.base_item.id == id) return i;
                break;
        }
    }
    return -1;
}

// Elimina ítems agotados o con durabilidad 0
void update_inventory(Character* c) {
    for (int i = 0; i < c->inventory_count; i++) {
        int remove = 0;
        switch (c->inventory[i].type) {
            case TYPE_CONSUMABLE:
                remove = (c->inventory[i].data.item.quantity <= 0);
                break;
            case TYPE_WEAPON:
                remove = (c->inventory[i].data.weapon.durability == 0 ||
                          c->inventory[i].data.weapon.base_item.quantity <= 0);
                break;
            case TYPE_ARMOR:
                remove = (c->inventory[i].data.armor.durability == 0 ||
                          c->inventory[i].data.armor.base_item.quantity <= 0);
                break;
        }
        if (remove) {
            remove_item(c, i);
            i--; // reajustar índice tras desplazar
        }
    }
}

// Setters auxiliares para ObjectData
void set_obj_quantity(ObjectData* obj, int quantity) {
    switch (obj->type) {
        case TYPE_CONSUMABLE: obj->data.item.quantity = quantity;              break;
        case TYPE_WEAPON:     obj->data.weapon.base_item.quantity = quantity;  break;
        case TYPE_ARMOR:      obj->data.armor.base_item.quantity  = quantity;  break;
    }
}

void use_obj_fn(ObjectData* obj, Character* target) {
    ItemAction fn = NULL;
    switch (obj->type) {
        case TYPE_CONSUMABLE: fn = obj->data.item.use_function;              break;
        case TYPE_WEAPON:     fn = obj->data.weapon.base_item.use_function;  break;
        case TYPE_ARMOR:      fn = obj->data.armor.base_item.use_function;   break;
    }
    if (fn) fn(target);
}

// Abre y gestiona el inventario de un personaje
char* open_inventory(Character* c, bool in_battle, Character* enemy) {
    if (c->inventory_count == 0) {
        printf("\nEl inventario está vacío.\n");
        enter_to_continue();
        return "Salir";
    }

    const char* inv_ops[MAX_INVENTORY + 1];
    for (int i = 0; i < c->inventory_count; i++) {
        inv_ops[i] = get_obj_name(&c->inventory[i]);
    }
    inv_ops[c->inventory_count] = "Salir";

    int chosen = menu(inv_ops, c->inventory_count + 1);
    if (chosen == c->inventory_count + 1) return "Salir";

    int selected_index = chosen - 1;
    ObjectData selected_item = c->inventory[selected_index];

    printf("\n--- %s ---\n",  get_obj_name(&selected_item));
    printf("%s\n",            get_obj_desc(&selected_item));
    printf("Cantidad: %d\n",  get_obj_quantity(&selected_item));

    if (in_battle && enemy != NULL) {
        const char* battle_ops[] = {"Usar", "Volver"};
        int accion = menu(battle_ops, 2);

        if (accion == 1) {
            if (get_obj_target(&selected_item) == TARGET_ENEMY) {
                use_obj_fn(&selected_item, enemy);
            } else if (get_obj_target(&selected_item) == TARGET_PLAYER) {
                use_obj_fn(&selected_item, c);
            }
            set_obj_quantity(&selected_item, get_obj_quantity(&selected_item) - 1);
            // Propagar la cantidad modificada de vuelta al inventario real
            set_obj_quantity(&c->inventory[selected_index], get_obj_quantity(&selected_item));
            if (get_obj_quantity(&selected_item) <= 0) remove_item(c, selected_index);
            return "Usar";
        } else if (accion == 2) {
            return open_inventory(c, in_battle, enemy);
        }
    } else {
        const char* peace_ops[] = {"Usar", "Soltar", "Volver"};
        int accion = menu(peace_ops, 3);

        if (accion == 1) {
            if (get_obj_target(&selected_item) == TARGET_PLAYER &&
                get_obj_can_use_outside_battle(&selected_item)) {
                use_obj_fn(&selected_item, c);
                set_obj_quantity(&selected_item, get_obj_quantity(&selected_item) - 1);
                set_obj_quantity(&c->inventory[selected_index], get_obj_quantity(&selected_item));
                if (get_obj_quantity(&selected_item) <= 0) remove_item(c, selected_index);
                return "Usar";
            } else {
                printf("\nNo puedes usar este objeto fuera de combate.\n");
                enter_to_continue();
                return open_inventory(c, in_battle, enemy);
            }
        } else if (accion == 2) {
            int amount_to_drop = 1;
            if (get_obj_quantity(&selected_item) > 1) {
                printf("\nTienes x%d de este objeto.\n", get_obj_quantity(&selected_item));
                amount_to_drop = get_int("Cantidad a soltar: ");
                if (amount_to_drop <= 0) {
                    printf("Cancelado.\n");
                    return open_inventory(c, in_battle, enemy);
                }
                if (amount_to_drop > get_obj_quantity(&selected_item)) {
                    amount_to_drop = get_obj_quantity(&selected_item);
                }
            }
            int new_qty = get_obj_quantity(&selected_item) - amount_to_drop;
            set_obj_quantity(&c->inventory[selected_index], new_qty);
            printf("\nHas soltado %s (x%d)\n", get_obj_name(&selected_item), amount_to_drop);
            if (new_qty <= 0) remove_item(c, selected_index);
            enter_to_continue();
            return open_inventory(c, in_battle, enemy);
        } else if (accion == 3) {
            return open_inventory(c, in_battle, enemy);
        }
    }
}

// ==========================================
// FUNCIONES DE PERSONAJE
// ==========================================

void init_character(Character* c, const char* name, int attack, int health, int hp_max,
                    Armor* defense, int xp_level, int xp_threshold, int xp_points, Weapon* weapon) {
    strncpy(c->name, name, MAX_STRING - 1);
    c->name[MAX_STRING - 1] = '\0';
    c->attack        = attack;
    c->hp_max        = hp_max;
    c->defense       = defense;
    c->xp_level      = xp_level;
    c->xp_threshold  = xp_threshold;
    c->xp_points     = xp_points;
    c->weapon        = weapon;
    c->inventory_count = 0;

    if (health > 0 && health <= hp_max) {
        c->health = health;
    } else {
        printf("Error: Salud inválida. Forzando a 1.\n");
        c->health = 1;
    }
}

int calculate_attack(Character* c) {
    if (c->weapon != NULL) {
        int w_damage = (int)sqrtf(c->weapon->damage * c->attack);
        return (c->attack > w_damage) ? c->attack : w_damage;
    }
    return c->attack;
}

void take_damage(Character* c, int damage, DamageType type) {
    int damage_received = 0;
    if (type == DMG_PHYSICAL) {
        if (c->defense != NULL) {
            // resistance reduce el daño: damage * (1 - resistance)
            damage_received = (int)(damage * (1.0f - c->defense->resistance));
        } else {
            damage_received = damage;  // Sin armadura, daño completo
        }
    } else if (type == DMG_MAGICAL) {
        damage_received = damage;
    }
    if (damage_received < 0) damage_received = 0;

    c->health -= damage_received;
    if (c->health <= 0) {
        c->health = 0;
        printf("%s ha muerto.\n", c->name);
    }
}

void level_up(Character* c, int xp_gained) {
    int xp_remaining = xp_gained + c->xp_points;
    while (xp_remaining >= c->xp_threshold) {
        xp_remaining   -= c->xp_threshold;
        c->attack       += c->attack      / 10;
        c->hp_max       += c->hp_max      / 10;
        c->xp_threshold += c->xp_threshold / 10;
        c->xp_level     += 1;
        printf("¡%s subió de nivel!\n", c->name);
    }
    c->xp_points = xp_remaining;
}

float escape_chance(float player_attack, float enemy_attack) {
    float random_factor = get_random(1, 3);
    float escape = (player_attack * 51.2f / enemy_attack) + 12.0f * random_factor;
    if (escape > 100.0f) escape = 100.0f;
    return escape / 100.0f;
}

// ==========================================
// DROP DE OBJETOS (opera sobre Character*)
// ==========================================

// Construye un ObjectData a partir del arma equipada del personaje
ObjectData drop_random_item(Character* c) {
    float random_num = get_random(0, 1);
    // EL personaje tiene un 50% de probabilidad de soltar ya sea su espada o su armadura
    if (random_num > 0.5f) {
        if (c->weapon != NULL && c->defense != NULL) {
            ObjectData item;
            if (random_num > 0.75f) {
                item.type = TYPE_WEAPON;
                item.data.weapon = *c->weapon;
            } else {
                item.type = TYPE_ARMOR;
                item.data.armor = *c->defense;
            }
            return item;
        } else if (c->weapon != NULL) {
            ObjectData item;
            item.type = TYPE_WEAPON;
            item.data.weapon = *c->weapon;
            return item;
        } else if (c->defense != NULL) {
            ObjectData item;
            item.type = TYPE_ARMOR;
            item.data.armor = *c->defense;
            return item;
        }
    }
    // Fallback: ítem del inventario, el otro 50% restante es para soltar un item del inventario
    // en caso de no tener simplemente no retorna nada
    if (c->inventory_count > 0) {
        int idx = (int)get_random(0, (float)(c->inventory_count - 1));
        return c->inventory[idx];
    }
    exit(1);
}

// ==========================================
// ESTADÍSTICAS DEL JUGADOR Y ENEMIGO
// ==========================================

void show_player_stats(Player* p) {
    Character* c = &p->base_char;
    printf("--- Estadísticas de %s ---\n", c->name);
    draw_progress_bar(c->health, c->hp_max, "HP");
    draw_progress_bar((float)c->xp_points, (float)c->xp_threshold, "XP");
    printf("Nivel: %d\n",              c->xp_level);
    printf("XP para subir de nivel: %d\n", c->xp_threshold);
    printf("Ataque: %d\n",             c->attack);
    if (c->defense != NULL) {
        printf("Defensa: %.0f%% reducción de daño físico\n", c->defense->resistance * 100.0f);
    } else {
        printf("Defensa: sin armadura\n");
    }
    if (c->weapon != NULL) {
        printf("Arma: %s (ataque total: %d pts)\n",
               c->weapon->base_item.name, calculate_attack(c));
    } else {
        printf("Arma: desarmado\n");
    }
    printf("Inventario: %d/%d\n", c->inventory_count, MAX_INVENTORY);
}

void show_enemy_stats(Enemy* p) {
    Character* c = &p->base_char;
    printf("--- Estadísticas de %s ---\n", c->name);
    draw_progress_bar(c->health, c->hp_max, "HP");
    printf("Rango: %c\n", p->rank);
    printf("Nivel: %d\n", c->xp_level);
    printf("Ataque: %d\n", c->attack);
    if (c->defense != NULL) {
        printf("Defensa: %.0f%% reducción de daño físico\n", c->defense->resistance * 100.0f);
    } else {
        printf("Defensa: sin armadura\n");
    }
    if (c->weapon != NULL) {
        printf("Arma: %s (ataque total: %d pts)\n",
               c->weapon->base_item.name, calculate_attack(c));
    } else {
        printf("Arma: desarmado\n");
    }
    printf("Inventario: %d/%d\n", c->inventory_count, MAX_INVENTORY);
}

// ==========================================
// COMBATE
// ==========================================

void combat(Player* p_player, Enemy* p_enemy) {
    Character* player_c = &p_player->base_char;
    Character* enemy_c  = &p_enemy->base_char;

    char first_user = (enemy_c->xp_level > player_c->xp_level) ? 'e' : 'p';

    printf("\n--- ¡Ha aparecido un %s! ---\n", enemy_c->name);
    bool can_escape = false;
    int  turn_counter = 1;

    do {
        if ((first_user == 'p' && turn_counter % 2 == 1) ||
            (first_user == 'e' && turn_counter % 2 == 0)) {
            // Turno del jugador
            const char* options[] = {"Atacar", "Abrir inventario", "Ver estadísticas del enemigo", "Escapar"};
            int selected_op = menu(options, 4);
            switch (selected_op) {
                case 1:
                    printf("%s ataca a %s\n", player_c->name, enemy_c->name);
                    take_damage(enemy_c, calculate_attack(player_c), DMG_PHYSICAL);
                    draw_progress_bar(enemy_c->health, enemy_c->hp_max, "HP");
                    enter_to_continue();
                    break;
                case 2:
                    char* action = open_inventory(player_c, true, enemy_c);
                    if (strcmp(action, "Salir") == 0) {
                        continue;
                    } else {
                        break;
                    }   
                case 3: {
                    show_enemy_stats(p_enemy);
                    enter_to_continue();
                    continue;
                }
                case 4: {
                    float esc_prob = escape_chance(player_c->attack, enemy_c->attack);
                    can_escape = (get_random(0, 1) <= esc_prob);
                    if (can_escape) {
                        printf("¡Intento de escape exitoso!\n");
                    } else {
                        printf("¡No se pudo escapar!\n");
                    }
                    enter_to_continue();
                    break;
                }
            }
        } else {
            // Turno del enemigo
            printf("%s ataca a %s\n", enemy_c->name, player_c->name);
            take_damage(player_c, calculate_attack(enemy_c), DMG_PHYSICAL);
            draw_progress_bar(player_c->health, player_c->hp_max, "HP");
            enter_to_continue();
        }

        if (player_c->health <= 0) {
            printf("%s ha muerto\n", player_c->name);
            return;
        }
        if (enemy_c->health <= 0) {
            printf("%s ha muerto\n", enemy_c->name);
            ObjectData dropped = drop_random_item(enemy_c);
            char* dropped_name = get_obj_name(&dropped);
            add_item(player_c, &dropped, dropped_name);
            free(dropped_name);
            return;
        }
        turn_counter++;
    } while (player_c->health > 0 && enemy_c->health > 0 && !can_escape);
}


#endif // UTILS_H