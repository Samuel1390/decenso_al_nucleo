
#ifndef GETTERS_H
#define GETTERS_H

#include "types.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "constants.h"

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

int get_obj_id(ObjectData* obj) {
    switch (obj->type) {
        case TYPE_CONSUMABLE: return obj->data.item.id;
        case TYPE_WEAPON:     return obj->data.weapon.base_item.id;
        case TYPE_ARMOR:      return obj->data.armor.base_item.id;
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

#endif // GETTERS_H
