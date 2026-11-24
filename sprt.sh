#!/usr/bin/env bash

# Exit immediately if a command exits with a non-zero status
set -e

usage() {
    echo "Usage: $0 [--cc <c_compiler>] [--cxx <cxx_compiler>] <commit_hash_base> <commit_hash_new>"
    exit 1
}

# Compilers passed to CMake, override with --cc/--cxx
C_COMPILER="gcc"
CXX_COMPILER="g++"

# Parse the options and the two commit hashes
COMMITS=()
while [ "$#" -gt 0 ]; do
    case "$1" in
        --cc)  [ "$#" -ge 2 ] || usage; C_COMPILER="$2"; shift 2 ;;
        --cxx) [ "$#" -ge 2 ] || usage; CXX_COMPILER="$2"; shift 2 ;;
        -*)    usage ;;
        *)     COMMITS+=("$1"); shift ;;
    esac
done
[ "${#COMMITS[@]}" -eq 2 ] || usage

COMMIT_BASE="${COMMITS[0]}"
COMMIT_NEW="${COMMITS[1]}"

# --- CONFIGURATION ---
BINARY_NAME="SlyChess"
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
    if [ "$STASHED" = "1" ]; then
        git stash pop >/dev/null 2>&1 || true
    fi
    exit 1
}

# Trap errors to run cleanup
trap cleanup_on_error ERR

# Only stash if there are uncommitted changes to tracked files, otherwise the
# "git stash pop" below would pop an older, unrelated stash
STASHED="0"
if [ -n "$(git status --porcelain -uno)" ]; then
    echo "=== Stashing any uncommitted local changes..."
    git stash
    STASHED="1"
else
    echo "=== No local changes to stash."
fi

# nproc doesn't exist on macOS
JOBS=$(getconf _NPROCESSORS_ONLN)

# --- 1. BUILD BASE ENGINE ---
echo "=== Checking out base commit: $COMMIT_BASE"
git checkout "$COMMIT_BASE"

echo "=== Configuring and compiling base engine..."
mkdir -p "$BUILD_BASE_DIR"

cmake -S . -B "$BUILD_BASE_DIR" -DCMAKE_C_COMPILER="$C_COMPILER" -DCMAKE_CXX_COMPILER="$CXX_COMPILER" -DCMAKE_BUILD_TYPE=Release
cmake --build "$BUILD_BASE_DIR" --config Release -j "$JOBS"

# --- 2. BUILD NEW ENGINE ---
echo "=== Checking out new commit: $COMMIT_NEW"
git checkout "$COMMIT_NEW"

echo "=== Configuring and compiling new engine..."
mkdir -p "$BUILD_NEW_DIR"

cmake -S . -B "$BUILD_NEW_DIR" -DCMAKE_C_COMPILER="$C_COMPILER" -DCMAKE_CXX_COMPILER="$CXX_COMPILER" -DCMAKE_BUILD_TYPE=Release
cmake --build "$BUILD_NEW_DIR" --config Release -j "$JOBS"

# --- 3. RESTORE REPO STATE ---
echo "=== Restoring original git state ($CURRENT_STATE)..."
git checkout "$CURRENT_STATE"
if [ "$STASHED" = "1" ]; then
    echo "=== Restoring stashed changes..."
    git stash pop >/dev/null 2>&1 || echo "Note: Conflict encountered while popping stash."
fi

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