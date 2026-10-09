#!/usr/bin/env bash

# Exit immediately if a command exits with a non-zero status
set -e

# Check if two commit hashes are provided as arguments
if [ "$#" -ne 2 ]; then
    echo "Usage: $0 <commit_hash_base> <commit_hash_new>"
    exit 1
fi

COMMIT_BASE="$1"
COMMIT_NEW="$2"

# --- CONFIGURATION ---
BINARY_NAME="slychess"
BUILD_BASE_DIR="../sprt_slychess/build_base"
BUILD_NEW_DIR="../sprt_slychess/build_new"

# Save the current git state (branch or detached HEAD) to restore later
CURRENT_STATE=$(git symbolic-ref --short HEAD 2>/dev/null || git rev-parse HEAD)

# Error handling function to restore git state on failure
cleanup_on_error() {
    echo "Error encountered. Aborting script."
    if [ -n "$CURRENT_STATE" ]; then
        git checkout "$CURRENT_STATE" >/dev/null 2>&1 || true
    fi
    exit 1
}

# Trap errors to run cleanup
trap cleanup_on_error ERR

echo "=== Stashing any uncommitted local changes..."
git stash

# --- 1. BUILD BASE ENGINE ---
echo "=== Checking out base commit: $COMMIT_BASE"
git checkout "$COMMIT_BASE"

echo "=== Configuring and compiling base engine..."
mkdir -p "$BUILD_BASE_DIR"
cmake -S . -B "$BUILD_BASE_DIR" -DCMAKE_C_COMPILER=gcc -DCMAKE_CXX_COMPILER=g++ -DCMAKE_BUILD_TYPE=Release
cmake --build "$BUILD_BASE_DIR" --config Release -j "$(nproc)"

# --- 2. BUILD NEW ENGINE ---
echo "=== Checking out new commit: $COMMIT_NEW"
git checkout "$COMMIT_NEW"

echo "=== Configuring and compiling new engine..."
mkdir -p "$BUILD_NEW_DIR"
cmake -S . -B "$BUILD_NEW_DIR" -DCMAKE_C_COMPILER=gcc -DCMAKE_CXX_COMPILER=g++ -DCMAKE_BUILD_TYPE=Release
cmake --build "$BUILD_NEW_DIR" --config Release -j "$(nproc)"

# --- 3. RESTORE REPO STATE ---
echo "=== Restoring original git state ($CURRENT_STATE)..."
git checkout "$CURRENT_STATE"
git stash pop >/dev/null 2>&1 || echo "Note: No stash to pop or conflict handled."

# Remove error trap for normal completion
trap - ERR

# --- 4. RUN SPRT TEST ---
echo "=== Starting fastchess SPRT test..."
fastchess \
    -engine cmd="./$BUILD_NEW_DIR/$BINARY_NAME" name="New-$COMMIT_NEW" \
    -engine cmd="./$BUILD_BASE_DIR/$BINARY_NAME" name="Base-$COMMIT_BASE" \
    -pgnout file="../sprt_slychess/sprt_results.pgn" \
    -openings file="../sprt_slychess/books/8moves_v3.pgn" format=pgn order=random \
    -each tc=5.0+0.1 -repeat -concurrency 6 -recover \
    -sprt elo0=0 elo1=10 alpha=0.05 beta=0.05 -rounds 10000 \
    -autosaveinterval 0

echo "=== SPRT test completed! Results saved to sprt_results.pgn"