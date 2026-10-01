#ifndef ENEMIES_H
#define ENEMIES_H
#include "types.h"
#include "constants.h"
#include "utils.h"

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

#endif // ENEMIES_H