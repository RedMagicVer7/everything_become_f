/**
 * counter.h - 16-bit Unsigned Integer Counter
 * ============================================
 * 
 * Simulates the C language unsigned short (0-65535) behavior.
 * This is the core of Dr. Magata's escape mechanism.
 * 
 * 0xFFFF = "全部成为F" (Everything Becomes F)
 * - 4 hex digits = 4 F's = symbolic perfection
 * - Counter increments every 2 hours
 * - 65535 × 2h ÷ 24 ÷ 365.25 ≈ 15 years to overflow
 * 
 * When the counter reaches 0xFFFF (65535) and increments by 1,
 * it overflows back to 0x0000, triggering the failsafe mechanism.
 */

#ifndef COUNTER_H
#define COUNTER_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#define COUNTER_MAX_VALUE 0xFFFF
#define INTERVALS_PER_YEAR 4383.0  /* 365.25 * 24 / 2 = intervals per year (2h each) */
#define MAX_OVERFLOW_CALLBACKS 8

/* Forward declaration */
typedef struct UnsignedShortCounter UnsignedShortCounter;

/* Callback type for overflow events */
typedef void (*overflow_callback_t)(UnsignedShortCounter *counter, void *context);

/**
 * UnsignedShortCounter - Simulates C unsigned short (0-65535)
 * 
 * The counter increments every 2 hours. After 65535 intervals (~15 years),
 * overflow occurs and the value wraps to zero.
 * 0xFFFF = 4 F's = "Everything Becomes F"
 */
struct UnsignedShortCounter {
    uint16_t value;              /* unsigned short, 0-65535 (0x0000-0xFFFF) */
    uint32_t overflow_count;     /* Number of overflows (can exceed 16-bit) */
    uint64_t total_increments;   /* Total increments performed */
    overflow_callback_t callbacks[MAX_OVERFLOW_CALLBACKS];
    void *callback_contexts[MAX_OVERFLOW_CALLBACKS];
    int callback_count;
};

/* Initialize counter to 0 */
void counter_init(UnsignedShortCounter *counter);

/* Initialize counter with specific value */
bool counter_init_with_value(UnsignedShortCounter *counter, uint16_t initial_value);

/* Increment counter by 1, returns true if overflow occurred */
bool counter_increment(UnsignedShortCounter *counter);

/* Increment counter by amount, returns true if overflow occurred */
bool counter_increment_by(UnsignedShortCounter *counter, uint16_t amount);

/* Get current counter value */
uint16_t counter_get_value(const UnsignedShortCounter *counter);

/* Get counter value as hex string (writes to buf, e.g., "0xFFFF") */
void counter_get_hex(const UnsignedShortCounter *counter, char *buf, size_t len);

/* Get intervals (2-hour periods) until next overflow */
uint32_t counter_intervals_until_overflow(const UnsignedShortCounter *counter);

/* Get years until next overflow */
double counter_years_until_overflow(const UnsignedShortCounter *counter);

/* Fast forward by specified intervals, returns number of overflows */
uint32_t counter_fast_forward(UnsignedShortCounter *counter, uint64_t intervals);

/* Register callback for overflow events */
bool counter_register_overflow_callback(UnsignedShortCounter *counter, 
                                        overflow_callback_t cb, void *ctx);

/* Unregister overflow callback */
bool counter_unregister_overflow_callback(UnsignedShortCounter *counter,
                                          overflow_callback_t cb);

/* Set counter to specific value (for testing) */
void counter_set_value(UnsignedShortCounter *counter, uint16_t value);

/* Reset counter to zero */
void counter_reset(UnsignedShortCounter *counter);

/* Check if counter is at max value (0xFFFF) */
bool counter_is_at_max(const UnsignedShortCounter *counter);

/* Check if counter is at zero */
bool counter_is_at_zero(const UnsignedShortCounter *counter);

/* Get overflow count */
uint32_t counter_get_overflow_count(const UnsignedShortCounter *counter);

/* Get total increments */
uint64_t counter_get_total_increments(const UnsignedShortCounter *counter);

#endif /* COUNTER_H */
