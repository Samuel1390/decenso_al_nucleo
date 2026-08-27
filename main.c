#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>
#include "utils.h"
#include "types.h"
#include "constants.h"


int next_id = 1;

ObjectData create_iron_sword() {
    Weapon base_iron_sword = {TYPE_WEAPON,{"Espada de hierro", "Arma cuerpo a cuerpo muy popular y eficaz"}, 3, 100};
    ObjectData iron_sword;
    iron_sword.type = TYPE_WEAPON;
    iron_sword.data.weapon.base_item.id = next_id++;
    iron_sword.data.weapon = base_iron_sword;
    void equip_sword(Player* player) {
        Weapon previous_weapon = player->base_char.weapon;
        if (previous_weapon) {
            // Si ya habia un arma equipada antes la intercambiamos por la nueva
            int free_idx = get_idx_from_id(player, iron_sword.data.weapon.base_item.id);
            remove_item(player, free_idx);
            player->base_char.weapon = &iron_sword.data.weapon;
            // Reconstruimos el arma previa
            ObjectData prev_w;
            prev_w.type = TYPE_WEAPON;
            prev_w.data.weapon = *previous_weapon;
            player->inventory[free_idx] = prev_w;
            printf("Se ha intercambiado %s por %s\n", previous_weapon->base_item.name, iron_sword.base_item.name);
            enter_to_continue();
        }
        player->base_char.weapon = &iron_sword.data.weapon;
        printf("%s se ha equipado con %s\n", player->base_char.name, iron_sword.base_item.name);
        enter_to_continue();
    }
}


ObjectData  create_healh_potion(int health_percentage, char* subffix, int quantity) {
    int quant = quantity;
    if (quant == NULL || quant <= 0) {
        quant = 1;
    }
    char name[20] = strcat("pocion de salud ", subffix);
    char description[100] = strcat("Restaura en un ", health_percentage"% la salud del personaje");
    ObjectData potion;
    potion.type = TYPE_CONSUMABLE;
    potion.data.item.id = next_id++;
    potion.data.item.name = name;
    potion.data.item.description = description;
    potion.data.item.quantity = 1;
    potion.data.item.can_use_outside_battle = true;
    potion.data.item.target_type = TARGET_PLAYER;
    
    float f_hp = (float)health_percentage / 100.0;

    void use_health_potion(Character* target) {
        target->health = min(target->health + target->hp_max * f_hp, target->hp_max);
        printf("%s ha recuperado %d puntos de salud\n", target->name, (int)(target->hp_max * f_hp));
        draw_progress_bar(target->health, target->hp_max, "HP");
        enter_to_continue();
    }
    potion.data.item.use_function = use_health_potion;
    return potion;
}

int main(void) {
    
    Weapon base_iron_sword = {TYPE_WEAPON,{"Espada de hierro", "Arma cuerpo a cuerpo muy popular y eficaz", 1, true, equip_sword}, 3, 100};
    // Armor base_iron_armor = {TYPE_ARMOR, {"Armadura de hierro", "Armadura que protege el cuerpo de ataques fisicos", 1, true, equip_armor}, 3, 100};

    ObjectData iron_sword;
    iron_sword.type = TYPE_WEAPON;
    iron_sword.data.weapon = base_iron_sword;
    // iron_armor.type = TYPE_ARMOR;
    // iron_armor.data.armor = base_iron_armor;
    ObjectData inventory_player[MAX_INVENTORY];
    Player player;
    init_character(&player.base_char, "Caballero", 80, 100, 100, NULL, 1, 100, 0, &iron_sword);
    ObjectData health_potion_I = create_healh_potion(40, "I");
    ObjectData health_potion_II = create_healh_potion(70, "II");
    
    player.inventory[0] = health_potion_I;
    player.inventory[1] = health_potion_II;
    player.inventory_count = 2;
    Character enemy1, enemy2;
    init_character(&enemy1, "Goblin", 10, 80, 80, NULL, 1, 100, 0, NULL);
    init_character(&enemy2, "Goblin", 20, 90, 90, NULL, 2, 110, 0, NULL);
    combat(&player, &enemy1);
    combat(&player, &enemy2);
    return 0;
}