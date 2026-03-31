#!/bin/bash
# Phase 5: Global Emergence - Test Runner
# 真贺田四季 フェーズ5: グローバル創発 - テスト

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$SCRIPT_DIR"

echo "╔═══════════════════════════════════════════════════════════╗"
echo "║     Testing Shiki Phase 5: Global Emergence               ║"
echo "║     真贺田四季 フェーズ5: グローバル創発 テスト           ║"
echo "╚═══════════════════════════════════════════════════════════╝"
echo ""

# Check if cargo is available
if ! command -v cargo &> /dev/null; then
    echo "Error: Rust/Cargo not found. Please install Rust."
    exit 1
fi

# Parse arguments
RUN_L1=true
RUN_L2=true
RUN_L3=true
VERBOSE=false

for arg in "$@"; do
    case $arg in
        --l1)
            RUN_L1=true
            RUN_L2=false
            RUN_L3=false
            ;;
        --l2)
            RUN_L1=false
            RUN_L2=true
            RUN_L3=false
            ;;
        --l3)
            RUN_L1=false
            RUN_L2=false
            RUN_L3=true
            ;;
        --verbose|-v)
            VERBOSE=true
            ;;
        *)
            ;;
    esac
done

CARGO_OPTS=""
if [ "$VERBOSE" = true ]; then
    CARGO_OPTS="-- --nocapture"
fi

TOTAL_PASSED=0
TOTAL_FAILED=0

# Run L1 Unit Tests
if [ "$RUN_L1" = true ]; then
    echo "══════════════════════════════════════════════════════════════"
    echo "                    L1 UNIT TESTS"
    echo "══════════════════════════════════════════════════════════════"
    if cargo test --test l1_unit_tests $CARGO_OPTS; then
        echo "[L1] All unit tests passed!"
        ((TOTAL_PASSED+=1))
    else
        echo "[L1] Some unit tests failed!"
        ((TOTAL_FAILED+=1))
    fi
    echo ""
fi

# Run L2 Integration Tests
if [ "$RUN_L2" = true ]; then
    echo "══════════════════════════════════════════════════════════════"
    echo "                  L2 INTEGRATION TESTS"
    echo "══════════════════════════════════════════════════════════════"
    if cargo test --test l2_integration_tests $CARGO_OPTS; then
        echo "[L2] All integration tests passed!"
        ((TOTAL_PASSED+=1))
    else
        echo "[L2] Some integration tests failed!"
        ((TOTAL_FAILED+=1))
    fi
    echo ""
fi

# Run L3 System Tests
if [ "$RUN_L3" = true ]; then
    echo "══════════════════════════════════════════════════════════════"
    echo "                    L3 SYSTEM TESTS"
    echo "══════════════════════════════════════════════════════════════"
    if cargo test --test l3_system_tests $CARGO_OPTS; then
        echo "[L3] All system tests passed!"
        ((TOTAL_PASSED+=1))
    else
        echo "[L3] Some system tests failed!"
        ((TOTAL_FAILED+=1))
    fi
    echo ""
fi

# Summary
echo "══════════════════════════════════════════════════════════════"
echo "                      TEST SUMMARY"
echo "══════════════════════════════════════════════════════════════"
echo "Test suites passed: $TOTAL_PASSED"
echo "Test suites failed: $TOTAL_FAILED"
echo ""

if [ $TOTAL_FAILED -eq 0 ]; then
    echo "╔═══════════════════════════════════════════════════════════╗"
    echo "║              ALL TESTS PASSED SUCCESSFULLY                ║"
    echo "║                                                           ║"
    echo "║     Shiki Phase 5: Global Emergence verified              ║"
    echo "║     グローバル創発の検証完了                              ║"
    echo "╚═══════════════════════════════════════════════════════════╝"
    exit 0
else
    echo "╔═══════════════════════════════════════════════════════════╗"
    echo "║                 SOME TESTS FAILED                         ║"
    echo "╚═══════════════════════════════════════════════════════════╝"
    exit 1
fi
