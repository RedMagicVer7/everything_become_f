#!/bin/bash
# Phase 3 Virtualization - Test Script
# =====================================
# Compiles and runs all Phase 3 tests

cd "$(dirname "$0")"

echo "============================================"
echo "  Phase 3: Virtualization - Tests"
echo "  フェーズ3: 仮想化 - テスト"
echo "============================================"
echo ""

# Create build directory
mkdir -p build

echo "Compiling Phase 3 sources and tests..."
echo ""

# Compile main sources first
javac -d build -sourcepath src/main/java src/main/java/shiki/phase3/*.java

if [ $? -ne 0 ]; then
    echo ""
    echo "============================================"
    echo "  MAIN SOURCE COMPILATION FAILED"
    echo "============================================"
    exit 1
fi

# Compile test sources
javac -d build -cp build -sourcepath src/test/java src/test/java/shiki/phase3/*.java

if [ $? -ne 0 ]; then
    echo ""
    echo "============================================"
    echo "  TEST COMPILATION FAILED"
    echo "============================================"
    exit 1
fi

echo "Compilation successful!"
echo ""

# Run tests
java -cp build shiki.phase3.Phase3TestRunner

exit $?
