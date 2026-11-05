# Task Plan: Sprint 4 - Gerenciador de ROMs e Múltiplos Jogos 8080 (v2.0)

**Task Slug:** `sprint-4-rom-manager`  
**Status:** Completed  
**Spec References:**  
- [`docs/specs/SPEC-005-rom-manager-checksums.md`](docs/specs/SPEC-005-rom-manager-checksums.md)  
- [`docs/specs/SPEC-006-multi-game-hardware-support.md`](docs/specs/SPEC-006-multi-game-hardware-support.md)  
**Branch:** `feature/sprint-4-rom-manager`  

---

## 🎯 Objetivo
Criar um gerenciador robusto de ROMs com extração in-memory de arquivos ZIP via `miniz` e validação CRC32, implementar arquitetura de Game Drivers para suporte a 5 títulos clássicos da placa 8080, e habilitar seleção de jogo via linha de comando.

---

## 📋 Lista de Tarefas

- [x] **Task 1: Integrar `miniz` (single-header ZIP/deflate em C puro)**
  - Criar `src/miniz.h` e `src/miniz.c` com implementação miniz v3.0.
  - *Verificação:* Compilação limpa sem warnings.

- [x] **Task 2: Implementar `src/rom_manager.h` e `src/rom_manager.c`**
  - Struct `RomPiece` (nome, offset, tamanho, CRC32) e `RomSetDefinition`.
  - `calcular_crc32()` — implementação C pura sem dependências.
  - `carregar_romset()` — suporta pasta de arquivos soltos E arquivo ZIP (via miniz).
  - `verificar_romset()` — retorna mensagem de erro amigável com CRC obtido vs esperado.
  - *Verificação:* Compilação limpa; `carregar_romset()` funcional com `rom/` e `.zip`.

- [x] **Task 3: Implementar `src/game_driver.h` e `src/game_driver.c`**
  - Struct `GameDriver` com campos id, nome, romset e callbacks `init_hardware`, `io_in`, `io_out`, `color_map`.
  - Registro dos 5 drivers: `invaders`, `invadpt2`, `lrescu`, `ballbomb`, `schaser`.
  - Driver especial `cpudiag`: intercepta `CALL 0x0005` para saída serial no terminal.
  - `obter_driver_por_id()`, `detectar_driver_por_arquivo()`, `listar_drivers_disponiveis()`.
  - *Verificação:* Compilação limpa; `obter_driver_por_id("invaders")` retorna driver correto.

- [x] **Task 4: Refatorar `src/main.c` para usar GameDriver e ROM Manager**
  - Parsear `argc/argv`: `-game <id>`, caminho direto de arquivo `.zip` ou `.bin`.
  - Substituir carga estática `carregar_rom()` por `carregar_romset()` do driver selecionado.
  - Loop de emulação: chamar callbacks `driver->io_in` / `driver->io_out` em vez de `io_in`/`io_out` diretamente.
  - Suporte especial CPUDIAG: interceptar PC == 0x0005 no loop.
  - Atualizar título da janela SDL com nome do jogo ativo.
  - *Verificação:* Build limpo; emulador inicia com `./SpaceInvadersEmulator` (padrão invaders) e com `-game invaders`.

- [x] **Task 5: Atualizar `CMakeLists.txt` e scripts de build**
  - CMakeLists.txt usa `file(GLOB SOURCES "src/*.c")`, detectando automaticamente os novos módulos.
  - *Verificação:* `cmake --build build` com código de saída 0.

- [x] **Task 6: Testes automatizados e Q/A**
  - QA-S4-01: Carga de `rom/` com CRC32 válidos → jogo inicia normalmente.
  - QA-S4-02: CRC32 corrompido / divergente → aviso legível com valores hex.
  - QA-S4-03: Arquivo ausente → erro amigável sem crash.
  - QA-S4-04: `obter_driver_por_id` retorna todos os 6 drivers registrados.
  - QA-S4-05: Carga in-memory via ZIP funcional.
  - *Verificação:* Suite de testes automatizada `tests/test_rom_manager.c` com 11/11 testes passando (100%).

- [x] **Task 7: Atualização de Documentação e DoD**
  - Atualizar `docs/sprints.md`, `docs/backlog.md` e `README.md`.

---

## ✅ Critérios de Conclusão (DoD)
- Build limpo com zero warnings.
- `carregar_romset()` funcional com pasta soltos e ZIP.
- 5 drivers arcade + 1 de diagnóstico registrados e acessíveis por ID.
- `main.c` aceita `-game <id>` e caminho direto de arquivo.
- Documentação atualizada.
