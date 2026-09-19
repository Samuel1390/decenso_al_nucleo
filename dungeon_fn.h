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
  fprintf(match, "------- Stats del jugador: -------- \n");
  fprintf_player_stats(player, match);
  fprintf(match, "----------------------------------\n");
  fclose(match);
}
void load_game_data(char* file_path) {
  FILE* match = fopen(file_path, "r");
  char username[30];
  char name[MAX_STRING];
  int curr_hall;
  int total_enemies_defeated;
  int total_item_used;
  int xp_level;
  int attack;
  Armor defense
  
  fgets(username, sizeof(username), stdin);
  fgets(name, sizeof(name), stdin);
  fgets()

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