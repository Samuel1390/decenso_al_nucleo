#ifndef UTILS_H
#define UTILS_H
#include "types.h"
#include "constants.h"
#include <math.h>

float min(float a, float b) {
    return (a < b) ? a : b;
}

float max(float a, float b) {
    return (a > b) ? a : b;
}

void enter_to_continue() {
    printf("[Enter] para continuar...\n");
    while (getchar() != '\n'); // Limpia el buffer hasta el enter
}

// Validación robusta de entrada de enteros
int get_int(const char* prompt) {
    char buffer[256];
    int value;
    
    while (true) {
        printf("%s", prompt);
        if (fgets(buffer, sizeof(buffer), stdin) != NULL) {
            // Intentamos parsear el input como un entero
            if (sscanf(buffer, "%d", &value) == 1) {
                return value;
            }
        }
        printf("Entrada inválida. Por favor, ingresa un número válido.\n");
    }
}
int menu(const char* options[], int num_options) {
    printf("\n\nEscribe el número de la opción para seleccionarla:\n");
    for (int i = 0; i < num_options; i++) {
        printf("%d. %s\n", i + 1, options[i]);
    }
    
    int selected;
    while (true) {
        selected = get_int("> ");
        if (selected >= 1 && selected <= num_options) {
            return selected; // Retorna 1-indexed
        }
        printf("Opción fuera de rango. Selecciona un número entre 1 y %d.\n", num_options);
    }
}

float get_random(float min, float max) {
    float scale = (float)rand() / (float)RAND_MAX;
    return min + scale * (max - min);
}

// --- FUNCIONES DE PERSONAJE (CARACTER) ---

void init_character(Character* c, const char* name, int attack, int health, int hp_max, Armor* defense,
    int xp_level, int xp_threshold, int xp_points, Weapon* weapon) {
    strncpy(c->name, name, MAX_STRING);
    c->attack = attack;
    c->hp_max = hp_max;
    c->defense = defense;
    c->xp_level = xp_level;
    c->xp_threshold = xp_threshold;
    c->xp_points = xp_points;
    c->weapon = weapon;
    
    if (health <= hp_max && health > 0) {
        c->health = health;
    } else {
        printf("Error: Salud inválida. Forzando a 1.\n");
        c->health = 1;
    }
}


int calculate_attack(Character* c) {
    if (c->weapon != NULL) {
        // El arma amplifica el ataque. Si es muy debil ataca con daño base.
        int w_damage = sqrtf((c->weapon->damage * c->attack));
        return (c->attack > w_damage) ? c->attack : w_damage;
    }
    return c->attack;
}

void take_damage(Character* c, int damage, DamageType type) {
    int damage_received = 0;
    if (type == DMG_PHYSICAL) {
        damage_received = damage * c->defense->resistance; // Adaptación simple de la resistencia
    } else if (type == DMG_MAGICAL) {
        damage_received = damage;
    }
    
    if (damage_received < 0) damage_received = 0; // Evita curarse con ataques débiles
    
    c->health -= damage_received;
    if (c->health <= 0) {
        c->health = 0;
        printf("%s ha muerto.\n", c->name);
    }
}

void level_up(Character* c, int xp_points) {
    int xp_remaining = xp_points + c->xp_points;
    while (xp_remaining >= c->xp_threshold) {
        xp_remaining -= c->xp_threshold;
        c->attack += c->attack / 10;       // +10%
        c->hp_max += c->hp_max / 10;       // +10%
        c->xp_threshold += c->xp_threshold / 10; // +10%
        c->xp_level += 1;
        printf("¡%s subió de nivel!\n", c->name);
    }
    c->xp_points = xp_remaining;
}

void draw_progress_bar(float current_value, float max_value, char label[]) {
    int norm_current_val = (int)(current_value / max_value * 50); // 50 barras totales para que no quede tan largo
    printf("[");
    for (int i = 0; i < norm_current_val; i++) {
        printf("#");
    }
    for (int i = norm_current_val; i < 50; i++) {
        printf("-");
    }
    printf("] %s %d/%d\n", label, (int)current_value, (int)max_value);

}

float escape_chance(float player_attack, float enemy_attack) {
    int random_factor = get_random(1, 3);
    int escape = ((player_attack * 51.2 / enemy_attack) + 12.0 * random_factor);

    if (escape > 100) escape = 100;
    return (float)escape / 100.0;
}

// --- FUNCIONES DE INVENTARIO Y JUGADOR ---

// Elimina un objeto del inventario desplazando el arreglo
void remove_item(Player* p, int index) {
    for (int i = index; i < p->inventory_count - 1; i++) {
        p->inventory[i] = p->inventory[i + 1];
    }
    p->inventory_count--;
}

int use_item(Character* character, Item* item) {
    
    
}

void open_inventory(Player* p, bool in_battle, Character* enemy) {
    if (p->inventory_count == 0) {
        printf("\nEl inventario está vacío.\n");
        enter_to_continue();
        return;
    }

    // Crear arreglo dinámico de opciones de texto
    const char* inv_ops[MAX_INVENTORY + 1];
    for (int i = 0; i < p->inventory_count; i++) {
        inv_ops[i] = p->inventory[i].name;
    }
    inv_ops[p->inventory_count] = "Salir";

    int item_id = menu(inv_ops, p->inventory_count + 1);
    if (item_id == p->inventory_count + 1) {
        return; // Salió del inventario
    }

    int selected_index = item_id - 1;
    Item* selected_item = &p->inventory[selected_index];

    printf("\n--- %s ---\n", selected_item->name);
    printf("%s\n", selected_item->description);
    printf("Cantidad: %d\n", selected_item->quantity);

    if (in_battle && enemy != NULL) {
        const char* battle_ops[] = {"Usar", "Volver"};
        int accion = menu(battle_ops, 2);
        
        if (accion == 1) {
            // USO DE PUNTEROS PARA AFECTAR ENTIDADES
            if (selected_item->target_type == TARGET_ENEMY) {
                if(selected_item->use_function) selected_item->use_function(enemy, selected_item);
            } else if (selected_item->target_type == TARGET_PLAYER) {
                if(selected_item->use_function) selected_item->use_function(&p->base_char, selected_item);
            }
            
            selected_item->quantity--;
            if(selected_item->quantity <= 0) remove_item(p, selected_index);
            return; // Después de usar en batalla suele terminar el turno
            
        } else if (accion == 2) {
            open_inventory(p, in_battle, enemy); // Llamada recursiva (volver)
        }
    } else {
        const char* peace_ops[] = {"Usar", "Soltar", "Volver"};
        int accion = menu(peace_ops, 3);

        if (accion == 1) {
            if (selected_item->target_type == TARGET_PLAYER && selected_item->can_use_outside_battle) {
                if(selected_item->use_function) selected_item->use_function(&p->base_char, selected_item);
                
                selected_item->quantity--;
                if(selected_item->quantity <= 0) remove_item(p, selected_index);
                
            } else {
                printf("\nNo puedes usar este objeto fuera de combate.\n");
                enter_to_continue();
                open_inventory(p, in_battle, enemy);
            }
        } 
        else if (accion == 2) {
            // VALIDACIÓN MEJORADA AL SOLTAR OBJETOS
            int amount_to_drop = 1;
            if (selected_item->quantity > 1) {
                printf("\nTienes %d de este objeto.\n", selected_item->quantity);
                amount_to_drop = get_int("Cantidad a soltar: ");
                
                // Evitamos números negativos o cero
                if (amount_to_drop <= 0) {
                    printf("Cancelado.\n");
                    open_inventory(p, in_battle, enemy);
                    return;
                }
                
                // Si intenta soltar más de los que tiene, soltamos el máximo
                if (amount_to_drop > selected_item->quantity) {
                    amount_to_drop = selected_item->quantity;
                }
            }

            selected_item->quantity -= amount_to_drop;
            printf("\nHas soltado %s (x%d)\n", selected_item->name, amount_to_drop);
            
            if (selected_item->quantity <= 0) {
                remove_item(p, selected_index);
            }
            
            enter_to_continue();
            open_inventory(p, in_battle, enemy);
        }
        else if (accion == 3) {
            open_inventory(p, in_battle, enemy);
        }
    }
}

int add_item(Player* p, ObjectData* obj, char* obj_name) {
    if (p->inventory_count < MAX_INVENTORY) {
        p->inventory[p->inventory_count] = *obj;
        p->inventory_count++;
        printf("%s ha obtenido %s\n", p->base_char.name, obj_name);
        return 1;
    } else {
        printf("Inventario lleno\n");
        return 0;
    }
}

int get_idx_from_id(Player* p, int id) {
    for (int i = 0, i < p->inventory_count, i++) {
        switch(p->inventory[i].type) {
            case TYPE_CONSUMABLE:
                if (p->inventory[i].data.item.id == id) {
                    return i;
                }
                break;
            case TYPE_WEAPON:
                if (p->inventory[i].data.weapon.base_item.id == id) {
                    return i;
                }
                break;
            case TYPE_ARMOR:
                if (p->inventory[i].data.armor.base_item.id == id) {
                    return i;
                }
                break;
        }
    }
    return -1;
}

void update_inventory(Player* p) {
    for (int i = 0, i < p->inventory_count, i++) {
        switch (p->inventory[i].type) {
            case TYPE_CONSUMABLE:
                if (p->inventory[i].data.item.quantity <= 0) {
                    remove_item(p, i);
                }
                break;
            case TYPE_WEAPON:
                if (p->inventory[i].data.weapon.durability <= 0 || p->inventory[i].data.weapon.quantity <= 0) {
                    remove_item(p, i);
                }
                break;
            case TYPE_ARMOR:
                if (p->inventory[i].data.armor.durability <= 0 || p->inventory[i].data.armor.quantity <= 0) {
                    remove_item(p, i);
                }
                break;
        }
    }
    
}

void combat(Player* p_player, Character* p_enemy) {
    int turn_counter = 1;
    char first_user = 'e';
    Character* first_turn = NULL;
    if (p_enemy->xp_level > p_player->base_char.xp_level) {
        first_turn = p_enemy;
        first_user = 'e';
    } else {
        first_turn = &p_player->base_char;
        first_user = 'p';
    }
    printf("\n--- ¡Ha aparecido un %s!---\n", p_enemy->name);
    bool can_scape = false;
    do {
        if ((first_user == 'p' && turn_counter % 2 == 1) || (first_user == 'e' && turn_counter % 2 == 0)) {
            const char* options[] = {"Atacar", "Abrir inventario", "Escapar"};
            int selected_op = menu(options, 3);
            switch (selected_op) {
                case 1:
                    printf("%s ataca a %s\n", p_player->base_char.name, p_enemy->name);
                    take_damage(p_enemy, calculate_attack(&p_player->base_char), DMG_PHYSICAL);
                    draw_progress_bar(p_enemy->health, p_enemy->hp_max, "HP");
                    enter_to_continue();
                    break;
                case 2:
                    open_inventory(p_player, true, p_enemy);
                    break;
                case 3:
                    float esc_prob = escape_chance(p_player->base_char.attack, p_enemy->attack);
                    // si el número aleatorio es menor o igual a la probabilidad de escape, el jugador escapa
                    can_scape = get_random(0, 1) <= esc_prob ? true : false; 
                    if (can_scape) {
                        printf("¡Intento de escape exitoso!\n");
                        enter_to_continue();
                        break;
                    } else {
                        printf("¡No se pudo escapar!\n");
                        enter_to_continue();
                    }
            }
            
        } else {
            printf("%s ataca a %s\n", p_enemy->name, p_player->base_char.name);
            take_damage(&p_player->base_char, calculate_attack(p_enemy), DMG_PHYSICAL);
            draw_progress_bar(p_player->base_char.health, p_player->base_char.hp_max, "HP");
            enter_to_continue();
        }
        if (p_player->base_char.health <= 0) {
            printf("%s ha muerto\n", p_player->base_char.name);
            return;
        }
        if (p_enemy->health <= 0) {
            printf("%s ha muerto\n", p_enemy->name);
            ObjectData dropped_item = drop_random_item(p_enemy);
            add_item(p_player, &dropped_item, get_obj_name(&dropped_item));
            return;
        }
        turn_counter++;
    } while ((p_player->base_char.health > 0 && p_enemy->health > 0) && !can_scape);

};

char* get_obj_name(ObjectData* obj) {
    switch(obj->type) {
        case(TYPE_CONSUMABLE): return obj->data.item.name;
        case(TYPE_WEAPON): return obj->data.weapon.base_item.name;
        case(TYPE_ARMOR): return obj->data.armor.base_item.name;
    }
}


void show_player_stats(Player* p) {
    printf("--- Estadísticas de %s ---\n", p->base_char.name);
    draw_progress_bar(p->base_char.health, p->base_char.hp_max, "HP");
    draw_progress_bar(p->base_char.xp_points, p->base_char.xp_threshold, "XP");
    printf("Nivel: %d\n", p->base_char.xp_level);
    printf("XP para subir de nivel: %d\n", p->base_char.xp_threshold);
    printf("Ataque: %d\n", p->base_char.attack);
    if (p->base_char.defense != NULL) {
        printf("Defensa: %.2f%%\n", p->base_char.defense->resistance * 100);
    } else {
        printf("Defensa: sin armadura\n");
    }
    if (p->base_char.weapon != NULL) {
        printf("Arma: %s (daño: +%d pts)\n", p->base_char.weapon->name, (int)calculate_attack(&p->base_char) - p->base_char.attack);
    } else {
        printf("Arma: desarmado\n");
    }
}

ObjectData drop_random_item(Enemy* e) {
    float random_num = get_random(0, 1);
    if (random_num > 0.5) {
        return (random_num > 0.75) ? e->base_char.weapon : e->base_char.defense;
    } else {
        int random_idx = get_random(0, e->inventory_count - 1);
        return e->inventory[random_idx];
    }
}

#endif UTILS_H