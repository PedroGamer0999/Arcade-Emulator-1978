# Product Backlog - Space Invaders Arcade Emulator

> Backlog priorizado por valor de entrega, risco e dependência arquitetural.
> Cada item mapeia diretamente para uma especificação técnica detalhada em `docs/specs/`.

---

## 🧭 Visão do Produto
Evoluir a base funcional do emulador de Space Invaders em C para um produto final robusto, fidedigno ao arcade original de 1978, multiplataforma (Windows e Linux), com áudio, vídeo autêntico, suporte a múltiplos jogos da placa Taito/Midway 8080, gerenciador/instalador de ROMs e um Launcher gráfico moderno.

---

## 📊 Matriz de Priorização do Backlog

| ID | Item de Entrega | Prioridade | Valor para o Usuário / Produto | Dependência | Spec Relacionada | Status |
| :--- | :--- | :---: | :--- | :---: | :---: | :---: |
| **BKL-001** | **CPU Cycle Timing & Interrupt Enable** | `P0 (Crítico)` | Garante gameplay a 60 FPS reais e precisão histórica na execução dos opcodes do 8080. | Nenhuma | [`SPEC-001`](specs/SPEC-001-cpu-timing-interrupts.md) | `Concluído (Sprint 1)` |
| **BKL-002** | **Color Overlay Autêntico & Filtro CRT** | `P1 (Alto)` | Transforma o vídeo monocromático no visual original do gabinete arcade com overlay colorido (UFO vermelho, base verde) e scanlines. | BKL-001 | [`SPEC-002`](specs/SPEC-002-arcade-color-overlay-crt.md) | `Concluído (Sprint 2)` |
| **BKL-003** | **Sistema de Áudio SDL2 & Mixer de Efeitos** | `P1 (Alto)` | Adiciona os sons clássicos das portas 0x03 e 0x05 (tiro, passos dos invasores, explosão, UFO) via SDL2 Audio. | BKL-001 | [`SPEC-003`](specs/SPEC-003-arcade-audio-mixer.md) | `Concluído (Sprint 2)` |
| **BKL-004** | **Build System Universal com CMake** | `P1 (Alto)` | Unifica a compilação nativa no Windows (MSVC/Clang/MinGW) e Linux (GCC/Clang), facilitando distribuição e CI/CD. | Nenhuma | [`SPEC-004`](specs/SPEC-004-cmake-crossplatform-build.md) | `Pronto para Sprint 3` |
| **BKL-005** | **ROM Manager com Cheque de Hashes (CRC32/SHA-1) e ZIP** | `P1 (Alto)` | Permite carregar ROMs compactadas (.zip do MAME) e valida a integridade dos dados para evitar falhas silenciosas. | BKL-004 | [`SPEC-005`](specs/SPEC-005-rom-manager-checksums.md) | `Pronto para Sprint 4` |
| **BKL-006** | **Suporte a Múltiplos Jogos da Placa 8080** | `P2 (Médio)` | Expande o emulador para rodar Space Invaders Part II, Lunar Rescue, Balloon Bomber e a ROM de teste CPUDIAG. | BKL-005 | [`SPEC-006`](specs/SPEC-006-multi-game-hardware-support.md) | `Pronto para Sprint 4` |
| **BKL-007** | **Launcher Gráfico Multiplataforma (Windows/Linux)** | `P2 (Médio)` | Janela moderna em SDL2 para selecionar jogos antes de abrir, ver capas/metadados e acessar configurações. | BKL-006 | [`SPEC-007`](specs/SPEC-007-crossplatform-launcher-gui.md) | `Pronto para Sprint 5` |
| **BKL-008** | **Downloader & Instalador de ROMs Integrado** | `P2 (Médio)` | Permite baixar ROMs permitidas / homebrew / domínio público e instalá-las com 1 clique pela interface. | BKL-007 | [`SPEC-008`](specs/SPEC-008-rom-downloader-installer.md) | `Pronto para Sprint 6` |
| **BKL-009** | **Controles Gamepad, DIP Switches & Save States** | `P3 (Desejável)` | Suporte a joysticks USB (SDL_GameController), configuração de vidas/bônus do gabinete e salvamento de estado (F5/F7). | BKL-007 | [`SPEC-009`](specs/SPEC-009-input-gamepad-dipswitches-savestates.md) | `Pronto para Sprint 6` |

---

## 📈 Critérios de Pronto (Definition of Done - DoD) Global
Para qualquer item do backlog ser considerado concluído:
1. Código em C seguindo princípios de Clean Code (sem vazamentos de memória, sem dead code).
2. Compilação sem warnings com `-Wall -Wextra -pedantic` no Windows e Linux.
3. Testes de Q/A executados e validados conforme a especificação correspondente.
4. Documentação de usuário e changelog atualizados.
