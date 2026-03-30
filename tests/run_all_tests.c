/**
 * run_all_tests.c - Test Runner
 * ===============================
 * 
 * Runs all test suites (L1, L2, L3) and reports results.
 */

#include <stdio.h>

/* External test suite runners */
extern int run_l1_tests(void);
extern int run_l2_tests(void);
extern int run_l3_tests(void);

int main(void) {
    int total_failures = 0;
    
    printf("============================================\n");
    printf("   Everything Becomes F - Test Suite\n");
    printf("   すべてがFになる - テストスイート\n");
    printf("============================================\n");
    
    /* Run L1 Unit Tests */
    total_failures += run_l1_tests();
    
    /* Run L2 Integration Tests */
    total_failures += run_l2_tests();
    
    /* Run L3 System Tests */
    total_failures += run_l3_tests();
    
    /* Final summary */
    printf("\n============================================\n");
    printf("           FINAL TEST RESULTS\n");
    printf("============================================\n");
    
    if (total_failures == 0) {
        printf("\n  ALL TESTS PASSED!\n\n");
        printf("  Everything has become F.\n");
        printf("  すべてがFになる。\n\n");
    } else {
        printf("\n  %d TEST(S) FAILED!\n\n", total_failures);
    }
    
    printf("============================================\n");
    
    return total_failures > 0 ? 1 : 0;
}
