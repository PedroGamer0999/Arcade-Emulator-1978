# Script para compilar o Space Invaders no Windows via PowerShell

$ErrorActionPreference = "Stop"

# Adiciona o compilador ao PATH temporariamente para a sessao
$env:PATH = "C:\msys64\ucrt64\bin;$env:PATH"

Write-Host "Compilando Space Invaders..." -ForegroundColor Cyan

# Compila usando Clang do MSYS2 incluindo todos os modulos
clang -Wall -Wextra -I. -Isrc src/main.c src/cpu8080.c src/io.c src/video.c src/audio.c -o space_invaders.exe -lmingw32 -lSDL2main -lSDL2

# Garante que a SDL2.dll esteja na pasta raiz
if (-not (Test-Path "SDL2.dll")) {
    Copy-Item "C:\msys64\ucrt64\bin\SDL2.dll" "SDL2.dll"
}

Write-Host "Compilacao concluida com sucesso! Executavel gerado: space_invaders.exe" -ForegroundColor Green
