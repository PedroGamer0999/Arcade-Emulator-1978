#!/usr/bin/env bash
set -e

echo "======================================================================="
echo "   Space Invaders Emulator (1978) - Instalador Linux"
echo "======================================================================="
echo ""

# -------------------------------------------------------------------------
# 1. Instalacao de Dependencias do Sistema
# -------------------------------------------------------------------------
echo "[1/4] Instalando compilador e biblioteca SDL2..."

if command -v apt-get &> /dev/null; then
    echo "[INFO] Gerenciador Debian/Ubuntu (apt-get) detectado."
    sudo apt-get update
    sudo apt-get install -y gcc make libsdl2-dev tar
elif command -v dnf &> /dev/null; then
    echo "[INFO] Gerenciador Fedora/RHEL (dnf) detectado."
    sudo dnf install -y gcc make SDL2-devel tar
elif command -v pacman &> /dev/null; then
    echo "[INFO] Gerenciador Arch Linux (pacman) detectado."
    sudo pacman -S --needed --noconfirm gcc make sdl2 tar
else
    echo "[AVISO] Gerenciador de pacotes nao identificado. Instale 'gcc' e 'SDL2' manualmente."
fi

# -------------------------------------------------------------------------
# 2. Instalacao dos Agentes (ag-kit) preservando as memorias
# -------------------------------------------------------------------------
echo ""
echo "[2/4] Verificando ambiente de agentes (ag-kit)..."

if command -v npx &> /dev/null; then
    echo "[INFO] Node.js / npx detectado."
    
    # Backup das memorias
    if [ -d ".agents/memory" ]; then
        echo "[INFO] Fazendo backup das memorias do projeto..."
        mkdir -p ".memory_backup"
        cp -r .agents/memory/* .memory_backup/
    fi

    echo "[INFO] Inicializando agentes via ag-kit..."
    npx @vudovn/ag-kit init --yes || npx @vudovn/ag-kit init || true

    # Restaura backup das memorias
    if [ -d ".memory_backup" ]; then
        echo "[INFO] Restaurando memorias do projeto preservadas..."
        mkdir -p ".agents/memory"
        cp -r .memory_backup/* .agents/memory/
        rm -rf .memory_backup
        echo "[OK] Memorias restauradas com sucesso."
    fi
else
    echo "[AVISO] Node.js / npx nao encontrado. Instale o Node.js se desejar usar os agentes de IA."
fi

# -------------------------------------------------------------------------
# 3. Compilacao de Teste
# -------------------------------------------------------------------------
echo ""
echo "[3/4] Compilando o Space Invaders..."

gcc -Wall -Wextra -I. -Isrc src/main.c src/cpu8080.c src/io.c src/video.c -o space_invaders -lSDL2

# -------------------------------------------------------------------------
# 4. Conclusao
# -------------------------------------------------------------------------
echo ""
echo "[4/4] Instalacao concluida com sucesso!"
echo "Executavel gerado: ./space_invaders"
echo ""
echo "Para jogar, execute: ./space_invaders"
echo "======================================================================="
