#ifndef DUNGEON_H
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <math.h>
#include "types.h"
#include "utils.h"
#include "enemies.h"



void save_game_data(Player *player, int curr_flor, int curr_hall, int cont_flors, int cont_halls, int cont_enemys, int cont_items) {
  //Nombre del Jugador, Pisos Alcanzados, Cantidad de Salas Exploradas, total de enemigos derrotados, objetos consumidos, Stats del jugador 
  FILE* match = fopen("match_data.dat", "w");
}


void dungeon(int seed, int n_flors, int n_halls, int curr_flor, int curr_hall, int curr_enemy_range, Player *player, Enemy *enemy)
{

  total_halls = n_halls;
  if (curr_hall == 1) {
    total_halls -= 1;
  }
  if (curr_flor == n_flors) {
    total_halls += 1;
  }
  int enemy_range = round((float)n_flors / 5.0);
  
  

  while (curr_hall < n_halls) {
    char options[] = ["Salir y Guardar partida", "Inventario", "Combatir"];
    int choise = menu(options, 3);
    switch(choise) {
      case 1:
        
        break;
      case 2:
        open_inventory(&player->base_char, false, NULL);// implementar contador de items usados
        break;
      case 3:
        player->base_char.rooms_visited += 1;
        printf("Haz entrado a la sala %d-%d\n", curr_flor, curr_hall);
        combat(&player, &enemy);
        curr_hall += 1;
        player->base_char.cont_kills+=1;
        break;
    }
    
  }
  curr_flor += 1;
  curr_hall = 1;
}

#ifndef