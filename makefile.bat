@echo off
set "PATH=C:\msys64\ucrt64\bin;%PATH%"
echo Compilando Space Invaders...
clang -Wall -Wextra -I. -Isrc src/main.c src/cpu8080.c src/io.c src/video.c -o space_invaders.exe -lmingw32 -lSDL2main -lSDL2
if %ERRORLEVEL% equ 0 (
    if not exist "SDL2.dll" copy "C:\msys64\ucrt64\bin\SDL2.dll" "SDL2.dll"
    echo Compilacao concluida com sucesso! Executavel gerado: space_invaders.exe
) else (
    echo Erro ao compilar o projeto.
)
pause
