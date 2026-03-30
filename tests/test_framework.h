/**
 * test_framework.h - Minimal Unit Test Framework
 * ================================================
 * 
 * A simple, header-only test framework for C.
 * No external dependencies required.
 */

#ifndef TEST_FRAMEWORK_H
#define TEST_FRAMEWORK_H

#include <stdio.h>
#include <string.h>
#include <stdbool.h>

/* Test counters - must be defined in test file */
static int _tests_run = 0;
static int _tests_passed = 0;
static int _tests_failed = 0;
static int _current_test_failed = 0;

/* Reset test state for new test suite */
#define TEST_SUITE_BEGIN() do { \
    _tests_run = 0; \
    _tests_passed = 0; \
    _tests_failed = 0; \
} while(0)

/* Define a test function */
#define TEST(name) static void name(void)

/* Run a test */
#define RUN_TEST(name) do { \
    _tests_run++; \
    _current_test_failed = 0; \
    printf("  %-50s ", #name); \
    fflush(stdout); \
    name(); \
    if (_current_test_failed) { \
        _tests_failed++; \
        printf("FAIL\n"); \
    } else { \
        _tests_passed++; \
        printf("PASS\n"); \
    } \
} while(0)

/* Assertion macros */
#define ASSERT_TRUE(expr) do { \
    if (!(expr)) { \
        printf("\n    ASSERT_TRUE failed: %s\n    at %s:%d\n", #expr, __FILE__, __LINE__); \
        _current_test_failed = 1; \
        return; \
    } \
} while(0)

#define ASSERT_FALSE(expr) do { \
    if (expr) { \
        printf("\n    ASSERT_FALSE failed: %s\n    at %s:%d\n", #expr, __FILE__, __LINE__); \
        _current_test_failed = 1; \
        return; \
    } \
} while(0)

#define ASSERT_EQ(a, b) do { \
    if ((a) != (b)) { \
        printf("\n    ASSERT_EQ failed: %s != %s\n    at %s:%d\n", #a, #b, __FILE__, __LINE__); \
        _current_test_failed = 1; \
        return; \
    } \
} while(0)

#define ASSERT_NEQ(a, b) do { \
    if ((a) == (b)) { \
        printf("\n    ASSERT_NEQ failed: %s == %s\n    at %s:%d\n", #a, #b, __FILE__, __LINE__); \
        _current_test_failed = 1; \
        return; \
    } \
} while(0)

#define ASSERT_GT(a, b) do { \
    if (!((a) > (b))) { \
        printf("\n    ASSERT_GT failed: %s <= %s\n    at %s:%d\n", #a, #b, __FILE__, __LINE__); \
        _current_test_failed = 1; \
        return; \
    } \
} while(0)

#define ASSERT_LT(a, b) do { \
    if (!((a) < (b))) { \
        printf("\n    ASSERT_LT failed: %s >= %s\n    at %s:%d\n", #a, #b, __FILE__, __LINE__); \
        _current_test_failed = 1; \
        return; \
    } \
} while(0)

#define ASSERT_GTE(a, b) do { \
    if (!((a) >= (b))) { \
        printf("\n    ASSERT_GTE failed: %s < %s\n    at %s:%d\n", #a, #b, __FILE__, __LINE__); \
        _current_test_failed = 1; \
        return; \
    } \
} while(0)

#define ASSERT_LTE(a, b) do { \
    if (!((a) <= (b))) { \
        printf("\n    ASSERT_LTE failed: %s > %s\n    at %s:%d\n", #a, #b, __FILE__, __LINE__); \
        _current_test_failed = 1; \
        return; \
    } \
} while(0)

#define ASSERT_STR_EQ(a, b) do { \
    if (strcmp(a, b) != 0) { \
        printf("\n    ASSERT_STR_EQ failed: \"%s\" != \"%s\"\n    at %s:%d\n", a, b, __FILE__, __LINE__); \
        _current_test_failed = 1; \
        return; \
    } \
} while(0)

#define ASSERT_NULL(ptr) do { \
    if ((ptr) != NULL) { \
        printf("\n    ASSERT_NULL failed: %s is not NULL\n    at %s:%d\n", #ptr, __FILE__, __LINE__); \
        _current_test_failed = 1; \
        return; \
    } \
} while(0)

#define ASSERT_NOT_NULL(ptr) do { \
    if ((ptr) == NULL) { \
        printf("\n    ASSERT_NOT_NULL failed: %s is NULL\n    at %s:%d\n", #ptr, __FILE__, __LINE__); \
        _current_test_failed = 1; \
        return; \
    } \
} while(0)

/* Print test summary */
#define TEST_SUMMARY() do { \
    printf("\n=== %d tests run, %d passed, %d failed ===\n", \
        _tests_run, _tests_passed, _tests_failed); \
} while(0)

/* Get test results */
#define TEST_GET_FAILED() (_tests_failed)
#define TEST_GET_PASSED() (_tests_passed)
#define TEST_GET_RUN() (_tests_run)

#endif /* TEST_FRAMEWORK_H */
