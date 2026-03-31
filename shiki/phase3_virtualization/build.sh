#!/bin/bash
# Phase 3 Virtualization - Build Script
# =====================================
# Compiles and runs the Phase 3 Java simulation

cd "$(dirname "$0")"

echo "============================================"
echo "  Phase 3: Virtualization - Build"
echo "  フェーズ3: 仮想化 - ビルド"
echo "============================================"
echo ""

# Create build directory
mkdir -p build

echo "Compiling Phase 3 (Java)..."
echo ""

# Compile all main source files
javac -d build -sourcepath src/main/java src/main/java/shiki/phase3/*.java

if [ $? -ne 0 ]; then
    echo ""
    echo "============================================"
    echo "  COMPILATION FAILED"
    echo "============================================"
    exit 1
fi

echo "Compilation successful!"
echo ""

# Run main program
echo "============================================"
echo "  Running Phase 3 Simulation"
echo "============================================"
echo ""

java -cp build shiki.phase3.Phase3Main "$@"

exit $?
