/**
 * electromagnetic_lock.h - Electromagnetic Lock Control System
 * ==============================================================
 * 
 * Models the electromagnetic locks in the Magata Research Facility.
 * These locks are fail-safe: they remain locked when powered,
 * and unlock when power is cut or when system overflow occurs.
 */

#ifndef ELECTROMAGNETIC_LOCK_H
#define ELECTROMAGNETIC_LOCK_H

#include <stdbool.h>
#include <stddef.h>

#define MAX_LOCK_ID_LEN 64
#define MAX_LOCATION_LEN 128
#define MAX_LOCKS_IN_SYSTEM 16
#define MAX_LOCK_STATE_CALLBACKS 4
#define MAX_FAILSAFE_CALLBACKS 4

/* Lock states */
typedef enum {
    LOCK_STATE_LOCKED,        /* Normal operation - power on, door sealed */
    LOCK_STATE_UNLOCKED,      /* Power cut or failsafe triggered */
    LOCK_STATE_MALFUNCTIONING /* Error state */
} LockState;

/* Forward declarations */
typedef struct ElectromagneticLock ElectromagneticLock;
typedef struct LockSystem LockSystem;

/* Callback types */
typedef void (*lock_state_callback_t)(ElectromagneticLock *lock, 
                                      LockState old_state, 
                                      LockState new_state,
                                      void *context);
typedef void (*lock_system_failsafe_callback_t)(LockSystem *system, void *context);

/**
 * ElectromagneticLock - Single electromagnetic lock
 * 
 * Electromagnetic locks work on fail-safe principle:
 * - Power ON = Lock engaged (door sealed)
 * - Power OFF = Lock released (door opens)
 */
struct ElectromagneticLock {
    char lock_id[MAX_LOCK_ID_LEN];
    char location[MAX_LOCATION_LEN];
    bool is_powered;
    LockState state;
    lock_state_callback_t state_callbacks[MAX_LOCK_STATE_CALLBACKS];
    void *callback_contexts[MAX_LOCK_STATE_CALLBACKS];
    int callback_count;
};

/* Initialize a lock */
void lock_init(ElectromagneticLock *lock, const char *lock_id, const char *location);

/* Power control */
void lock_power_on(ElectromagneticLock *lock);
void lock_power_off(ElectromagneticLock *lock);

/* Trigger failsafe (force unlock) */
void lock_trigger_failsafe(ElectromagneticLock *lock);

/* Set malfunction state */
void lock_set_malfunction(ElectromagneticLock *lock);

/* State queries */
bool lock_is_locked(const ElectromagneticLock *lock);
bool lock_is_unlocked(const ElectromagneticLock *lock);
bool lock_is_powered(const ElectromagneticLock *lock);
LockState lock_get_state(const ElectromagneticLock *lock);

/* Register state change callback */
bool lock_register_state_callback(ElectromagneticLock *lock,
                                  lock_state_callback_t cb, void *ctx);

/**
 * LockSystem - Manages multiple electromagnetic locks
 */
struct LockSystem {
    char name[MAX_LOCATION_LEN];
    ElectromagneticLock locks[MAX_LOCKS_IN_SYSTEM];
    int lock_count;
    bool failsafe_triggered;
    lock_system_failsafe_callback_t failsafe_callbacks[MAX_FAILSAFE_CALLBACKS];
    void *failsafe_callback_contexts[MAX_FAILSAFE_CALLBACKS];
    int failsafe_callback_count;
};

/* Initialize lock system */
void lock_system_init(LockSystem *system, const char *name);

/* Add a lock to the system */
bool lock_system_add_lock(LockSystem *system, ElectromagneticLock *lock);

/* Create and add a new lock */
ElectromagneticLock *lock_system_create_lock(LockSystem *system, 
                                             const char *lock_id, 
                                             const char *location);

/* Get lock by ID */
ElectromagneticLock *lock_system_get_lock(LockSystem *system, const char *lock_id);

/* Get lock count */
int lock_system_get_lock_count(const LockSystem *system);

/* Count locked/unlocked locks */
int lock_system_get_locked_count(const LockSystem *system);
int lock_system_get_unlocked_count(const LockSystem *system);

/* Check if all locks are in specific state */
bool lock_system_all_locked(const LockSystem *system);
bool lock_system_all_unlocked(const LockSystem *system);

/* Trigger global failsafe (unlock all), returns number unlocked */
int lock_system_trigger_global_failsafe(LockSystem *system);

/* Power control for all locks */
void lock_system_power_on_all(LockSystem *system);
void lock_system_power_off_all(LockSystem *system);

/* Reset system */
void lock_system_reset(LockSystem *system);

/* Check if failsafe was triggered */
bool lock_system_is_failsafe_triggered(const LockSystem *system);

/* Register failsafe callback */
bool lock_system_register_failsafe_callback(LockSystem *system,
                                            lock_system_failsafe_callback_t cb,
                                            void *ctx);

#endif /* ELECTROMAGNETIC_LOCK_H */
