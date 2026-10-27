@echo off
setlocal EnableDelayedExpansion

echo ============================================================
echo  Space Invaders Emulator - CMake Build (Windows)
echo ============================================================

:: ── Detect MSYS2 / MinGW ────────────────────────────────────────────────────
set "MSYS2_PATH="
for %%P in (
    "C:\msys64\ucrt64\bin"
    "C:\msys64\mingw64\bin"
    "C:\msys2\ucrt64\bin"
    "C:\msys2\mingw64\bin"
) do (
    if exist "%%~P\gcc.exe" (
        set "MSYS2_PATH=%%~P"
        goto :found_compiler
    )
    if exist "%%~P\clang.exe" (
        set "MSYS2_PATH=%%~P"
        goto :found_compiler
    )
)

echo [WARNING] MSYS2/MinGW not found in default locations.
echo           Make sure cmake and a C compiler are on your PATH.
goto :run_cmake

:found_compiler
echo [INFO] Compiler found: !MSYS2_PATH!
set "PATH=!MSYS2_PATH!;%PATH%"

:run_cmake
:: ── Detect cmake ────────────────────────────────────────────────────────────
where cmake >nul 2>&1
if errorlevel 1 (
    echo [ERROR] cmake not found. Install CMake from https://cmake.org/download/
    exit /b 1
)

:: ── Configure ────────────────────────────────────────────────────────────────
echo.
echo [1/2] Configuring with CMake...
cmake -B build -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release
if errorlevel 1 (
    echo [ERROR] CMake configuration failed.
    exit /b 1
)

:: ── Build ─────────────────────────────────────────────────────────────────────
echo.
echo [2/2] Building...
cmake --build build --config Release
if errorlevel 1 (
    echo [ERROR] Build failed.
    exit /b 1
)

echo.
echo ============================================================
echo  Build successful!
echo  Executable: build\SpaceInvadersEmulator.exe
echo ============================================================
endlocal
