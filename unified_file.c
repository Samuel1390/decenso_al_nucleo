#define MAX_ENEMY_INVENTORY 5
#define MAX_INVENTORY 32
#define MAX_STRING 100
#define MAX_STACK_AMOUNT 16
#define BASE_XP_THRESHOLD 100
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include <math.h>
#include <time.h>
#include <assert.h>
// ***************************************
// TYPES
// ***************************************

// 1. ENUMERACIONES

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

// 2. DECLARACIONES ADELANTADAS (Forward Declarations)

typedef struct Character Character;
typedef struct Item      Item;
typedef struct ObjectData ObjectData;  // Forward declaration necesaria para Character
// 3. PUNTEROS A FUNCIÓN
typedef void (*ItemAction)(const struct Item* item, struct Character* user, struct Character* target);
// 4. ESTRUCTURAS PRINCIPALES
struct Item {
    ItemType  type;
    char      name[MAX_STRING];
    char      description[MAX_STRING + 200];
    int function;
    int       quantity;
    bool      can_use_outside_battle;
    TargetType target_type;
    int       file_id;
    int       global_id;
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

struct ObjectData {
    ItemType type;
    char rank;
    union {
        Item   item;
        Weapon weapon;
        Armor  armor;
    } data;
};
typedef struct {
    char name[MAX_STRING];
    int damage_dealt;
    int health_points_restored;
    int cont_kills;
    int cont_items_used;
    int rooms_visited;
} Stats;
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
    int file_id;
    int global_id;
    Stats stats;
};

// 5. TIPOS COMPUESTOS
typedef struct {
    Character base_char;
} Player;

typedef struct {
    Character base_char;
    char rank;  // rareza del enemigo: 'S' > 'A' > 'B' > 'C' > 'D'
} Enemy;

// ***************************************
// GETTERS
// ***************************************

// Retorna una copia alocada del nombre del objeto (el llamador debe free())
char* get_obj_name(ObjectData* obj) {
    char* name = (char*)malloc(MAX_STRING * sizeof(char));
    if (name == NULL) { perror("malloc"); return NULL; }
    switch (obj->type) {
        case TYPE_CONSUMABLE: strncpy(name, obj->data.item.name,              MAX_STRING - 1); break;
        case TYPE_WEAPON:     strncpy(name, obj->data.weapon.base_item.name,  MAX_STRING - 1); break;
        case TYPE_ARMOR:      strncpy(name, obj->data.armor.base_item.name,   MAX_STRING - 1); break;
        default:              strncpy(name, "Anónimo",                        MAX_STRING - 1); break;
    }
    name[MAX_STRING - 1] = '\0';
    return name;
}

// Retorna una copia alocada de la descripción (el llamador debe free())
char* get_obj_desc(ObjectData* obj) {
    int   desc_size = MAX_STRING + 200;
    char* desc = (char*)malloc(desc_size * sizeof(char));
    if (desc == NULL) { perror("malloc"); return NULL; }
    switch (obj->type) {
        case TYPE_CONSUMABLE: strncpy(desc, obj->data.item.description,              desc_size - 1); break;
        case TYPE_WEAPON:     strncpy(desc, obj->data.weapon.base_item.description,  desc_size - 1); break;
        case TYPE_ARMOR:      strncpy(desc, obj->data.armor.base_item.description,   desc_size - 1); break;
        default:              strncpy(desc, "Sin descripción",                       desc_size - 1); break;
    }
    desc[desc_size - 1] = '\0';
    return desc;
}

int get_obj_quantity(ObjectData* obj) {
    switch (obj->type) {
        case TYPE_CONSUMABLE: return obj->data.item.quantity;
        case TYPE_WEAPON:     return obj->data.weapon.base_item.quantity;
        case TYPE_ARMOR:      return obj->data.armor.base_item.quantity;
    }
    return 0;
}

int get_obj_global_id(ObjectData* obj) {
    switch (obj->type) {
        case TYPE_CONSUMABLE: return obj->data.item.global_id;
        case TYPE_WEAPON:     return obj->data.weapon.base_item.global_id;
        case TYPE_ARMOR:      return obj->data.armor.base_item.global_id;
    }
    return -1;
}

TargetType get_obj_target(ObjectData* obj) {
    switch (obj->type) {
        case TYPE_CONSUMABLE: return obj->data.item.target_type;
        case TYPE_WEAPON:     return obj->data.weapon.base_item.target_type;
        case TYPE_ARMOR:      return obj->data.armor.base_item.target_type;
    }
    return TARGET_PLAYER;
}

bool get_obj_can_use_outside_battle(ObjectData* obj) {
    switch (obj->type) {
        case TYPE_CONSUMABLE: return obj->data.item.can_use_outside_battle;
        case TYPE_WEAPON:     return obj->data.weapon.base_item.can_use_outside_battle;
        case TYPE_ARMOR:      return obj->data.armor.base_item.can_use_outside_battle;
    }
    return false;
}

// ***************************************
// UTILIDADES GENERALES
// ***************************************

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
//************************* */
// ENEMIES
//************************* */
void normalize_enemy(Player *player, Enemy *enemy, int mu, int sigma) {
  // Esta funcion equilibra al enemigo en un nivel al rededor de mu + player_lv con una desviacion estandar de sigma
  // Comunmente usamos mu para referirnos a la media y sigma para referirnos a la desviacion estandar
  int player_level = player->base_char.xp_level;

  int new_level = (int)get_random(max_int(player_level + mu - sigma, 1), player_level + mu + sigma);
  enemy->base_char.xp_level = new_level;
  
  int original_xp = enemy->base_char.xp_points;
  int xp_points = drop_xp(enemy, player);
  level_up(&enemy->base_char, xp_points, 0);
  enemy->base_char.xp_points = original_xp; // Restaurar el XP base para que al morir dé XP al jugador
}

Enemy create_enemy(char path[], int id, int global_id, Player *player) {
  Enemy enemy;
  bool found = false;
  char line[MAX_STRING];
  FILE* file = fopen(path, "r");
  if (file == NULL) {
    printf("Error al abrir el archivo");
    abort();
  }
  int file_id;
  char name[MAX_STRING];
  int attack;
  int health;
  int hp_max;
  int xp_level;
  int xp_threshold;
  int xp_points;

  Armor* armor = NULL;
  Weapon* weapon = NULL;
  char rank;
  while(fgets(line, sizeof(line), file) != NULL) {
    if (sscanf(line, "%d", &file_id) == 1 && file_id == id) {
      if (sscanf(line, "%d %s %d %d %d %d %d %d %c", &file_id, name, &attack, &health, &hp_max, &xp_level, &xp_threshold, &xp_points, &rank) == 9) {
        // Ojo pendiente hay que modificar el bestiario.txt ya que no se tomara en cuenta el nivel de experiencia sino que sera uno por eso el xp_level / xp_level
        init_character(&enemy.base_char, name, attack, health, hp_max, armor, xp_level / xp_level, xp_threshold, xp_points, weapon);
        enemy.rank = rank;
        enemy.base_char.global_id = global_id;
        normalize_enemy(player, &enemy, 0, 0);
        found = true;
      }
    }
  }
  fclose(file);
  if (found) {
    return enemy;
  } else {
    printf("No se encontro el enemigo con id: %d\n", id);
    abort();
  }
}

ObjectData create_random_item(char rank, int global_id) {
  char path[MAX_STRING];
  FILE* file;
  char line[MAX_STRING];
  ItemType type;
  float random_f = get_random(0,1);
  if (random_f >= 0.5) {
    type = TYPE_CONSUMABLE;
    strcpy(path, "./items.txt");
  } else if (random_f >= 0.25) {
    type = TYPE_WEAPON;
    strcpy(path, "./weapons.txt");
  } else  {
    type = TYPE_ARMOR;
    strcpy(path, "./armors.txt");
  }
  file = fopen(path, "r");
  if (file == NULL) {
    printf("Error al abrir archivo en la ruta %s", path);
    abort();
  }
  int ids[40];
  int ids_idx = 0;

  switch(type) {
    case TYPE_CONSUMABLE: {
      char item_type[MAX_STRING];
      char name[MAX_STRING];
      int function;
      int quantity;
      int can_use_outside_battle;
      char target[MAX_STRING];
      char item_rank;
      int id = id;
      // 1. filtrar los objetos que no pertenecen al rango espeficifado
      while (fgets(line, sizeof(line), file) != NULL) {
        if (sscanf(line, "%s %s %d %d %d %s %c %d", item_type, name, &function, &quantity, &can_use_outside_battle, target, &item_rank, &id) == 8) {
          if (item_rank == rank) {
            ids[ids_idx] = id;
            ids_idx += 1;
          }
        }
      }
      fclose(file);
      file = fopen(path, "r");
      //obtenemos un id aleatorio
      int random_id = ids[(int)get_random(0, ids_idx)];
      //buscamos el objeto con el id aleatorio
      while (fgets(line, sizeof(line), file) != NULL) {
        if (sscanf(line, "%s %s %d %d %d %s %c %d", item_type, name, &function, &quantity, &can_use_outside_battle, target, &item_rank, &id) == 8) {
          if (id == random_id) {
            ObjectData item;
            item.type = TYPE_CONSUMABLE;
            item.data.item.type = TYPE_CONSUMABLE;
            item.data.item.file_id = id;
            item.data.item.global_id = global_id;
            strncpy(item.data.item.name, name, MAX_STRING - 1);
            item.data.item.name[MAX_STRING - 1] = '\0';
            strncpy(item.data.item.description, "description", MAX_STRING + 199);
            item.data.item.quantity = quantity;
            item.data.item.can_use_outside_battle = can_use_outside_battle;
            item.data.item.target_type = strcmp(target, "TARGET_ENEMY") == 0 ? TARGET_ENEMY : TARGET_PLAYER;
            // pendiente, crear funciones para cada tipo de pocion
            // item.data.item.use_function = function;
            item.rank = item_rank;
            return item;
          }
        }
      }
    }
    case TYPE_WEAPON: {
      char item_type[MAX_STRING];
      char name[MAX_STRING];
      int damage;
      int durability;
      char item_rank;
      int id = id;
      // 1. filtrar los objetos que no pertenecen al rango espeficifado
      while (fgets(line, sizeof(line), file) != NULL) {
        if (sscanf(line, "%s %s %d %d %c %d", item_type, name, &damage, &durability, &item_rank, &id) == 6) {
          if (item_rank == rank) {
            ids[ids_idx] = id;
            ids_idx += 1;
          }
        }
      }
      fclose(file);
      file = fopen(path, "r");
      //obtenemos un id aleatorio
      int random_id = ids[(int)get_random(0, ids_idx)];
      while(fgets(line, sizeof(line), file) != NULL) {
        if (sscanf(line, "%s %s %d %d %c %d", item_type, name, &damage, &durability, &item_rank, &id) == 6) {
          if (id == random_id) {
            ObjectData item;
            item.type = TYPE_WEAPON;
            item.data.weapon.base_item.type = TYPE_WEAPON;
            item.data.weapon.base_item.file_id = id;
            item.data.weapon.base_item.global_id = global_id;
            strncpy(item.data.weapon.base_item.name, name, MAX_STRING - 1);
            item.data.weapon.base_item.name[MAX_STRING - 1] = '\0';
            char description[MAX_STRING + 199];
            sprintf(description, "%s aumenta el daño base en un %d%%", name, damage);
            strncpy(item.data.weapon.base_item.description, description, MAX_STRING + 199);
            item.data.weapon.base_item.quantity = 1;
            item.data.weapon.base_item.can_use_outside_battle = false;
            item.data.weapon.base_item.target_type = TARGET_ENEMY;
            // item.data.weapon.base_item.use_function = equip_iron_sword;
            item.data.weapon.damage = damage;
            item.data.weapon.durability = durability;
            item.rank = item_rank;
            return item;
          }
        }
      }
    }
    case TYPE_ARMOR: {
      char item_type[MAX_STRING];
      char name[MAX_STRING];
      int resistance;
      int durability;
      char item_rank;
      int id = id;
      // 1. filtrar los objetos que no pertenecen al rango espeficifado
      while (fgets(line, sizeof(line), file) != NULL) {
        if (sscanf(line, "%s %s %d %d %c %d", item_type, name, &resistance, &durability, &item_rank, &id) == 6) {
          if (item_rank == rank) {
            ids[ids_idx] = id;
            ids_idx += 1;
          }
        }
      }
      fclose(file);
      file = fopen(path, "r");
      //obtenemos un id aleatorio
      int random_id = ids[(int)get_random(0, ids_idx)];
      while(fgets(line, sizeof(line), file) != NULL) {
        if (sscanf(line, "%s %s %d %d %c %d", item_type, name, &resistance, &durability, &item_rank, &id) == 6) {
          if (id == random_id) {
            ObjectData item;
            item.type = TYPE_ARMOR;
            item.data.armor.base_item.type = TYPE_ARMOR;
            item.data.armor.base_item.file_id = id;
            item.data.armor.base_item.global_id = global_id;
            strncpy(item.data.armor.base_item.name, name, MAX_STRING - 1);
            item.data.armor.base_item.name[MAX_STRING - 1] = '\0';
            char description[MAX_STRING + 199];
            sprintf(description, "???");
            strncpy(item.data.armor.base_item.description, description, MAX_STRING + 199);
            item.data.armor.base_item.quantity = 1;
            item.data.armor.base_item.can_use_outside_battle = false;
            item.data.armor.base_item.target_type = TARGET_PLAYER;
            // item.data.armor.base_item.use_function = equip_iron_armor;
            item.data.armor.resistance = resistance;
            item.data.armor.durability = durability;
            item.rank = item_rank;
            return item;
          }
        }
      }
    }
  }
}

Enemy create_random_enemy(char path[], char rank, int global_id, Player *player, int enemy_lv_media, int enemy_lv_std) {
  // Esta funcion creara un enemigo con una media y desviacion estandar en su nivel
  // Comunmente usamos mu para referirnos a la media y sigma para referirnos a la desviacion estandar
  // pero en este caso usaremos enemy_lv_media y enemy_lv_std para no confundirnos con el nombre de las variables
  Enemy enemy;
  bool found = false;
  char line[MAX_STRING];
  FILE* file = fopen(path, "r");
  if (file == NULL) {
    printf("Error al abrir el archivo");
    abort();
  }
  int file_id;
  char name[MAX_STRING];
  int attack;
  int health;
  int hp_max;
  int xp_level;
  int xp_threshold;
  int xp_points;

  Armor* armor = NULL;
  Weapon* weapon = NULL;
  int random_id = 1;
  switch(rank) {
    case 'D':
      random_id = (int)get_random(1, 4);
      break;
    case 'C':
      random_id = (int)get_random(5, 11);
      break;
    case 'B':
      random_id = (int)get_random(12, 17);
      break;
    case 'A':
      random_id = (int)get_random(18, 24);
      break;
    case 'S':
      random_id = (int)get_random(25, 26);
      break;
    case 'R':
      random_id = (int)get_random(27, 31);
      break;
    default:
      printf("Error: Rango de enemigo invalido\n");
      abort();
      break;
  }
  while(fgets(line, sizeof(line), file) != NULL) {
    if (sscanf(line, "%d", &file_id) == 1 && file_id == random_id) {
      if (sscanf(line, "%d %s %d %d %d %d %d %d %c", &file_id, name, &attack, &health, &hp_max, &xp_level, &xp_threshold, &xp_points, &rank) == 9) {
        // Ojo pendiente hay que modificar el bestiario.txt ya que no se tomara en cuenta el nivel de experiencia sino que sera uno por eso el xp_level / xp_level
        init_character(&enemy.base_char, name, attack, health, hp_max, armor, xp_level / xp_level, 100, xp_points, weapon);
        enemy.rank = rank;
        enemy.base_char.file_id = file_id;
        enemy.base_char.global_id = global_id;
        ObjectData item1 = create_random_item(rank, global_id + 1);
        ObjectData item2 = create_random_item(rank, global_id + 2);
        add_item(&enemy.base_char, &item1, item1.data.item.name);
        add_item(&enemy.base_char, &item2, item2.data.item.name);
        normalize_enemy(player, &enemy, enemy_lv_media, enemy_lv_std);
        found = true;
      }
    }
  }
  fclose(file);
  if (found) {
    return enemy;
  } else {
    printf("No se encontro el enemigo con id: %d\n", random_id);
    abort();
  }
}

//******************************* */
// DUNGEON
//******************************* */


int global_id_counter = 1 * 512;

void tokenize_items(FILE* match, Player* player) {
  for (int i=0; i < player->base_char.inventory_count; i++) {
    ObjectData obj = player->base_char.inventory[i];
    switch(obj.type) {
      case TYPE_CONSUMABLE: {
        char* target_type = obj.data.item.target_type == TARGET_PLAYER ? "TARGET_PLAYER" : "TARGET_ENEMY";
        fprintf(match, "%s %s %d %d %d %s %d %d\n", "TYPE_CONSUMABLE",
          obj.data.item.name,
          obj.data.item.function,
          obj.data.item.quantity, obj.data.item.can_use_outside_battle,
          target_type,
          obj.data.item.file_id,
          obj.data.item.global_id
        );
        break;
      }
      case TYPE_WEAPON: {
        fprintf(match, "%s %s %d %d %d %d\n", "TYPE_WEAPON",
          obj.data.weapon.base_item.name,
          (int)(obj.data.weapon.damage * 100), obj.data.weapon.durability,
           obj.data.weapon.base_item.file_id,
           obj.data.weapon.base_item.global_id);
        break;
      }
      case TYPE_ARMOR: {
        fprintf(match, "%s %s %d %d %d %d\n", "TYPE_ARMOR",
          obj.data.armor.base_item.name,
          (int)(obj.data.armor.resistance * 100), obj.data.armor.durability,
          obj.data.armor.base_item.file_id,
          obj.data.armor.base_item.global_id);
          break;
        }
        default: {
          fprintf(match, "NULL\n");
      };
    }
  }
}

void save_hightscore(Player *player, char username[30], int curr_flor, int curr_hall) {
  //Nombre del Jugador, Pisos Alcanzados, Cantidad de Salas Exploradas, total de enemigos derrotados, objetos consumidos, Stats del jugador 
  FILE* match = fopen("hightscore.dat", "a");
  fprintf(match, "%s\n", username);
  fprintf(match, "%s\n", player->base_char.name);
  fprintf(match, "Último piso alcanzado: %d\n", curr_flor);
  fprintf(match, "Sala actual: %d\n", curr_hall);
  fprintf(match, "Total de enemigos derrotados: %d\n", player->base_char.stats.cont_kills);
  fprintf(match, "Total de objetos consumidos: %d\n", player->base_char.stats.cont_items_used);
  fprintf_player_stats(player, match);
  fprintf(match, "----------------------------------\n");
  fprintf(match, "Inventario: \n");
  
  // Tokenizar objetos
  tokenize_items(match, player);
  
  fclose(match);
}
void save_game_data(Player *player, char username[30], int curr_flor, int curr_hall) {
  //Nombre del Jugador, Pisos Alcanzados, Cantidad de Salas Exploradas, total de enemigos derrotados, objetos consumidos, Stats del jugador 
  FILE* match = fopen("match_data.dat", "w");
  fprintf(match, "%s\n", username);
  fprintf(match, "\t%s\n", player->base_char.name);
  fprintf(match, "\tlast_floor_reached: %d\n", curr_flor);
  fprintf(match, "\tcurrent_hall: %d\n", curr_hall);
  fprintf(match, "\ttotal_enemies_defeated: %d\n", player->base_char.stats.cont_kills);
  fprintf(match, "\ttotal_items_used: %d\n", player->base_char.stats.cont_items_used);
  fprintf(match, "-------- player_stats: -------- \n");
  fprintf_player_stats_formatted(player, match);
  fprintf(match, "---------- end_player_stats ----------\n");

  tokenize_items(match, player);
  fclose(match);
}
static void trim_whitespace(char* dest, const char* src, size_t dest_size) {
  if (!src || !dest || dest_size == 0) return;
  while (*src && isspace((unsigned char)*src)) src++;
  size_t len = strlen(src);
  while (len > 0 && isspace((unsigned char)src[len - 1])) len--;
  if (len >= dest_size) len = dest_size - 1;
  strncpy(dest, src, len);
  dest[len] = '\0';
}

static char* find_last_n_tokens(char* str, int n) {
  // regresa los ultimos n tokens/palabras de un string
  // ejemplo, con n = 2: "pasa 10 monedas a juan" -> "a juan"
  if (!str) return NULL;
  int len = (int)strlen(str);
  int tokens_found = 0;
  int in_token = 0;
  // iteramos desde el el ultimo caracter
  for (int i = len - 1; i >= 0; i--) {
    if (!isspace((unsigned char)str[i])) {
      if (!in_token) {
        in_token = 1;
        tokens_found++;
      }
      if (tokens_found == n) {
        // si encontramos la cantidad de tokens que buscabamos iteramos hasta el principio de ese ultimo token
        // asi nos posicionamos al inicio de la subcadena que queremos extraer
        while (i >= 0 && !isspace((unsigned char)str[i])) {
          i--;
        }
        // retornamos el puntero al inicio de la subcadena
        return &str[i + 1];
      }
    } else {
      in_token = 0;
    }
  }
  return NULL; // NULL cunado la cantidad de tokens es mayor a los que puede extraer de la cadena original
}

void read_tokenized_items(const char* line, bool equip, Player* player) {
    // lee objetos de tipo Object_data
    if (!line || !player) return;

    const char* str = strchr(line, ':');
    if (str != NULL) {
        str = str + 1;
    } else {
        str = line;
    }

    while (*str && isspace((unsigned char)*str)) str++;
    if (*str == '\0') return;

    char first_token[64] = "";
    int token_len = 0;
    while (str[token_len] != '\0' && !isspace((unsigned char)str[token_len])) {
        token_len++;
    }
    if (token_len == 0) return;
    if (token_len >= (int)sizeof(first_token)) token_len = sizeof(first_token) - 1;
    strncpy(first_token, str, token_len);
    first_token[token_len] = '\0';

    ItemType type;
    if (strcmp(first_token, "TYPE_CONSUMABLE") == 0 || strcmp(first_token, "Item") == 0) {
        type = TYPE_CONSUMABLE;
    } else if (strcmp(first_token, "TYPE_WEAPON") == 0 || strcmp(first_token, "Weapon") == 0) {
        type = TYPE_WEAPON;
    } else if (strcmp(first_token, "TYPE_ARMOR") == 0 || strcmp(first_token, "Armor") == 0) {
        type = TYPE_ARMOR;
    } else {
        return;
    }

    char* rest = (char*)(str + token_len);
    while (*rest && isspace((unsigned char)*rest)) rest++;
    if (*rest == '\0') return;

    switch (type) {
        case TYPE_CONSUMABLE: {
            char* token_ptr = find_last_n_tokens(rest, 6);
            if (token_ptr != NULL) {
                char item_name[MAX_STRING] = "";
                size_t name_len = token_ptr - rest;
                if (name_len >= sizeof(item_name)) name_len = sizeof(item_name) - 1;
                strncpy(item_name, rest, name_len);
                item_name[name_len] = '\0';
                char trimmed_name[MAX_STRING];
                trim_whitespace(trimmed_name, item_name, sizeof(trimmed_name));
                for (int i = 0; trimmed_name[i] != '\0'; i++) {
                    if (trimmed_name[i] == '_') trimmed_name[i] = ' ';
                }

                int function_val = 0;
                int quantity_val = 1;
                int can_use_val = 1;
                char target_str[64] = "TARGET_PLAYER";
                int item_id = 0;
                int global_id = 0;

                if (sscanf(token_ptr, "%d %d %d %63s %d %d",
                           &function_val, &quantity_val, &can_use_val, target_str, &item_id, &global_id) == 6) {
                    ObjectData obj;
                    memset(&obj, 0, sizeof(ObjectData));
                    obj.type = TYPE_CONSUMABLE;
                    obj.data.item.type = TYPE_CONSUMABLE;
                    obj.data.item.file_id = item_id;
                    obj.data.item.global_id = global_id;
                    strncpy(obj.data.item.name, trimmed_name, MAX_STRING - 1);
                    obj.data.item.name[MAX_STRING - 1] = '\0';
                    obj.data.item.function = function_val;
                    obj.data.item.quantity = quantity_val;
                    obj.data.item.can_use_outside_battle = (can_use_val != 0);
                    obj.data.item.target_type = (strcmp(target_str, "TARGET_ENEMY") == 0) ? TARGET_ENEMY : TARGET_PLAYER;
                    obj.data.item.use_function = use_generic_item;
                    snprintf(obj.data.item.description, sizeof(obj.data.item.description),
                             (obj.data.item.target_type == TARGET_PLAYER)
                                 ? "Restaura en un %d%% la salud del personaje"
                                 : "Inflige un %d%% de daño al objetivo", function_val);

                    int idx = -1;
                    for (int i = 0; i < player->base_char.inventory_count; i++) {
                        if (player->base_char.inventory[i].type == TYPE_CONSUMABLE &&
                            (player->base_char.inventory[i].data.item.global_id == global_id ||
                             strcmp(player->base_char.inventory[i].data.item.name, trimmed_name) == 0)) {
                            idx = i;
                            break;
                        }
                    }
                    if (idx != -1) {
                        player->base_char.inventory[idx] = obj;
                    } else if (player->base_char.inventory_count < MAX_INVENTORY) {
                        player->base_char.inventory[player->base_char.inventory_count++] = obj;
                    }
                }
            }
            break;
        }
        case TYPE_WEAPON: {
            char* token_ptr = find_last_n_tokens(rest, 4);
            if (token_ptr != NULL) {
                char weapon_name[MAX_STRING] = "";
                size_t name_len = token_ptr - rest;
                if (name_len >= sizeof(weapon_name)) name_len = sizeof(weapon_name) - 1;
                strncpy(weapon_name, rest, name_len);
                weapon_name[name_len] = '\0';
                char trimmed_name[MAX_STRING];
                trim_whitespace(trimmed_name, weapon_name, sizeof(trimmed_name));
                for (int i = 0; trimmed_name[i] != '\0'; i++) {
                    if (trimmed_name[i] == '_') trimmed_name[i] = ' ';
                }

                int dmg_x100 = 0;
                int durability_val = 100;
                int item_id = 0;
                int global_id = 0;

                if (sscanf(token_ptr, "%d %d %d %d", &dmg_x100, &durability_val, &item_id, &global_id) == 4) {
                    ObjectData obj;
                    memset(&obj, 0, sizeof(ObjectData));
                    obj.type = TYPE_WEAPON;
                    obj.data.weapon.base_item.type = TYPE_WEAPON;
                    obj.data.weapon.base_item.file_id = item_id;
                    obj.data.weapon.base_item.global_id = global_id;
                    strncpy(obj.data.weapon.base_item.name, trimmed_name, MAX_STRING - 1);
                    obj.data.weapon.base_item.name[MAX_STRING - 1] = '\0';
                    obj.data.weapon.damage = (float)dmg_x100 / 100.0f;
                    obj.data.weapon.durability = durability_val;
                    obj.data.weapon.base_item.quantity = 1;
                    obj.data.weapon.base_item.can_use_outside_battle = false;
                    obj.data.weapon.base_item.target_type = TARGET_ENEMY;
                    obj.data.weapon.base_item.use_function = NULL;
                    snprintf(obj.data.weapon.base_item.description, sizeof(obj.data.weapon.base_item.description),
                             "%s (Daño: %.1f, Durabilidad: %d)", trimmed_name, obj.data.weapon.damage, durability_val);

                    int idx = -1;
                    for (int i = 0; i < player->base_char.inventory_count; i++) {
                        if (player->base_char.inventory[i].type == TYPE_WEAPON &&
                            (player->base_char.inventory[i].data.weapon.base_item.global_id == global_id ||
                             strcmp(player->base_char.inventory[i].data.weapon.base_item.name, trimmed_name) == 0)) {
                            idx = i;
                            break;
                        }
                    }
                    if (idx != -1) {
                        player->base_char.inventory[idx] = obj;
                    } else if (player->base_char.inventory_count < MAX_INVENTORY) {
                        idx = player->base_char.inventory_count;
                        player->base_char.inventory[player->base_char.inventory_count++] = obj;
                    }

                    if (equip && idx != -1) {
                        player->base_char.weapon = &player->base_char.inventory[idx].data.weapon;
                    }
                }
            }
            break;
        }
        case TYPE_ARMOR: {
            char* token_ptr = find_last_n_tokens(rest, 4);
            if (token_ptr != NULL) {
                char armor_name[MAX_STRING] = "";
                size_t name_len = token_ptr - rest;
                if (name_len >= sizeof(armor_name)) name_len = sizeof(armor_name) - 1;
                strncpy(armor_name, rest, name_len);
                armor_name[name_len] = '\0';
                char trimmed_name[MAX_STRING];
                trim_whitespace(trimmed_name, armor_name, sizeof(trimmed_name));
                for (int i = 0; trimmed_name[i] != '\0'; i++) {
                    if (trimmed_name[i] == '_') trimmed_name[i] = ' ';
                }

                int res_x100 = 0;
                int durability_val = 100;
                int item_id = 0;
                int global_id = 0;

                if (sscanf(token_ptr, "%d %d %d %d", &res_x100, &durability_val, &item_id, &global_id) == 4) {
                    ObjectData obj;
                    memset(&obj, 0, sizeof(ObjectData));
                    obj.type = TYPE_ARMOR;
                    obj.data.armor.base_item.type = TYPE_ARMOR;
                    obj.data.armor.base_item.file_id = item_id;
                    obj.data.armor.base_item.global_id = global_id;
                    strncpy(obj.data.armor.base_item.name, trimmed_name, MAX_STRING - 1);
                    obj.data.armor.base_item.name[MAX_STRING - 1] = '\0';
                    obj.data.armor.resistance = (float)res_x100 / 100.0f;
                    obj.data.armor.durability = durability_val;
                    obj.data.armor.base_item.quantity = 1;
                    obj.data.armor.base_item.can_use_outside_battle = false;
                    obj.data.armor.base_item.target_type = TARGET_PLAYER;
                    obj.data.armor.base_item.use_function = NULL;
                    snprintf(obj.data.armor.base_item.description, sizeof(obj.data.armor.base_item.description),
                             "%s (Defensa: %.0f%%, Durabilidad: %d)", trimmed_name, obj.data.armor.resistance * 100.0f, durability_val);

                    int idx = -1;
                    for (int i = 0; i < player->base_char.inventory_count; i++) {
                        if (player->base_char.inventory[i].type == TYPE_ARMOR &&
                            (player->base_char.inventory[i].data.armor.base_item.global_id == global_id ||
                             strcmp(player->base_char.inventory[i].data.armor.base_item.name, trimmed_name) == 0)) {
                            idx = i;
                            break;
                        }
                    }
                    if (idx != -1) {
                        player->base_char.inventory[idx] = obj;
                    } else if (player->base_char.inventory_count < MAX_INVENTORY) {
                        idx = player->base_char.inventory_count;
                        player->base_char.inventory[player->base_char.inventory_count++] = obj;
                    }

                    if (equip && idx != -1) {
                        player->base_char.defense = &player->base_char.inventory[idx].data.armor;
                    }
                }
            }
            break;
        }
    }
}

static int extract_int_after_colon(const char* line) {
  const char* p = strchr(line, ':');
  if (!p) return 0;
  return atoi(p + 1);
}

static void extract_str_after_colon(const char* line, char* dest, size_t dest_size) {
  const char* p = strchr(line, ':');
  if (!p) {
    if (dest_size > 0) dest[0] = '\0';
    return;
  }
  trim_whitespace(dest, p + 1, dest_size);
}

int get_last_int_element(char *str) {
  char **arr = split(str, " \t\r\n");
  int i = 0;
  if (arr[0] == NULL) {
    free(arr);
    return 0;
  }
  while(arr[i + 1] != NULL) {
    i++;
  }
  int last_idx = i;
  char *last_str = arr[last_idx];
  char *end_ptr;
  int n = (int)strtol(last_str, &end_ptr, 10);
  if (last_str == end_ptr) {
    printf("Error: No se pudo realizar ninguna conversión.\n");
    free(arr);
    return 0;
  }
  free(arr);
  return n;
}

char* get_last_str_element(char *str) {
  char **arr = split(str, " \t\r\n");
  int i = 0;
  if (arr[0] == NULL) {
    free(arr);
    return NULL;
  }
  while(arr[i + 1] != NULL) {
    i++;
  }
  int last_idx = i;
  char *last_str = arr[last_idx];
  free(arr);
  return last_str;
}

int load_game_data(const char* file_path, Player* player, char username[MAX_STRING], int* curr_flor, int* curr_hall) {
  const char* path = (file_path != NULL) ? file_path : "match_data.dat";
  FILE* file = fopen(path, "r");
  if (file == NULL) {
    printf("Error: No se pudo abrir el archivo de guardado en '%s'\n", path);
    return 0;
  }

  if (player != NULL) {
    memset(player, 0, sizeof(Player));
    player->base_char.health = 100;
    player->base_char.hp_max = 100;
    player->base_char.attack = 10;
    player->base_char.xp_level = 1;
    player->base_char.xp_threshold = 100;
    player->base_char.xp_points = 0;
    player->base_char.weapon = NULL;
    player->base_char.defense = NULL;
    player->base_char.inventory_count = 0;
  }

  if (username != NULL) {
    strncpy(username, "unknown", MAX_STRING - 1);
    username[MAX_STRING - 1] = '\0';
  }
  if (curr_flor != NULL) {
    *curr_flor = 1;
  }
  if (curr_hall != NULL) {
    *curr_hall = 1;
  }

  char line[512];
  char trimmed[512];
  char equipped_weapon_name[MAX_STRING] = "";
  char equipped_defense_name[MAX_STRING] = "";

  enum {
    SECTION_HEADER,
    SECTION_STATS,
    SECTION_ITEMS
  } section = SECTION_HEADER;

  int got_username = 0;
  int got_character_name = 0;

  while (fgets(line, sizeof(line), file) != NULL) {
    trim_whitespace(trimmed, line, sizeof(trimmed));
    if (trimmed[0] == '\0') {
      continue;
    }

    if (strstr(trimmed, "player_stats:") != NULL) {
      section = SECTION_STATS;
      continue;
    }
    if (strstr(trimmed, "end_player_stats") != NULL) {
      section = SECTION_ITEMS;
      continue;
    }

    if (section == SECTION_HEADER) {
      if (!got_username) {
        if (username != NULL) {
          strncpy(username, trimmed, 29);
          username[29] = '\0';
        }
        got_username = 1;
        continue;
      }

      if (strstr(trimmed, "last_floor_reached:") != NULL) {
        if (curr_flor != NULL) {
          *curr_flor = extract_int_after_colon(trimmed);
        }
        continue;
      }
      if (strstr(trimmed, "current_hall:") != NULL) {
        if (curr_hall != NULL) {
          *curr_hall = extract_int_after_colon(trimmed);
        }
        continue;
      }
      if (strstr(trimmed, "total_enemies_defeated:") != NULL) {
        if (player != NULL) {
          player->base_char.stats.cont_kills = extract_int_after_colon(trimmed);
        }
        continue;
      }
      if (strstr(trimmed, "total_items_used:") != NULL) {
        if (player != NULL) {
          player->base_char.stats.cont_items_used = extract_int_after_colon(trimmed);
        }
        continue;
      }

      // Nombre del personaje (ej. "Caballero")
      if (!got_character_name && strchr(trimmed, ':') == NULL) {
        if (player != NULL) {
          strncpy(player->base_char.name, trimmed, MAX_STRING - 1);
          player->base_char.name[MAX_STRING - 1] = '\0';
        }
        got_character_name = 1;
        continue;
      }
    } else if (section == SECTION_STATS) {
      if (strstr(trimmed, "level:") != NULL) {
        if (player != NULL) {
          player->base_char.xp_level = extract_int_after_colon(trimmed);
        }
      } else if (strstr(trimmed, "xp_threshold:") != NULL) {
        if (player != NULL) {
          player->base_char.xp_threshold = extract_int_after_colon(trimmed);
        }
      } else if (strstr(trimmed, "attack:") != NULL) {
        if (player != NULL) {
          player->base_char.attack = extract_int_after_colon(trimmed);
        }
      } else if (strstr(trimmed, "defense:") != NULL) {
        if (strstr(trimmed, "no_defense") == NULL) {
          read_tokenized_items(trimmed, true, player);
        }
      } else if (strstr(trimmed, "weapon:") != NULL) {
        if (strstr(trimmed, "no_weapon") == NULL) {
          read_tokenized_items(trimmed, true, player);
        }
      }
    } else if (section == SECTION_ITEMS) {
      if (player == NULL || player->base_char.inventory_count >= MAX_INVENTORY) {
        continue;
      }
      read_tokenized_items(trimmed, false, player);
    }
  }

  fclose(file);

  if (player != NULL) {
    int hp = 100;
    for (int lvl = 10; lvl < player->base_char.xp_level; lvl++) {
      hp += hp / 10;
    }
    player->base_char.hp_max = hp;
    player->base_char.health = hp;
  }

  return 1;
}

char int2rank(int enemy_range) {
  switch(enemy_range) {
    case 1:
      return 'D';
    case 2:
      return 'C';
    case 3:
      return 'B';
    case 4:
      return 'A';
    case 5:
      return 'S';
    case 6:
      return 'R';
    default:
      printf("Error: Rango de enemigo invalido\n");
      abort();
  }
}

int dungeon(int seed, int n_flors, int n_halls, int curr_flor, int curr_hall, int curr_enemy_range, Player *player, char* name)
{
  int total_halls = n_halls;
  if (curr_hall == 1) {
    total_halls -= 1;
  }
  if (curr_flor == n_flors) {
    total_halls += 1;
  }
  char enemy_range = int2rank((int)round((float)n_flors / 5.0));
  char  username[MAX_STRING];
  
  if (username[0] == '\0') {
    printf("¿Cual es tu nombre guerrero?\n");
    fgets(username, sizeof(username), stdin);
  } else {
    strncpy(username, name, MAX_STRING - 1);
    username[MAX_STRING - 1] = '\0';
  }
  while (curr_flor < n_flors) {
    while (curr_hall < n_halls) {
      int time_to_kill = 0;
      int enemy_lv_media = 0;
      int enemy_lv_std = curr_flor / 2;
      if (curr_hall > 1) { // aqui time_to_kill viene del piso anterior asi que es mayor a cero
        if (time_to_kill == 1) { // si el ttk es muy bajo le damos ventaja al enemigo sumandole un a la enemy_lv_media de su nivel que inicialmente es la misma que la del jugador
          enemy_lv_media = 1;
        } else if (time_to_kill > 3) { // si el ttk es muy alto le damos ventaja al jugador restandole uno a la enemy_lv_media de su nivel
          enemy_lv_media = -1;
        }
      }
      const char* options[] = {"Combatir", "Inventario", "Ver estadísticas", "Salir y Guardar partida"};
      Enemy enemy = create_random_enemy("bestiario.txt", enemy_range, global_id_counter++, player, enemy_lv_media, enemy_lv_std);
      int choise = menu(options, 4);
      switch(choise) {
        case 1:
          player->base_char.stats.rooms_visited += 1;
          printf("Haz entrado a la sala %d-%d\n", curr_flor, curr_hall);
          time_to_kill = combat(player, &enemy);
          printf("Tiempo para matar al enemigo: %d", time_to_kill);
          
          if (player->base_char.health == 0) {
            printf("Fin del juego");
            printf("%s\n", username);
            printf("%s\n", player->base_char.name);
            printf("Último piso alcanzado: %d\n", curr_flor);
            printf("Sala actual: %d\n", curr_hall);
            printf("Total de enemigos derrotados: %d\n", player->base_char.stats.cont_kills);
            printf("Total de objetos consumidos: %d\n", player->base_char.stats.cont_items_used);
            printf("------- Stats del jugador: -------- \n");
            show_player_stats(player);
            printf("----------------------------------\n");
            
            save_hightscore(player, username, curr_flor, curr_hall);
            printf("Puntaje guardado en hightscore.dat");
            return 0;
          }
          curr_hall += 1;
          player->base_char.stats.cont_kills+=1;
          break;
        case 2:
          open_inventory(&player->base_char, false, NULL);// implementar contador de items usados
          continue;
        case 3:
          show_player_stats(player);
          enter_to_continue();
          continue;
        case 4:
          save_game_data(player, username, curr_flor, curr_hall);
          return 0;
      }
      curr_hall += 1;
    }
    curr_hall = 1;
    if (curr_flor < n_flors) {
      curr_flor += 1;
      printf("\n");
      printf("Subiste al piso: %d\n", curr_flor);
      printf("Sala actual: %d\n", curr_hall);
      enter_to_continue();
    }
  }
  return 0;
}

//***************************** */
// cuerpo principal del juego
//***************************** */

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
  Weapon w = {0};
    w.base_item.type               = TYPE_WEAPON;
    w.base_item.global_id                 = next_id++;
    strncpy(w.base_item.name,        "Espada de hierro", MAX_STRING - 1);
  w.base_item.name[MAX_STRING - 1] = '\0';
  strncpy(w.base_item.description, "Arma cuerpo a cuerpo muy popular y eficaz", sizeof(w.base_item.description) - 1);
  w.base_item.description[sizeof(w.base_item.description) - 1] = '\0';
    w.base_item.quantity           = 1;
    w.base_item.can_use_outside_battle = false;
    w.base_item.target_type        = TARGET_ENEMY;
    w.base_item.use_function       = equip_iron_sword;
    w.damage                       = 3.0f;
    w.durability                   = 100;

    *out_weapon = w;           // Escribir en el buffer del llamador

    ObjectData obj;
    obj.type         = TYPE_WEAPON;
    memcpy(&obj.data.weapon, &w, sizeof(obj.data.weapon));
    return obj;
}

ObjectData create_iron_armor(Armor* out_armor) {
  Armor a = {0};
    a.base_item.type               = TYPE_ARMOR;
    a.base_item.global_id                 = next_id++;
    strncpy(a.base_item.name,        "Armadura de hierro", MAX_STRING - 1);
  a.base_item.name[MAX_STRING - 1] = '\0';
  strncpy(a.base_item.description, "Armadura que protege el cuerpo de ataques físicos", sizeof(a.base_item.description) - 1);
  a.base_item.description[sizeof(a.base_item.description) - 1] = '\0';
    a.base_item.quantity           = 1;
    a.base_item.can_use_outside_battle = false;
    a.base_item.target_type        = TARGET_PLAYER;
    a.base_item.use_function       = equip_iron_armor;
    a.resistance                   = 0.5f;   // 50% reducción de daño físico
    a.durability                   = 100;

    *out_armor = a;

    ObjectData obj;
    obj.type        = TYPE_ARMOR;
    memcpy(&obj.data.armor, &a, sizeof(obj.data.armor));
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