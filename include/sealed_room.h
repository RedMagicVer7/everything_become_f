/**
 * sealed_room.h - Sealed Room Model
 * ===================================
 * 
 * Models the sealed room (密室) where Dr. Shiki Magata was confined
 * for 15 years in the Magata Research Facility.
 * 
 * The only way out: system overflow triggers failsafe → locks open.
 */

#ifndef SEALED_ROOM_H
#define SEALED_ROOM_H

#include <stdbool.h>
#include <stdint.h>
#include "electromagnetic_lock.h"

#define MAX_ROOM_ID_LEN 64
#define MAX_ROOM_NAME_LEN 128
#define MAX_DOORS_IN_ROOM 4
#define MAX_ROOM_STATE_CALLBACKS 4
#define MAGATA_CONFINEMENT_YEARS 15
#define MAGATA_CONFINEMENT_HOURS 131490  /* 15 * 365.25 * 24 */

/* Room states */
typedef enum {
    ROOM_STATE_SEALED,      /* Room is fully sealed, no escape possible */
    ROOM_STATE_BREACHED,    /* Locks released, escape possible */
    ROOM_STATE_ESCAPED,     /* Occupant has escaped */
    ROOM_STATE_COMPROMISED  /* Security compromised but not escaped */
} RoomState;

/**
 * Door - A door in the sealed room
 */
typedef struct {
    char door_id[MAX_ROOM_ID_LEN];
    char name[MAX_ROOM_NAME_LEN];
    bool is_one_way;        /* Can only be opened from outside */
    ElectromagneticLock *lock;
    bool is_open;
} Door;

/* Door functions */
void door_init(Door *door, const char *door_id, const char *name, bool is_one_way);
bool door_can_open_from_inside(const Door *door);
bool door_can_open_from_outside(const Door *door);
bool door_open(Door *door);
void door_close(Door *door);

/* Forward declaration */
typedef struct SealedRoom SealedRoom;

/* Callback type */
typedef void (*room_state_callback_t)(SealedRoom *room, 
                                      RoomState old_state, 
                                      RoomState new_state,
                                      void *context);

/**
 * SealedRoom - A sealed room in the research facility
 */
struct SealedRoom {
    char room_id[MAX_ROOM_ID_LEN];
    char name[MAX_ROOM_NAME_LEN];
    RoomState state;
    Door doors[MAX_DOORS_IN_ROOM];
    int door_count;
    bool surveillance_active;
    bool occupant_present;
    int32_t breach_time;     /* Hour when breached, -1 if not breached */
    int32_t escape_time;     /* Hour when escaped, -1 if not escaped */
    room_state_callback_t state_callbacks[MAX_ROOM_STATE_CALLBACKS];
    void *callback_contexts[MAX_ROOM_STATE_CALLBACKS];
    int callback_count;
};

/* Initialize sealed room */
void sealed_room_init(SealedRoom *room, const char *room_id, const char *name);

/* State queries */
RoomState sealed_room_get_state(const SealedRoom *room);
bool sealed_room_is_sealed(const SealedRoom *room);
bool sealed_room_is_breached(const SealedRoom *room);
bool sealed_room_is_escaped(const SealedRoom *room);
bool sealed_room_occupant_present(const SealedRoom *room);

/* Door management */
Door *sealed_room_add_door(SealedRoom *room, const char *door_id, 
                           const char *name, bool is_one_way,
                           ElectromagneticLock *lock);
int sealed_room_get_door_count(const SealedRoom *room);

/* Check seal integrity */
bool sealed_room_check_seal_integrity(const SealedRoom *room);

/* State transitions */
bool sealed_room_breach(SealedRoom *room, int32_t hour);
bool sealed_room_escape(SealedRoom *room, int32_t hour);

/* Get breach/escape times */
int32_t sealed_room_get_breach_time(const SealedRoom *room);
int32_t sealed_room_get_escape_time(const SealedRoom *room);

/* Register state change callback */
bool sealed_room_register_state_callback(SealedRoom *room,
                                         room_state_callback_t cb, void *ctx);

/**
 * MagataQuarters - Dr. Shiki Magata's sealed living quarters
 * 
 * She was confined here for 15 years (from age 14 to 29).
 */
typedef struct {
    SealedRoom base;
    ElectromagneticLock main_lock;
    Door main_door;
    int confinement_start_year;
    int confinement_end_year;
    bool has_network_access;
    bool can_program;
} MagataQuarters;

/* Initialize Magata's quarters */
void magata_quarters_init(MagataQuarters *quarters);

/* Get the base room */
SealedRoom *magata_quarters_get_room(MagataQuarters *quarters);

/* Get main lock */
ElectromagneticLock *magata_quarters_get_main_lock(MagataQuarters *quarters);

/* Trigger overflow escape sequence */
bool magata_quarters_trigger_overflow_escape(MagataQuarters *quarters, int32_t hour);

/* Property queries */
bool magata_quarters_has_network_access(const MagataQuarters *quarters);
bool magata_quarters_can_program(const MagataQuarters *quarters);

#endif /* SEALED_ROOM_H */
