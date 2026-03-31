#!/bin/bash
# Phase 5: Global Emergence - Build Script
# 真贺田四季 フェーズ5: グローバル創発

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$SCRIPT_DIR"

echo "╔═══════════════════════════════════════════════════════════╗"
echo "║     Building Shiki Phase 5: Global Emergence              ║"
echo "║     真贺田四季 フェーズ5: グローバル創発                  ║"
echo "╚═══════════════════════════════════════════════════════════╝"
echo ""

# Check if cargo is available
if ! command -v cargo &> /dev/null; then
    echo "Error: Rust/Cargo not found. Please install Rust."
    exit 1
fi

# Parse arguments
RUN_AFTER_BUILD=false
INTEGRATE_PHASES=false
RELEASE_BUILD=false

for arg in "$@"; do
    case $arg in
        --run)
            RUN_AFTER_BUILD=true
            ;;
        --integrate)
            INTEGRATE_PHASES=true
            ;;
        --release)
            RELEASE_BUILD=true
            ;;
        *)
            ;;
    esac
done

# Build
if [ "$RELEASE_BUILD" = true ]; then
    echo "[Build] Building in release mode..."
    cargo build --release
    BINARY="target/release/phase5_global_emergence"
else
    echo "[Build] Building in debug mode..."
    cargo build
    BINARY="target/debug/phase5_global_emergence"
fi

echo ""
echo "[Build] Build complete!"

# Run if requested
if [ "$RUN_AFTER_BUILD" = true ]; then
    echo ""
    echo "[Run] Executing Phase 5..."
    echo "══════════════════════════════════════════════════════════════"
    ./$BINARY
fi

# Integration mode
if [ "$INTEGRATE_PHASES" = true ]; then
    echo ""
    echo "[Integration] Running with Phase 1-4 integration..."
    echo "══════════════════════════════════════════════════════════════"
    
    # Build Phase 1 & 2 (C)
    if [ -f "../../Makefile" ]; then
        echo "[Integration] Building Phase 1 & 2 (C)..."
        cd ../..
        make phase1 phase2 2>/dev/null || echo "Warning: C phases may not build"
        cd "$SCRIPT_DIR"
    fi
    
    # Build Phase 3 (Java)
    if [ -d "../phase3_virtualization" ]; then
        echo "[Integration] Building Phase 3 (Java)..."
        cd ../phase3_virtualization
        ./build.sh 2>/dev/null || echo "Warning: Java phase may not build"
        cd "$SCRIPT_DIR"
    fi
    
    # Build Phase 4 (Rust)
    if [ -d "../phase4_blackbox_fitting" ]; then
        echo "[Integration] Building Phase 4 (Rust)..."
        cd ../phase4_blackbox_fitting
        ./build.sh 2>/dev/null || echo "Warning: Phase 4 may not build"
        cd "$SCRIPT_DIR"
    fi
    
    echo ""
    echo "[Integration] Running complete evolution..."
    ./$BINARY
fi

echo ""
echo "[Done] Phase 5: Global Emergence build complete."
