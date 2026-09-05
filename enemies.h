#ifndef ENEMIES_H
#define ENEMIES_H
#include "types.h"
#include "constants.h"
#include "utils.h"

void normalize_enemy(Player *player, Enemy *enemy) {
  int player_level = player->base_char.xp_level;

  int enemy_level = enemy->base_char.xp_level;
  int new_level = (int)get_random(max_int(player_level - 2, 1), player_level + 2);
  enemy->base_char.xp_level = new_level;
  int xp_points = drop_xp(enemy, false);
  level_up(&enemy->base_char, xp_points);
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
        enemy.base_char.id = global_id;
        normalize_enemy(player, &enemy);
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


#endif // ENEMIES_H