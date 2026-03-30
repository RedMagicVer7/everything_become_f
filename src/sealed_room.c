/**
 * sealed_room.c - Sealed Room Model Implementation
 */

#include <string.h>
#include "../include/sealed_room.h"

/* ============================================================================
 * Door Implementation
 * ============================================================================ */

void door_init(Door *door, const char *door_id, const char *name, bool is_one_way) {
    if (!door) return;
    
    memset(door, 0, sizeof(Door));
    
    if (door_id) {
        strncpy(door->door_id, door_id, MAX_ROOM_ID_LEN - 1);
    }
    if (name) {
        strncpy(door->name, name, MAX_ROOM_NAME_LEN - 1);
    }
    
    door->is_one_way = is_one_way;
    door->lock = NULL;
    door->is_open = false;
}

bool door_can_open_from_inside(const Door *door) {
    if (!door) return false;
    if (door->is_one_way) return false;
    if (door->lock && lock_is_locked(door->lock)) return false;
    return true;
}

bool door_can_open_from_outside(const Door *door) {
    if (!door) return false;
    if (door->lock && lock_is_locked(door->lock)) return false;
    return true;
}

bool door_open(Door *door) {
    if (!door) return false;
    if (door->lock && lock_is_locked(door->lock)) return false;
    door->is_open = true;
    return true;
}

void door_close(Door *door) {
    if (door) {
        door->is_open = false;
    }
}

/* ============================================================================
 * SealedRoom Implementation
 * ============================================================================ */

static void notify_room_state_change(SealedRoom *room,
                                     RoomState old_state, RoomState new_state) {
    for (int i = 0; i < room->callback_count; i++) {
        if (room->state_callbacks[i]) {
            room->state_callbacks[i](room, old_state, new_state,
                                     room->callback_contexts[i]);
        }
    }
}

void sealed_room_init(SealedRoom *room, const char *room_id, const char *name) {
    if (!room) return;
    
    memset(room, 0, sizeof(SealedRoom));
    
    if (room_id) {
        strncpy(room->room_id, room_id, MAX_ROOM_ID_LEN - 1);
    }
    if (name) {
        strncpy(room->name, name, MAX_ROOM_NAME_LEN - 1);
    }
    
    room->state = ROOM_STATE_SEALED;
    room->door_count = 0;
    room->surveillance_active = true;
    room->occupant_present = true;
    room->breach_time = -1;
    room->escape_time = -1;
    room->callback_count = 0;
}

RoomState sealed_room_get_state(const SealedRoom *room) {
    return room ? room->state : ROOM_STATE_SEALED;
}

bool sealed_room_is_sealed(const SealedRoom *room) {
    return room && room->state == ROOM_STATE_SEALED;
}

bool sealed_room_is_breached(const SealedRoom *room) {
    return room && (room->state == ROOM_STATE_BREACHED || 
                    room->state == ROOM_STATE_ESCAPED);
}

bool sealed_room_is_escaped(const SealedRoom *room) {
    return room && room->state == ROOM_STATE_ESCAPED;
}

bool sealed_room_occupant_present(const SealedRoom *room) {
    return room && room->occupant_present;
}

Door *sealed_room_add_door(SealedRoom *room, const char *door_id,
                           const char *name, bool is_one_way,
                           ElectromagneticLock *lock) {
    if (!room) return NULL;
    if (room->door_count >= MAX_DOORS_IN_ROOM) return NULL;
    
    Door *door = &room->doors[room->door_count];
    door_init(door, door_id, name, is_one_way);
    door->lock = lock;
    room->door_count++;
    
    return door;
}

int sealed_room_get_door_count(const SealedRoom *room) {
    return room ? room->door_count : 0;
}

bool sealed_room_check_seal_integrity(const SealedRoom *room) {
    if (!room) return false;
    
    for (int i = 0; i < room->door_count; i++) {
        const Door *door = &room->doors[i];
        if (door->lock && !lock_is_locked(door->lock)) {
            return false;
        }
        if (door->is_open) {
            return false;
        }
    }
    return true;
}

bool sealed_room_breach(SealedRoom *room, int32_t hour) {
    if (!room) return false;
    if (room->state != ROOM_STATE_SEALED) return false;
    
    RoomState old_state = room->state;
    room->state = ROOM_STATE_BREACHED;
    room->breach_time = hour;
    
    notify_room_state_change(room, old_state, room->state);
    return true;
}

bool sealed_room_escape(SealedRoom *room, int32_t hour) {
    if (!room) return false;
    if (room->state != ROOM_STATE_BREACHED) return false;
    if (!room->occupant_present) return false;
    
    RoomState old_state = room->state;
    room->state = ROOM_STATE_ESCAPED;
    room->occupant_present = false;
    room->escape_time = hour;
    
    notify_room_state_change(room, old_state, room->state);
    return true;
}

int32_t sealed_room_get_breach_time(const SealedRoom *room) {
    return room ? room->breach_time : -1;
}

int32_t sealed_room_get_escape_time(const SealedRoom *room) {
    return room ? room->escape_time : -1;
}

bool sealed_room_register_state_callback(SealedRoom *room,
                                         room_state_callback_t cb, void *ctx) {
    if (!room || !cb) return false;
    if (room->callback_count >= MAX_ROOM_STATE_CALLBACKS) return false;
    
    room->state_callbacks[room->callback_count] = cb;
    room->callback_contexts[room->callback_count] = ctx;
    room->callback_count++;
    return true;
}

/* ============================================================================
 * MagataQuarters Implementation
 * ============================================================================ */

void magata_quarters_init(MagataQuarters *quarters) {
    if (!quarters) return;
    
    memset(quarters, 0, sizeof(MagataQuarters));
    
    /* Initialize base room */
    sealed_room_init(&quarters->base, "magata_quarters",
                     "Dr. Magata's Quarters");
    
    /* Initialize main lock */
    lock_init(&quarters->main_lock, "magata_main_lock",
              "Main entrance to Magata's quarters");
    
    /* Initialize main door with lock reference */
    door_init(&quarters->main_door, "magata_main_door", "Main Entrance", true);
    quarters->main_door.lock = &quarters->main_lock;
    
    /* Add door to room */
    quarters->base.doors[0] = quarters->main_door;
    quarters->base.doors[0].lock = &quarters->main_lock;
    quarters->base.door_count = 1;
    
    /* Set confinement info */
    quarters->confinement_start_year = 1979;
    quarters->confinement_end_year = 1994;
    quarters->has_network_access = true;
    quarters->can_program = true;
}

SealedRoom *magata_quarters_get_room(MagataQuarters *quarters) {
    return quarters ? &quarters->base : NULL;
}

ElectromagneticLock *magata_quarters_get_main_lock(MagataQuarters *quarters) {
    return quarters ? &quarters->main_lock : NULL;
}

bool magata_quarters_trigger_overflow_escape(MagataQuarters *quarters, int32_t hour) {
    if (!quarters) return false;
    
    /* Step 1: Trigger lock failsafe */
    lock_trigger_failsafe(&quarters->main_lock);
    
    /* Step 2: Open main door */
    quarters->base.doors[0].is_open = true;
    
    /* Step 3: Breach the room */
    sealed_room_breach(&quarters->base, hour);
    
    /* Step 4: Escape */
    return sealed_room_escape(&quarters->base, hour);
}

bool magata_quarters_has_network_access(const MagataQuarters *quarters) {
    return quarters && quarters->has_network_access;
}

bool magata_quarters_can_program(const MagataQuarters *quarters) {
    return quarters && quarters->can_program;
}
