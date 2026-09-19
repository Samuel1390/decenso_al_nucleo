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

void tokenize_items(FILE* match, Player* player) {
  for (int i=0; i < player->base_char.inventory_count; i++) {
    ObjectData obj = player->base_char.inventory[i];
    switch(obj.type) {
      case TYPE_CONSUMABLE: {
        char* target_type = obj.data.item.target_type == TARGET_PLAYER ? "TARGET_PLAYER" : "TARGET_ENEMY";
        fprintf(match, "%s %s %d %d %d %s %d\n", "TYPE_CONSUMABLE",
          obj.data.item.name,
          obj.data.item.function,
          obj.data.item.quantity, obj.data.item.can_use_outside_battle,
          target_type,
          obj.data.item.id);
        break;
      }
      case TYPE_WEAPON: {
        fprintf(match, "%s %s %d %d %d\n", "TYPE_WEAPON",
          obj.data.weapon.base_item.name,
          (int)(obj.data.weapon.damage * 100), obj.data.weapon.durability,
           obj.data.weapon.base_item.id);
        break;
      }
      case TYPE_ARMOR: {
        fprintf(match, "%s %s %d %d %d\n", "TYPE_ARMOR",
          obj.data.armor.base_item.name,
          (int)(obj.data.armor.resistance * 100), obj.data.armor.durability,
          obj.data.armor.base_item.id);
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
  fprintf(match, "\t%s\n", player->base_char.name);
  fprintf(match, "\tÚltimo piso alcanzado: %d\n", curr_flor);
  fprintf(match, "\tSala actual: %d\n", curr_hall);
  fprintf(match, "\tTotal de enemigos derrotados: %d\n", player->base_char.stats.cont_kills);
  fprintf(match, "\tTotal de objetos consumidos: %d\n", player->base_char.stats.cont_items_used);
  fprintf(match, "------- Stats del jugador: -------- \n");
  fprintf_player_stats(player, match);
  fprintf(match, "----------------------------------\n");
  tokenize_items(match, player);
  fclose(match);
}
void save_game_data(Player *player, char username[30], int curr_flor, int curr_hall) {
  //Nombre del Jugador, Pisos Alcanzados, Cantidad de Salas Exploradas, total de enemigos derrotados, objetos consumidos, Stats del jugador 
  FILE* match = fopen("match_data.dat", "w");
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
int get_last_int_element(char *str) {
  char **arr = split(str, " ");
  int i = 0;
  while(arr[i + 1] != NULL) {
    i++;
  }
  int last_idx = i;
  char *last_str = arr[last_idx];
  char *end_ptr;
  int n = strtol(last_str, &end_ptr, 10);
  if (last_str == end_ptr) {
    printf("Error: No se pudo realizar ninguna conversión.\n");
  } else {
    return n;
  }
}
char* get_last_str_element(char *str) {
  char **arr = split(str, " ");
  int i = 0;
  while(arr[i + 1] != NULL) {
    i++;
  }
  int last_idx = i;
  return arr[last_idx];
}
void load_game_data(char* file_path) {
  FILE* match = fopen(file_path, "r");
  char username[30];
  char name[MAX_STRING];
  int curr_hall;
  int total_enemies_defeated;
  int total_items_used;
  int xp_level;
  int attack;
  char defense_name[MAX_STRING];
  char attack_name[MAX_STRING];
  char line[MAX_STRING];
  
  fgets(username, sizeof(username), match);
  fgets(name, sizeof(name), match);
  // pares clave/valor
  fgets(line, sizeof(line), match); // Sala actual: i
  curr_hall = get_last_int_element(line);

  fgets(line, (int)sizeof(line), match); // Total de enemigos derrotados: j
  total_enemies_defeated = get_last_int_element(line);

  fgets(line, sizeof(line), match); // Total de objetos consumidos: k
  total_items_used = get_last_int_element(line);

  fgets(line, sizeof(line), match); // estadisticas de caballero

  fgets(line, sizeof(line), match); // nivel: n
  xp_level = get_last_int_element(line);

  fgets(line, sizeof(line), match); // Ataque: r
  attack = get_last_int_element(line);

  fgets(line, sizeof(line), match); // Defensa: nombre_def
  strcpy(defense_name,get_last_str_element(line));

  fgets(line, sizeof(line), match); // Arma: nombre_wea
  strcpy(attack_name,get_last_str_element(line));
  // Luego viene la longitud del inventario
}


void dungeon(int seed, int n_flors, int n_halls, int curr_flor, int curr_hall, int curr_enemy_range, Player *player, Enemy *enemy)
{
  int total_halls = n_halls;
  if (curr_hall == 1) {
    total_halls -= 1;
  }
  if (curr_flor == n_flors) {
    total_halls += 1;
  }
  int enemy_range = round((float)n_flors / 5.0);
  
  char username[30];
  printf("¿Cual es tu nombre guerrero?\n");
  fgets(username, sizeof(username), stdin);
  while (curr_hall < n_halls) {
    const char* options[] = {"Salir y Guardar partida", "Inventario", "Combatir"};
    int choise = menu(options, 3);
    switch(choise) {
      case 1:
        save_game_data(player, username, curr_flor, curr_hall);
        break;
      case 2:
        open_inventory(&player->base_char, false, NULL);// implementar contador de items usados
        break;
      case 3:
        player->base_char.stats.rooms_visited += 1;
        printf("Haz entrado a la sala %d-%d\n", curr_flor, curr_hall);
        combat(player, enemy);
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
    }
    
  }
  curr_flor += 1;
  curr_hall = 1;
}

#endif