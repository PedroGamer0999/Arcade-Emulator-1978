# Space Invaders Arcade Emulator (1978)

[![Language](https://img.shields.io/badge/Language-English%20%7C%20Portugu%C3%AAs-blue.svg)](#space-invaders-arcade-emulator-1978)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](LICENCE.md)

[English](#english) | [Português (Brasil)](#português-brasil)

---

<a name="english"></a>
## English

An emulator for the classic arcade game **Space Invaders (1978)**, developed in **C** using the **SDL2** library.

### 🕹️ Overview

This project recreates in software the original arcade machine architecture designed by Taito / Midway:

* **Intel 8080 CPU (`src/cpu8080.c` / `src/cpu8080.h`)**:
  * Emulates the 8-bit registers (`A, B, C, D, E, H, L`), Stack Pointer (`SP`), Program Counter (`PC`), and status flags (`Zero, Sign, Parity, Carry`).
  * 64 KB mapped memory space.
  * Decodes and executes the Intel 8080 instruction set.
* **I/O & Dedicated Shift Register (`src/io.c` / `src/io.h`)**:
  * Emulates the hardware *Fujitsu MB14241 Dedicated Shift Register* chip, used by the arcade board to accelerate sprite bit-shifting in real time.
  * Maps arcade input ports `0x01` and `0x02` (coin, start, player controls).
* **Video Rendering (`src/video.c` / `src/video.h`)**:
  * 224 × 256 monochrome resolution (1 bit per pixel in VRAM, from `0x2400` to `0x3FFF`).
  * Real-time 90° counter-clockwise rotation to match the original vertical CRT arcade monitor.
  * 2x scaling (448 × 512 window).
* **Execution Loop & Interrupts (`src/main.c`)**:
  * Loads the 4 ROM chunks into memory:
    * `rom/invaders.h` (offset `0x0000`)
    * `rom/invaders.g` (offset `0x0800`)
    * `rom/invaders.f` (offset `0x1000`)
    * `rom/invaders.e` (offset `0x1800`)
  * Triggers two scanline interrupts per frame (`RST 1` at mid-screen and `RST 2` at VBLANK), regulating timing at ~60 FPS.

---

### 📁 Directory Structure

```text
008-space-invaders-emulator/
├── LICENCE.md              # MIT License
├── Makefile                # Standard Makefile for Linux / macOS (make)
├── makefile.bat            # Windows batch script for one-click compilation
├── makefile.ps1            # Windows PowerShell build script
├── README.md               # Documentation
├── SDL2.dll                # SDL2 runtime library for Windows
├── setup.bat               # Windows one-click environment installer
├── setup.sh                # Linux one-click environment installer
├── space_invaders.exe      # Compiled Windows executable
├── rom/                    # Space Invaders original ROM files
│   ├── invaders.h
│   ├── invaders.g
│   ├── invaders.f
│   └── invaders.e
└── src/                    # C Source files
    ├── cpu8080.c / .h
    ├── io.c / .h
    ├── video.c / .h
    └── main.c
```

---

### ⚙️ Automated Setup (One-Click)

To install all dependencies (compilers, SDL2, VS Code configs) and initialize AI agents in a single command:

* **On Windows:** Double-click `setup.bat` or run `.\setup.bat` in terminal.
* **On Linux:** Run `chmod +x setup.sh && ./setup.sh` in terminal.

---

### 🔨 Manual Build

#### Option 1: On Linux / macOS (GNU Make)
```bash
make
```

#### Option 2: On Windows via PowerShell (makefile.ps1)
Run in PowerShell:
```powershell
.\makefile.ps1
```

#### Option 3: On Windows via Explorer (makefile.bat)
Double-click `makefile.bat`.

#### Option 4: Direct Clang Command
```powershell
$env:PATH = "C:\msys64\ucrt64\bin;$env:PATH"
clang -Wall -Wextra -I. -Isrc src/main.c src/cpu8080.c src/io.c src/video.c -o space_invaders.exe -lmingw32 -lSDL2main -lSDL2
```

---

### 🚀 How to Run

Ensure `SDL2.dll` and the `rom/` directory are located in the same folder as `space_invaders.exe`:

* **In PowerShell:**
  ```powershell
  .\space_invaders.exe
  ```
* **In Windows Explorer:**  
  Double-click `space_invaders.exe`.

---

### 🎮 Controls

| Key | Action |
| :--- | :--- |
| <kbd>C</kbd> or <kbd>Enter</kbd> | Insert Coin |
| <kbd>1</kbd> | 1 Player Start |
| <kbd>←</kbd> | Move Left |
| <kbd>→</kbd> | Move Right |
| <kbd>Space</kbd> | Shoot |

---

<br/>

<a name="português-brasil"></a>
## Português (Brasil)

Um emulador da clássica máquina de arcade do jogo **Space Invaders (1978)**, desenvolvido em linguagem **C** utilizando a biblioteca gráfica **SDL2**.

### 🕹️ Sobre o Projeto

O projeto recria fielmente em software a arquitetura de hardware do gabinete arcade original lançado pela Taito / Midway:

* **CPU Intel 8080 (`src/cpu8080.c` / `src/cpu8080.h`)**:
  * Emulação dos registradores de 8 bits (`A, B, C, D, E, H, L`), Stack Pointer (`SP`), Program Counter (`PC`) e flags de status (`Zero, Sign, Parity, Carry`).
  * Gerenciamento de 64 KB de memória RAM/ROM.
  * Decodificação e execução dos opcodes de montagem do processador 8080.
* **Hardware de I/O e Shift Register (`src/io.c` / `src/io.h`)**:
  * Emulação do chip dedicado de deslocamento de bits (*Fujitsu MB14241 Dedicated Shift Register*), usado pela máquina para desenhar sprites em alta velocidade.
  * Mapeamento de entradas das portas `0x01` e `0x02` (moedas, botões e controles).
* **Renderização de Vídeo (`src/video.c` / `src/video.h`)**:
  * Resolução original de **224 × 256** pixels monocromáticos (1 bit por pixel na VRAM, entre `0x2400` e `0x3FFF`).
  * Rotação de 90° no sentido anti-horário em tempo real para compensar o monitor CRT vertical do gabinete original.
  * Escala gráfica de 2x (janela de 448 × 512 pixels).
* **Ciclo de Execução e Interrupções (`src/main.c`)**:
  * Carrega os 4 arquivos de ROM na memória:
    * `rom/invaders.h` (endereço `0x0000`)
    * `rom/invaders.g` (endereço `0x0800`)
    * `rom/invaders.f` (endereço `0x1000`)
    * `rom/invaders.e` (endereço `0x1800`)
  * Gera as duas interrupções de varredura por frame (`RST 1` no meio da tela e `RST 2` no VBLANK), sincronizando a taxa em ~60 FPS.

---

### 📁 Estrutura de Arquivos

```text
008-space-invaders-emulator/
├── LICENCE.md              # Licença MIT
├── Makefile                # Makefile padrão para Linux / macOS (make)
├── makefile.bat            # Script para compilação via duplo clique no Windows
├── makefile.ps1            # Script de compilação via PowerShell no Windows
├── README.md               # Documentação
├── SDL2.dll                # Biblioteca dinâmica da SDL2 para execução
├── setup.bat               # Instalador automatizado de ambiente para Windows
├── setup.sh                # Instalador automatizado de ambiente para Linux
├── space_invaders.exe      # Executável gerado
├── rom/                    # ROMs originais do Space Invaders
│   ├── invaders.h
│   ├── invaders.g
│   ├── invaders.f
│   └── invaders.e
└── src/                    # Código-fonte em C
    ├── cpu8080.c / .h
    ├── io.c / .h
    ├── video.c / .h
    └── main.c
```

---

### ⚙️ Instalação Automatizada (1 Clique)

Para instalar todas as dependências (compilador, SDL2, configurações do VS Code) e os agentes de IA automaticamente:

* **No Windows:** Dê duplo clique no arquivo `setup.bat` ou execute `.\setup.bat` no terminal.
* **No Linux:** Execute `chmod +x setup.sh && ./setup.sh` no terminal.

---

### 🔨 Compilação Manual

#### Opção 1: No Linux / macOS (via Makefile)
Execute no terminal:
```bash
make
```

#### Opção 2: No Windows pelo PowerShell (makefile.ps1)
Execute o script automatizado no PowerShell:
```powershell
.\makefile.ps1
```

#### Opção 3: No Windows pelo Explorer (makefile.bat)
Dê duplo clique no arquivo `makefile.bat`.

#### Opção 4: Comando Manual Direto
```powershell
$env:PATH = "C:\msys64\ucrt64\bin;$env:PATH"
clang -Wall -Wextra -I. -Isrc src/main.c src/cpu8080.c src/io.c src/video.c -o space_invaders.exe -lmingw32 -lSDL2main -lSDL2
```

---

### 🚀 Como Executar

Certifique-se de que o arquivo `SDL2.dll` e a pasta `rom/` estejam no mesmo diretório de `space_invaders.exe`:

* **Pelo PowerShell:**
  ```powershell
  .\space_invaders.exe
  ```
* **Pelo Windows Explorer:**  
  Dê duplo clique diretamente no arquivo `space_invaders.exe`.

---

### 🎮 Controles

| Tecla | Ação |
| :--- | :--- |
| <kbd>C</kbd> ou <kbd>Enter</kbd> | Inserir Moeda / Crédito (*Insert Coin*) |
| <kbd>1</kbd> | Iniciar Partida (*1 Player Start*) |
| <kbd>←</kbd> | Mover para a Esquerda |
| <kbd>→</kbd> | Mover para a Direita |
| <kbd>Espaço</kbd> | Atirar |
