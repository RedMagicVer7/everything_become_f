/**
 * electromagnetic_lock.c - Electromagnetic Lock Control System Implementation
 */

#include <string.h>
#include "../include/electromagnetic_lock.h"

/* ============================================================================
 * ElectromagneticLock Implementation
 * ============================================================================ */

static void notify_lock_state_change(ElectromagneticLock *lock,
                                     LockState old_state, LockState new_state) {
    for (int i = 0; i < lock->callback_count; i++) {
        if (lock->state_callbacks[i]) {
            lock->state_callbacks[i](lock, old_state, new_state,
                                     lock->callback_contexts[i]);
        }
    }
}

void lock_init(ElectromagneticLock *lock, const char *lock_id, const char *location) {
    if (!lock) return;
    
    memset(lock, 0, sizeof(ElectromagneticLock));
    
    if (lock_id) {
        strncpy(lock->lock_id, lock_id, MAX_LOCK_ID_LEN - 1);
    }
    if (location) {
        strncpy(lock->location, location, MAX_LOCATION_LEN - 1);
    }
    
    lock->is_powered = true;
    lock->state = LOCK_STATE_LOCKED;
    lock->callback_count = 0;
}

void lock_power_on(ElectromagneticLock *lock) {
    if (!lock) return;
    
    lock->is_powered = true;
    LockState old_state = lock->state;
    lock->state = LOCK_STATE_LOCKED;
    
    if (old_state != lock->state) {
        notify_lock_state_change(lock, old_state, lock->state);
    }
}

void lock_power_off(ElectromagneticLock *lock) {
    if (!lock) return;
    
    lock->is_powered = false;
    LockState old_state = lock->state;
    lock->state = LOCK_STATE_UNLOCKED;
    
    if (old_state != lock->state) {
        notify_lock_state_change(lock, old_state, lock->state);
    }
}

void lock_trigger_failsafe(ElectromagneticLock *lock) {
    if (!lock) return;
    
    LockState old_state = lock->state;
    lock->is_powered = false;
    lock->state = LOCK_STATE_UNLOCKED;
    
    if (old_state != lock->state) {
        notify_lock_state_change(lock, old_state, lock->state);
    }
}

void lock_set_malfunction(ElectromagneticLock *lock) {
    if (!lock) return;
    
    LockState old_state = lock->state;
    lock->state = LOCK_STATE_MALFUNCTIONING;
    
    if (old_state != lock->state) {
        notify_lock_state_change(lock, old_state, lock->state);
    }
}

bool lock_is_locked(const ElectromagneticLock *lock) {
    return lock && lock->state == LOCK_STATE_LOCKED;
}

bool lock_is_unlocked(const ElectromagneticLock *lock) {
    return lock && lock->state == LOCK_STATE_UNLOCKED;
}

bool lock_is_powered(const ElectromagneticLock *lock) {
    return lock && lock->is_powered;
}

LockState lock_get_state(const ElectromagneticLock *lock) {
    return lock ? lock->state : LOCK_STATE_LOCKED;
}

bool lock_register_state_callback(ElectromagneticLock *lock,
                                  lock_state_callback_t cb, void *ctx) {
    if (!lock || !cb) return false;
    if (lock->callback_count >= MAX_LOCK_STATE_CALLBACKS) return false;
    
    lock->state_callbacks[lock->callback_count] = cb;
    lock->callback_contexts[lock->callback_count] = ctx;
    lock->callback_count++;
    return true;
}

/* ============================================================================
 * LockSystem Implementation
 * ============================================================================ */

static void notify_failsafe_callbacks(LockSystem *system) {
    for (int i = 0; i < system->failsafe_callback_count; i++) {
        if (system->failsafe_callbacks[i]) {
            system->failsafe_callbacks[i](system,
                                          system->failsafe_callback_contexts[i]);
        }
    }
}

void lock_system_init(LockSystem *system, const char *name) {
    if (!system) return;
    
    memset(system, 0, sizeof(LockSystem));
    
    if (name) {
        strncpy(system->name, name, MAX_LOCATION_LEN - 1);
    }
    
    system->lock_count = 0;
    system->failsafe_triggered = false;
    system->failsafe_callback_count = 0;
}

bool lock_system_add_lock(LockSystem *system, ElectromagneticLock *lock) {
    if (!system || !lock) return false;
    if (system->lock_count >= MAX_LOCKS_IN_SYSTEM) return false;
    
    /* Copy lock to system's array */
    system->locks[system->lock_count] = *lock;
    system->lock_count++;
    return true;
}

ElectromagneticLock *lock_system_create_lock(LockSystem *system,
                                             const char *lock_id,
                                             const char *location) {
    if (!system) return NULL;
    if (system->lock_count >= MAX_LOCKS_IN_SYSTEM) return NULL;
    
    ElectromagneticLock *lock = &system->locks[system->lock_count];
    lock_init(lock, lock_id, location);
    system->lock_count++;
    return lock;
}

ElectromagneticLock *lock_system_get_lock(LockSystem *system, const char *lock_id) {
    if (!system || !lock_id) return NULL;
    
    for (int i = 0; i < system->lock_count; i++) {
        if (strcmp(system->locks[i].lock_id, lock_id) == 0) {
            return &system->locks[i];
        }
    }
    return NULL;
}

int lock_system_get_lock_count(const LockSystem *system) {
    return system ? system->lock_count : 0;
}

int lock_system_get_locked_count(const LockSystem *system) {
    if (!system) return 0;
    
    int count = 0;
    for (int i = 0; i < system->lock_count; i++) {
        if (lock_is_locked(&system->locks[i])) {
            count++;
        }
    }
    return count;
}

int lock_system_get_unlocked_count(const LockSystem *system) {
    if (!system) return 0;
    
    int count = 0;
    for (int i = 0; i < system->lock_count; i++) {
        if (lock_is_unlocked(&system->locks[i])) {
            count++;
        }
    }
    return count;
}

bool lock_system_all_locked(const LockSystem *system) {
    if (!system || system->lock_count == 0) return false;
    
    for (int i = 0; i < system->lock_count; i++) {
        if (!lock_is_locked(&system->locks[i])) {
            return false;
        }
    }
    return true;
}

bool lock_system_all_unlocked(const LockSystem *system) {
    if (!system || system->lock_count == 0) return false;
    
    for (int i = 0; i < system->lock_count; i++) {
        if (!lock_is_unlocked(&system->locks[i])) {
            return false;
        }
    }
    return true;
}

int lock_system_trigger_global_failsafe(LockSystem *system) {
    if (!system) return 0;
    
    system->failsafe_triggered = true;
    int unlocked_count = 0;
    
    for (int i = 0; i < system->lock_count; i++) {
        if (lock_is_locked(&system->locks[i])) {
            lock_trigger_failsafe(&system->locks[i]);
            unlocked_count++;
        }
    }
    
    notify_failsafe_callbacks(system);
    return unlocked_count;
}

void lock_system_power_on_all(LockSystem *system) {
    if (!system) return;
    
    for (int i = 0; i < system->lock_count; i++) {
        lock_power_on(&system->locks[i]);
    }
}

void lock_system_power_off_all(LockSystem *system) {
    if (!system) return;
    
    for (int i = 0; i < system->lock_count; i++) {
        lock_power_off(&system->locks[i]);
    }
}

void lock_system_reset(LockSystem *system) {
    if (!system) return;
    
    system->failsafe_triggered = false;
    lock_system_power_on_all(system);
}

bool lock_system_is_failsafe_triggered(const LockSystem *system) {
    return system && system->failsafe_triggered;
}

bool lock_system_register_failsafe_callback(LockSystem *system,
                                            lock_system_failsafe_callback_t cb,
                                            void *ctx) {
    if (!system || !cb) return false;
    if (system->failsafe_callback_count >= MAX_FAILSAFE_CALLBACKS) return false;
    
    system->failsafe_callbacks[system->failsafe_callback_count] = cb;
    system->failsafe_callback_contexts[system->failsafe_callback_count] = ctx;
    system->failsafe_callback_count++;
    return true;
}
