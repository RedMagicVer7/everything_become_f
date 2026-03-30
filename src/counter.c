/**
 * counter.c - 16-bit Unsigned Short Integer Counter Implementation
 */

#include <stdio.h>
#include <string.h>
#include "../include/counter.h"

void counter_init(UnsignedShortCounter *counter) {
    if (!counter) return;
    
    counter->value = 0;
    counter->overflow_count = 0;
    counter->total_increments = 0;
    counter->callback_count = 0;
    
    for (int i = 0; i < MAX_OVERFLOW_CALLBACKS; i++) {
        counter->callbacks[i] = NULL;
        counter->callback_contexts[i] = NULL;
    }
}

bool counter_init_with_value(UnsignedShortCounter *counter, uint16_t initial_value) {
    if (!counter) return false;
    
    counter_init(counter);
    counter->value = initial_value;
    return true;
}

/* Trigger all overflow callbacks */
static void trigger_overflow_callbacks(UnsignedShortCounter *counter) {
    for (int i = 0; i < counter->callback_count; i++) {
        if (counter->callbacks[i]) {
            counter->callbacks[i](counter, counter->callback_contexts[i]);
        }
    }
}

bool counter_increment(UnsignedShortCounter *counter) {
    return counter_increment_by(counter, 1);
}

bool counter_increment_by(UnsignedShortCounter *counter, uint16_t amount) {
    if (!counter) return false;
    
    counter->total_increments += amount;
    uint32_t new_value = (uint32_t)counter->value + amount;
    
    if (new_value > COUNTER_MAX_VALUE) {
        /* Overflow occurs - use modular arithmetic */
        counter->value = (uint16_t)(new_value & COUNTER_MAX_VALUE);
        counter->overflow_count++;
        trigger_overflow_callbacks(counter);
        return true;
    }
    
    counter->value = (uint16_t)new_value;
    return false;
}

uint16_t counter_get_value(const UnsignedShortCounter *counter) {
    return counter ? counter->value : 0;
}

void counter_get_hex(const UnsignedShortCounter *counter, char *buf, size_t len) {
    if (!counter || !buf || len < 7) return;
    snprintf(buf, len, "0x%04X", counter->value);
}

uint32_t counter_hours_until_overflow(const UnsignedShortCounter *counter) {
    if (!counter) return 0;
    return (uint32_t)(COUNTER_MAX_VALUE - counter->value + 1);
}

double counter_years_until_overflow(const UnsignedShortCounter *counter) {
    return (double)counter_hours_until_overflow(counter) / HOURS_PER_YEAR;
}

uint32_t counter_fast_forward(UnsignedShortCounter *counter, uint32_t hours) {
    if (!counter || hours == 0) return 0;
    
    counter->total_increments += hours;
    uint32_t new_value = (uint32_t)counter->value + hours;
    
    if (new_value <= COUNTER_MAX_VALUE) {
        counter->value = (uint16_t)new_value;
        return 0;
    }
    
    /* Calculate overflows using division - O(1) */
    uint32_t overflows = new_value / ((uint32_t)COUNTER_MAX_VALUE + 1);
    counter->value = (uint16_t)(new_value % ((uint32_t)COUNTER_MAX_VALUE + 1));
    counter->overflow_count += overflows;
    
    /* Trigger callbacks for each overflow */
    for (uint32_t i = 0; i < overflows; i++) {
        trigger_overflow_callbacks(counter);
    }
    
    return overflows;
}

bool counter_register_overflow_callback(UnsignedShortCounter *counter,
                                        overflow_callback_t cb, void *ctx) {
    if (!counter || !cb) return false;
    if (counter->callback_count >= MAX_OVERFLOW_CALLBACKS) return false;
    
    counter->callbacks[counter->callback_count] = cb;
    counter->callback_contexts[counter->callback_count] = ctx;
    counter->callback_count++;
    return true;
}

bool counter_unregister_overflow_callback(UnsignedShortCounter *counter,
                                          overflow_callback_t cb) {
    if (!counter || !cb) return false;
    
    for (int i = 0; i < counter->callback_count; i++) {
        if (counter->callbacks[i] == cb) {
            /* Shift remaining callbacks */
            for (int j = i; j < counter->callback_count - 1; j++) {
                counter->callbacks[j] = counter->callbacks[j + 1];
                counter->callback_contexts[j] = counter->callback_contexts[j + 1];
            }
            counter->callback_count--;
            return true;
        }
    }
    return false;
}

void counter_set_value(UnsignedShortCounter *counter, uint16_t value) {
    if (counter) {
        counter->value = value;
    }
}

void counter_reset(UnsignedShortCounter *counter) {
    if (counter) {
        counter->value = 0;
    }
}

bool counter_is_at_max(const UnsignedShortCounter *counter) {
    return counter && counter->value == COUNTER_MAX_VALUE;
}

bool counter_is_at_zero(const UnsignedShortCounter *counter) {
    return counter && counter->value == 0;
}

uint32_t counter_get_overflow_count(const UnsignedShortCounter *counter) {
    return counter ? counter->overflow_count : 0;
}

uint32_t counter_get_total_increments(const UnsignedShortCounter *counter) {
    return counter ? counter->total_increments : 0;
}
