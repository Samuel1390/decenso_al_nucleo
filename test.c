#include <stdio.h>
#include "types.h"
#include "dungeon_fn.h"

int main() {
    Player player;
    char username[MAX_STRING];
    int curr_flor = 0, curr_hall = 0;

    if (load_game_data("match_data.dat", &player, username, &curr_flor, &curr_hall)) {
        printf("--- GAME DATA LOADED SUCCESSFULLY ---\n");
        printf("Username: %s\n", username);
        printf("Player Name: %s\n", player.base_char.name);
        printf("Floor: %d, Hall: %d\n", curr_flor, curr_hall);
        printf("Level: %d, Attack: %d\n", player.base_char.xp_level, player.base_char.attack);

        if (player.base_char.weapon != NULL) {
            printf("Equipped Weapon: %s (Damage: %.2f)\n",
                   player.base_char.weapon->base_item.name, player.base_char.weapon->damage);
        } else {
            printf("Equipped Weapon: None\n");
        }

        if (player.base_char.defense != NULL) {
            printf("Equipped Defense: %s (Resistance: %.2f)\n",
                   player.base_char.defense->base_item.name, player.base_char.defense->resistance);
        } else {
            printf("Equipped Defense: None\n");
        }

        printf("Inventory Count: %d\n", player.base_char.inventory_count);
        for (int i = 0; i < player.base_char.inventory_count; i++) {
            ObjectData* obj = &player.base_char.inventory[i];
            printf("  Item %d: Type=%d, Name=%s\n", i + 1, obj->type, get_obj_name(obj));
        }
    } else {
        printf("Failed to load game data.\n");
    }

    return 0;
}
