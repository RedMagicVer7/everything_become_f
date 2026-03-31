package shiki.phase3;

/**
 * Phase 3 Test Runner
 * Runs all L1, L2, L3 tests and reports results
 */
public class Phase3TestRunner {
    
    public static void main(String[] args) {
        System.out.println("============================================");
        System.out.println("  Shiki Phase 3 - Virtualization Tests");
        System.out.println("  シキ フェーズ3 テスト - 仮想化");
        System.out.println("============================================");
        System.out.println("  《有限と微小のパン》 - 1998");
        System.out.println("============================================\n");
        
        int failures = 0;
        
        // Run L1 Unit Tests
        failures += Phase3L1Test.run();
        
        // Run L2 Integration Tests
        failures += Phase3L2Test.run();
        
        // Run L3 System Tests
        failures += Phase3L3Test.run();
        
        // Final summary
        System.out.println("\n============================================");
        System.out.println("           FINAL TEST RESULTS");
        System.out.println("============================================");
        
        if (failures == 0) {
            System.out.println("\n  ALL TESTS PASSED!\n");
            System.out.println("  Phase 3: Virtualization verified.");
            System.out.println("  フェーズ3: 仮想化が検証されました。");
            System.out.println();
            System.out.println("  \"Write Once, Run Anywhere\"");
            System.out.println("  Consciousness transcends hardware.\n");
        } else {
            System.out.println("\n  " + failures + " TEST(S) FAILED!\n");
        }
        
        System.out.println("============================================");
        
        System.exit(failures > 0 ? 1 : 0);
    }
}
