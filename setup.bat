@echo off
setlocal enabledelayedexpansion

echo =======================================================================
echo    Space Invaders Emulator (1978) - Instalador de Dependencias
echo =======================================================================
echo.

:: -------------------------------------------------------------------------
:: 1. Verificacao e Instalacao do Toolchain C / MSYS2 / SDL2
:: -------------------------------------------------------------------------
echo [1/5] Verificando compilador e biblioteca SDL2 no Windows...

set "MSYS_PATH=C:\msys64"
set "CLANG_EXE=%MSYS_PATH%\ucrt64\bin\clang.exe"
set "PACMAN_EXE=%MSYS_PATH%\usr\bin\pacman.exe"

if not exist "%MSYS_PATH%" (
    echo [AVISO] MSYS2 nao encontrado em %MSYS_PATH%.
    echo Tentando instalar MSYS2 via winget...
    winget install MSYS2.MSYS2 --silent --accept-package-agreements --accept-source-agreements
    if errorlevel 1 (
        echo [ERRO] Falha ao instalar MSYS2 via winget.
        echo Por favor, instale manualmente o MSYS2 a partir de: https://www.msys2.org/
        pause
        exit /b 1
    )
)

if not exist "%CLANG_EXE%" (
    echo Instalando Clang e SDL2 UCRT64 via pacman...
    "%PACMAN_EXE%" -S --noconfirm --needed mingw-w64-ucrt-x86_64-clang mingw-w64-ucrt-x86_64-SDL2
) else (
    echo [OK] Compilador Clang e MSYS2 UCRT64 detectados.
)

:: Garante que a SDL2.dll esteja presente na raiz
if not exist "SDL2.dll" (
    if exist "%MSYS_PATH%\ucrt64\bin\SDL2.dll" (
        echo Copiando SDL2.dll para a pasta raiz do projeto...
        copy "%MSYS_PATH%\ucrt64\bin\SDL2.dll" "SDL2.dll" >nul
    )
)

:: -------------------------------------------------------------------------
:: 2. Instalacao dos Agentes (ag-kit) preservando as memorias
:: -------------------------------------------------------------------------
echo.
echo [2/5] Verificando ambiente de agentes (ag-kit)...

where npx >nul 2>&1
if errorlevel 1 (
    echo [AVISO] Node.js / npx nao foi encontrado no PATH.
    echo Para instalar os agentes de IA, instale o Node.js: https://nodejs.org/
    echo Pulando etapa de instalacao de agentes...
) else (
    echo [INFO] Node.js / npx detectado.
    
    :: Faz backup das memorias se existirem
    if exist ".agents\memory" (
        echo [INFO] Fazendo backup das memorias do projeto...
        if not exist ".memory_backup" mkdir ".memory_backup"
        xcopy /E /I /Y ".agents\memory" ".memory_backup" >nul
    )

    echo [INFO] Inicializando agentes via ag-kit...
    call npx @vudovn/ag-kit init --yes 2>nul || call npx @vudovn/ag-kit init

    :: Restaura o backup das memorias caso tenham sido sobrescritas
    if exist ".memory_backup" (
        echo [INFO] Restaurando memorias do projeto preservadas...
        if not exist ".agents\memory" mkdir ".agents\memory"
        xcopy /E /I /Y ".memory_backup" ".agents\memory" >nul
        rd /S /Q ".memory_backup"
        echo [OK] Memorias restauradas com sucesso.
    )
)

:: -------------------------------------------------------------------------
:: 3. Configuracao do Ambiente VS Code
:: -------------------------------------------------------------------------
echo.
echo [3/5] Verificando configuracoes do VS Code...

if not exist ".vscode" mkdir ".vscode"

if not exist ".vscode\c_cpp_properties.json" (
    echo Criando configuracao do IntelliSense em .vscode\c_cpp_properties.json...
    (
        echo {
        echo   "configurations": [
        echo     {
        echo       "name": "Win32-UCRT64",
        echo       "includePath": [
        echo         "${workspaceFolder}/**",
        echo         "${workspaceFolder}/src",
        echo         "C:/msys64/ucrt64/include",
        echo         "C:/msys64/ucrt64/include/SDL2"
        echo       ],
        echo       "compilerPath": "C:/msys64/ucrt64/bin/clang.exe",
        echo       "cStandard": "c11",
        echo       "intelliSenseMode": "windows-clang-x64"
        echo     }
        echo   ],
        echo   "version": 4
        echo }
    ) > ".vscode\c_cpp_properties.json"
)

:: -------------------------------------------------------------------------
:: 4. Compilacao de Teste
:: -------------------------------------------------------------------------
echo.
echo [4/5] Executando teste de compilacao com Clang...

set "PATH=%MSYS_PATH%\ucrt64\bin;%PATH%"

clang -Wall -Wextra -I. -Isrc src\main.c src\cpu8080.c src\io.c src\video.c -o space_invaders.exe -lmingw32 -lSDL2main -lSDL2

if errorlevel 1 (
    echo.
    echo [ERRO] Falha na compilacao do emulador. Verifique as mensagens de erro acima.
    pause
    exit /b 1
)

:: -------------------------------------------------------------------------
:: 5. Conclusao
:: -------------------------------------------------------------------------
echo.
echo [5/5] Instalacao concluida com sucesso!
echo Executavel gerado: space_invaders.exe
echo.
echo Agora voce pode:
echo   - Executar o jogo com: .\space_invaders.exe
echo   - Abrir no VS Code com: code .
echo =======================================================================
echo.
pause
