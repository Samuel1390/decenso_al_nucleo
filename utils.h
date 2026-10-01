#ifndef UTILS_H
#define UTILS_H

#include "types.h"
#include "constants.h"
#include "getters.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// UTILIDADES GENERALES

float get_random(float lo, float hi) {
    float scale = (float)rand() / (float)RAND_MAX;
    return lo + scale * (hi - lo);
}

float min_f(float a, float b) { return (a < b) ? a : b; }
float max_f(float a, float b) { return (a > b) ? a : b; }
int min_int(int a, int b) { return (a < b) ? a : b; }
int max_int(int a, int b) { return (a > b) ? a : b; }

void enter_to_continue() {
    printf("[Enter] para continuar...\n");
    int ch;
    while ((ch = getchar()) != '\n' && ch != EOF);
}

char** split(char* str, char* delim) {
    int n = 100;
    char **arr = (char **)malloc(n * sizeof(char *));

    if (arr == NULL) {
        printf("Error al asignar memoria\n");
        abort();
    }
    int i = 0;
    char *token = strtok(str, delim);
    while(token != NULL && i < n - 1) {
        arr[i] = token;
        token = strtok(NULL, delim);
        i++;
    }
    arr[i] = NULL;
    return arr;
}

char* join(char** arr, char* delim) {
    if (arr == NULL || arr[0] == NULL) {
        char* empty = (char*)malloc(1 * sizeof(char));
        if (empty != NULL) empty[0] = '\0';
        return empty;
    }

    int total_length = 0;
    int delim_length = strlen(delim);
    int i = 0;

    while (arr[i] != NULL) {
        total_length += strlen(arr[i]);
        i++;
    }
    int num_words = i;
    total_length += (num_words - 1) * delim_length + 1;
    
    char* result = (char*)malloc(total_length * sizeof(char));
    if (result == NULL) {
        printf("Error al asignar memoria en join\n");
        return NULL;
    }

    // 4. Construir la cadena uniendo los elementos
    result[0] = '\0'; // Inicializar la cadena vacía para usar strcat con seguridad
    for (i = 0; i < num_words; i++) {
        strcat(result, arr[i]);
        // Añadir el delimitador solo si no es el último elemento
        if (i < num_words - 1) {
            strcat(result, delim);
        }
    }

    return result;
}


// Validación robusta de entrada de enteros
int get_int(const char* prompt) {
    char buffer[256];
    int value;
    while (1) {
        printf("%s", prompt);
        if (fgets(buffer, sizeof(buffer), stdin) != NULL) {
            if (sscanf(buffer, "%d", &value) == 1) return value;
        } else {
            exit(0); // Salir limpiamente si se cierra la entrada (EOF)
        }
        printf("Entrada inválida. Por favor, ingresa un número válido.\n");
    }
}

// funciones para para manejar el dropeo de xp y aumento del umbral
float delta(float x) {
    // esta funcion la usamos para calcular los nuevos umbrales de xp
    return (float)pow(1.25, x - 4.0f) + 0.5f;
}

// la funcion delta_inverse es la inversa de delta, la usamos para calcular el nivel adecuado de xp que deberia dropear el enemigo
float delta_inverse(float x) {
    float argument = x - 0.5f;
    
    // Validación para evitar errores matemáticos (logaritmo de números <= 0)
    if (argument <= 0.0f) {
        return 0.0f; 
    }
    
    return (float)(log(argument) / log(1.25)) + 4.0f;
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
// INVENTARIO (operan sobre Character*)

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
    // pendiente, implementar la funcion para soltar items a voluntad del jugador para agregar uno nuevo
    return 0;
}

// Busca un objeto por ID; retorna su índice o -1 si no existe
int get_idx_from_id(Character* c, int id) {
    for (int i = 0; i < c->inventory_count; i++) {
        switch (c->inventory[i].type) {
            case TYPE_CONSUMABLE:
                if (c->inventory[i].data.item.global_id == id) return i;
                break;
            case TYPE_WEAPON:
                if (c->inventory[i].data.weapon.base_item.global_id == id) return i;
                break;
            case TYPE_ARMOR:
                if (c->inventory[i].data.armor.base_item.global_id == id) return i;
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

// --- Lógica de uso de ítems genéricos (consumibles) ---
void use_generic_item(const struct Item* item, struct Character* user, struct Character* target) {
    float pct = (float)item->function / 100.0f;
    if (user != NULL) {
        user->stats.cont_items_used++;
    }
    if (item->target_type == TARGET_PLAYER) {
        int amount = (int)(target->hp_max * pct);
        if (target->health + amount > target->hp_max) {
            amount = target->hp_max - target->health;
        }
        target->health += amount;
        target->stats.health_points_restored += amount;
        printf("%s ha recuperado %d puntos de salud\n", target->name, amount);
        draw_progress_bar(target->health, target->hp_max, "HP");
        enter_to_continue();
    } else if (item->target_type == TARGET_ENEMY) {
        int damage = (int)(target->hp_max * pct);
        if (damage > target->health) {
            damage = target->health;
        }
        target->health -= damage;
        if (user != NULL) {
            user->stats.damage_dealt += damage;
        }
        printf("%s ha recibido %d puntos de daño por %s\n", target->name, damage, item->name);
        draw_progress_bar(target->health, target->hp_max, "HP");
        enter_to_continue();
    }
}

// En utils.h
void use_obj_fn(ObjectData* obj, Character* user, Character* target) {
    ItemAction fn = NULL;
    const Item* item_ptr = NULL;
    switch (obj->type) {
        case TYPE_CONSUMABLE: 
            fn = obj->data.item.use_function; 
            item_ptr = &obj->data.item;
            break;
        case TYPE_WEAPON:     
            fn = obj->data.weapon.base_item.use_function; 
            item_ptr = &obj->data.weapon.base_item;
            break;
        case TYPE_ARMOR:      
            fn = obj->data.armor.base_item.use_function; 
            item_ptr = &obj->data.armor.base_item;
            break;
    }
    if (fn && item_ptr) {
        fn(item_ptr, user, target);
    }
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
        char str_quantity[4];
        snprintf(str_quantity, sizeof(str_quantity), " x%d", get_obj_quantity(&c->inventory[i]));
        inv_ops[i] = strcat(get_obj_name(&c->inventory[i]), str_quantity);
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
    // Determinar quién es el objetivo según la propiedad del ítem
    Character* target = (get_obj_target(&selected_item) == TARGET_ENEMY) ? enemy : c;

    // Pasar los 3 argumentos requeridos: objeto, usuario (jugador) y objetivo
    use_obj_fn(&selected_item, c, target);

    set_obj_quantity(&selected_item, get_obj_quantity(&selected_item) - 1);
    c->stats.cont_items_used += 1;

    // Propagar la cantidad modificada de vuelta al inventario real
    set_obj_quantity(&c->inventory[selected_index], get_obj_quantity(&selected_item));
    if (get_obj_quantity(&selected_item) <= 0) {
        remove_item(c, selected_index);
    }
    return "Usar";
    } else if (accion == 2) {
    return open_inventory(c, in_battle, enemy);
    }
        
    } else {
        const char* peace_ops[] = {"Usar", "Soltar", "Volver"};
        int accion = menu(peace_ops, 3);

        if (accion == 1) { // Pendiente, se debe cambiar por un switch
            if (get_obj_target(&selected_item) == TARGET_PLAYER &&
                get_obj_can_use_outside_battle(&selected_item)) {
                use_obj_fn(&selected_item, c, c);
                set_obj_quantity(&selected_item, get_obj_quantity(&selected_item) - 1);
                c->stats.cont_items_used += 1;
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
            // nueva cantidad,
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
    return "Salir";
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

void take_damage(Character* c, int damage) {
    int damage_received = 0;
    if (c->defense != NULL) {
        // resistance reduce el daño: damage * (1 - resistance)
        damage_received = (int)(damage * (1.0f - c->defense->resistance / 100));
    } else {
        damage_received = damage;  // Sin armadura, daño completo
    }
    if (damage_received < 0) damage_received = 1;

    c->health -= damage_received;
    if (c->health <= 0) {
        c->health = 0;
        printf("%s ha muerto.\n", c->name);
    }
}

void level_up(Character* c, int xp_gained, int print_notice) {
    int xp_remaining = xp_gained + c->xp_points;
    while (xp_remaining >= c->xp_threshold) {
        xp_remaining   -= c->xp_threshold;
        c->attack       += c->attack      / 10;
        c->hp_max       += c->hp_max      / 10;
        c->xp_threshold += (int)(delta((float)c->xp_level) * BASE_XP_THRESHOLD); // ya que xp_level es > 1 el numero siempre resultante siempre es mayor a BASE_XP_THRESHOLD
        c->xp_level     += 1;
        if (print_notice) {
            printf("¡%s subió a nivel %d!\n", c->name, c->xp_level);
        }
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

// Intenta soltar un objeto del personaje. Retorna true si soltó un objeto, false en caso contrario.
bool drop_random_item(Character* c, ObjectData* out_item, char rank) {
    bool has_equipment = (c->weapon != NULL || c->defense != NULL);
    bool has_inventory = (c->inventory_count > 0);
    // para rangos comunes la probabilidad de dropear items es de un 60%
    if (rank == 'D' || (rank == 'C' && get_random(0,1) < 0.4f)) {
        return false; 
    }

    if (!has_equipment && !has_inventory) {
        return false;
    }
    float random_num = get_random(0, 1);
    if (has_equipment && (random_num > 0.5f || !has_inventory)) {
        if (c->weapon != NULL && c->defense != NULL) {
            if (random_num > 0.75f) {
                out_item->type = TYPE_WEAPON;
                out_item->data.weapon = *c->weapon;
            } else {
                out_item->type = TYPE_ARMOR;
                out_item->data.armor = *c->defense;
            }
            return true;
        } else if (c->weapon != NULL) {
            out_item->type = TYPE_WEAPON;
            out_item->data.weapon = *c->weapon;
            return true;
        } else if (c->defense != NULL) {
            out_item->type = TYPE_ARMOR;
            out_item->data.armor = *c->defense;
            return true;
        }
    }

    if (has_inventory) {
        int idx = (int)get_random(0, (float)(c->inventory_count - 1));
        *out_item = c->inventory[idx];
        return true;
    }

    return false;
}

// ==========================================
// ESTADÍSTICAS DEL JUGADOR Y ENEMIGO
// ==========================================

void fprintf_player_stats(Player* p, FILE* f) {
    Character* c = &p->base_char;
    fprintf(f, "--- Estadísticas de %s ---\n", c->name);
    draw_progress_bar(c->health, c->hp_max, "HP");
    draw_progress_bar((float)c->xp_points, (float)c->xp_threshold, "XP");
    fprintf(f, "Nivel: %d\n",  c->xp_level);
    fprintf(f, "XP para subir de nivel: %d\n", c->xp_threshold);
    fprintf(f, "Ataque: %d\n", c->attack);
    if (c->defense != NULL) {
        fprintf(f, "Defensa: %s\n", c->defense->base_item.name);
    } else {
        fprintf(f, "Defensa: sin armadura\n");
    }
    if (c->weapon != NULL) {
        fprintf(f, "Arma: %s\n",
               c->weapon->base_item.name);
    } else {
        fprintf(f, "Arma: desarmado\n");
    }
    fprintf(f, "Inventario: %d/%d\n", c->inventory_count, MAX_INVENTORY);
}
void fprintf_player_stats_formatted(Player* p, FILE* f) {
    Character* c = &p->base_char;
    draw_progress_bar(c->health, c->hp_max, "HP");
    draw_progress_bar((float)c->xp_points, (float)c->xp_threshold, "XP");
    fprintf(f, "level: %d\n",  c->xp_level);
    fprintf(f, "xp_threshold: %d\n", c->xp_threshold);
    fprintf(f, "attack: %d\n", c->attack);
    if (c->defense != NULL) {
        char* name_formatted = join(split(c->defense->base_item.name, " "), "_"); // para de Armadura de Hierro a Armadura_de_Hierro
        fprintf(f, "defense: %s %s %d %d %d %d\n", "TYPE_ARMOR",
          name_formatted,
          (int)(c->defense->resistance * 100), c->defense->durability,
          c->defense->base_item.file_id,
          c->defense->base_item.global_id);
        free(name_formatted);
    } else {
        fprintf(f, "defense: no_defense\n");
    }
    if (c->weapon != NULL) {
        char* name_formatted = join(split(c->weapon->base_item.name, " "), "_"); // para de Espada de Hierro a Espada_de_Hierro
        fprintf(f, "weapon: %s %s %d %d %d %d\n", "TYPE_WEAPON",
          name_formatted,
          (int)(c->weapon->damage * 100), c->weapon->durability,
          c->weapon->base_item.file_id,
          c->weapon->base_item.global_id);
        free(name_formatted);
    } else {
        fprintf(f, "weapon: no_weapon\n");
    }
    fprintf(f, "inventory_count: %d\n", c->inventory_count);
    fprintf(f, "max_inventory: %d\n", MAX_INVENTORY);
}
void show_player_stats(Player* p) {
    Character* c = &p->base_char;
    printf("--- Estadísticas de %s ---\n", c->name);
    draw_progress_bar(c->health, c->hp_max, "HP");
    draw_progress_bar((float)c->xp_points, (float)c->xp_threshold, "XP");
    printf("Nivel: %d\n",              c->xp_level);
    printf("XP para subir de nivel: %d\n", c->xp_threshold);
    printf("Ataque: %d\n",             c->attack);
    if (c->defense != NULL) {
        printf("Defensa: %s (%.0f%% reducción de daño físico)\n", c->defense->base_item.name, c->defense->resistance * 100.0f);
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
int drop_xp(Enemy* enemy, bool use_rank) {
    char rank = enemy->rank;
    int xp_level = enemy->base_char.xp_level;
    int xp_droped = 0;
    int threshold = (int)(delta_inverse((float)xp_level) * BASE_XP_THRESHOLD);
    xp_droped += threshold;
    if (use_rank) {
        switch(rank) {
            case 'D':
                xp_droped = (int)(xp_droped * 0.9);
                break;
            case 'C':
                xp_droped = (int)(xp_droped * 0.95);
                break;
            case 'B':
                xp_droped = (int)(xp_droped * 1.0);
                break;
            case 'A':
                xp_droped = (int)(xp_droped * 1.05);
                break;
            case 'S':
                xp_droped = (int)(xp_droped * 1.2);
                break;
            case 'R':
                xp_droped = (int)(xp_droped * 1.8);
                break;
            default:
                break;
        }
    }
    return xp_droped;
}
// COMBATE

int combat(Player* p_player, Enemy* p_enemy) { // -> returona el time to kill del jugador, si muere devuelve -1
    Character* player_c = &p_player->base_char;
    Character* enemy_c  = &p_enemy->base_char;
    int ttk = 0;

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
                    take_damage(enemy_c, calculate_attack(player_c));
                    draw_progress_bar(enemy_c->health, enemy_c->hp_max, "HP");
                    ttk += 1;
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
            take_damage(player_c, calculate_attack(enemy_c));
            draw_progress_bar(player_c->health, player_c->hp_max, "HP");
            enter_to_continue();
        }

        if (player_c->health <= 0) {
            printf("HAS MUERTO, fin del juego\n");
            enter_to_continue();
            return -1;
        }
        if (enemy_c->health <= 0) {
            printf("%s ha muerto\n", enemy_c->name);
            ObjectData dropped;
            if (drop_random_item(enemy_c, &dropped, p_enemy->rank)) {
                char* dropped_name = get_obj_name(&dropped);
                add_item(player_c, &dropped, dropped_name);
                // Si get_obj_name devuelve un puntero interno y no un malloc, no uses free() aquí
            }
            int xp_drop = drop_xp(p_enemy, p_player);
            printf("Haz ganado %d puntos de experiencia\n", xp_drop);
            enter_to_continue();
            level_up(player_c, xp_drop, 1);
            enter_to_continue();

            int max_hp = player_c->hp_max;
            int health = player_c->health;
            p_player->base_char.health = min_int(max_hp, health + (int)((float)max_hp * 0.4f));
            printf("Has recuperado el 40%%%% de tu vida\n");
            enter_to_continue();
            return ttk;
        }

        turn_counter++;
    } while (player_c->health > 0 && enemy_c->health > 0 && !can_escape);
}



#endif // UTILS_H