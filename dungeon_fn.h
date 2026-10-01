#ifndef DUNGEON_H
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <math.h>
#include "types.h"
#include "constants.h"
#include "utils.h"
#include "enemies.h"

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
            break;
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

#endif