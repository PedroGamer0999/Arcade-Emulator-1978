#!/usr/bin/env bash
# build.sh — Space Invaders Emulator CMake build helper (Linux / macOS)
set -euo pipefail

COLOR_CYAN="\033[0;36m"
COLOR_GREEN="\033[0;32m"
COLOR_RED="\033[0;31m"
COLOR_RESET="\033[0m"

info()  { printf "${COLOR_CYAN}[INFO]${COLOR_RESET}  %s\n" "$*"; }
ok()    { printf "${COLOR_GREEN}[OK]${COLOR_RESET}    %s\n" "$*"; }
error() { printf "${COLOR_RED}[ERROR]${COLOR_RESET} %s\n" "$*" >&2; exit 1; }

echo "============================================================"
echo " Space Invaders Emulator - CMake Build (Linux / macOS)"
echo "============================================================"

# ── Check dependencies ─────────────────────────────────────────────────────────
command -v cmake  &>/dev/null || error "cmake not found. Install: sudo apt install cmake"
command -v cc     &>/dev/null || \
command -v gcc    &>/dev/null || \
command -v clang  &>/dev/null || error "C compiler not found. Install: sudo apt install build-essential"

if ! pkg-config --exists sdl2 2>/dev/null && [ ! -f /usr/include/SDL2/SDL.h ]; then
    error "SDL2 not found. Install: sudo apt install libsdl2-dev  (Debian/Ubuntu)\n         or: sudo dnf install SDL2-devel  (Fedora)\n         or: sudo pacman -S sdl2  (Arch)"
fi

BUILD_TYPE="${1:-Release}"
BUILD_DIR="build"

# ── Configure ──────────────────────────────────────────────────────────────────
info "Configuring — type: ${BUILD_TYPE}"
cmake -B "${BUILD_DIR}" -DCMAKE_BUILD_TYPE="${BUILD_TYPE}"

# ── Build ──────────────────────────────────────────────────────────────────────
JOBS=$(nproc 2>/dev/null || sysctl -n hw.logicalcpu 2>/dev/null || echo 4)
info "Building with ${JOBS} parallel jobs..."
cmake --build "${BUILD_DIR}" --config "${BUILD_TYPE}" -- -j"${JOBS}"

echo ""
echo "============================================================"
ok "Build successful!"
echo "   Executable: ${BUILD_DIR}/SpaceInvadersEmulator"
echo "============================================================"
