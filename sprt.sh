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
BINARY_NAME="slychess"
WORKTREE_BASE_DIR="../sprt_slychess/worktree_base"
WORKTREE_NEW_DIR="../sprt_slychess/worktree_new"
BUILD_BASE_DIR="$WORKTREE_BASE_DIR/build"
BUILD_NEW_DIR="$WORKTREE_NEW_DIR/build"

# Resolve the commits here, so e.g. "HEAD" means this checkout's HEAD, not the worktree's
SHA_BASE=$(git rev-parse --verify --short "$COMMIT_BASE^{commit}")
SHA_NEW=$(git rev-parse --verify --short "$COMMIT_NEW^{commit}")

# Portable CPU count (nproc doesn't exist on macOS)
JOBS=$(getconf _NPROCESSORS_ONLN)

# Check out a commit in its own worktree, so the main working copy is never touched.
# The worktrees are reused between runs, which keeps rebuilds incremental.
checkout_worktree() {
    if [ -e "$1/.git" ]; then
        git -C "$1" checkout --quiet --force --detach "$2"
    else
        git worktree prune
        git worktree add --detach "$1" "$2"
    fi
}

# --- 1. BUILD BASE ENGINE ---
echo "=== Checking out base commit $COMMIT_BASE into $WORKTREE_BASE_DIR"
checkout_worktree "$WORKTREE_BASE_DIR" "$SHA_BASE"

echo "=== Configuring and compiling base engine..."
mkdir -p "$BUILD_BASE_DIR"
cmake -S "$WORKTREE_BASE_DIR" -B "$BUILD_BASE_DIR" -DCMAKE_C_COMPILER="$C_COMPILER" -DCMAKE_CXX_COMPILER="$CXX_COMPILER" -DCMAKE_BUILD_TYPE=Release
cmake --build "$BUILD_BASE_DIR" --config Release -j "$JOBS"

# --- 2. BUILD NEW ENGINE ---
echo "=== Checking out new commit $COMMIT_NEW into $WORKTREE_NEW_DIR"
checkout_worktree "$WORKTREE_NEW_DIR" "$SHA_NEW"

echo "=== Configuring and compiling new engine..."
mkdir -p "$BUILD_NEW_DIR"
cmake -S "$WORKTREE_NEW_DIR" -B "$BUILD_NEW_DIR" -DCMAKE_C_COMPILER="$C_COMPILER" -DCMAKE_CXX_COMPILER="$CXX_COMPILER" -DCMAKE_BUILD_TYPE=Release
cmake --build "$BUILD_NEW_DIR" --config Release -j "$JOBS"

# --- 3. RUN SPRT TEST ---
echo "=== Starting fastchess SPRT test..."
fastchess \
    -engine cmd="./$BUILD_NEW_DIR/$BINARY_NAME" name="New-$COMMIT_NEW" \
    -engine cmd="./$BUILD_BASE_DIR/$BINARY_NAME" name="Base-$COMMIT_BASE" \
    -pgnout file="../sprt_slychess/sprt_${SHA_NEW}_vs_${SHA_BASE}.pgn" \
    -openings file="../sprt_slychess/books/8moves_v3.pgn" format=pgn order=random \
    -each tc=5.0+0.1 -repeat -concurrency 6 -recover \
    -sprt elo0=0 elo1=5 alpha=0.05 beta=0.05 -rounds 50000 \
    -autosaveinterval 0

echo "=== SPRT test completed! Results saved to sprt_${SHA_NEW}_vs_${SHA_BASE}.pgn"