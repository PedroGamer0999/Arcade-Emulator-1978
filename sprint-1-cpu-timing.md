# Task Plan: Sprint 1 - Estabilidade e Timing Preciso de CPU (2.0 MHz)

**Task Slug:** `sprint-1-cpu-timing`  
**Status:** Completed  
**Spec Reference:** [`docs/specs/SPEC-001-cpu-timing-interrupts.md`](docs/specs/SPEC-001-cpu-timing-interrupts.md)  
**Branch:** `feature/sprint-1-cpu-timing`  

---

## 🎯 Objetivo
Substituir o loop arbitrário de 2000 instruções por emulação precisa baseada no clock de 2.0 MHz do Intel 8080 (33.333 ciclos/frame), controle formal de interrupções (`inte`, `EI`, `DI`, `RST 1`, `RST 2`) e sincronização de 60 FPS com `SDL_GetPerformanceCounter`.

---

## 📋 Lista de Tarefas

- [x] **Task 1: Atualizar Header `src/cpu8080.h`**
  - Adicionar campos `uint8_t inte` e `uint8_t halt` à `struct State8080`.
  - Atualizar assinatura de `exec_instrucao(State8080 *cpu, InputOutput *io, int *cycles_spent)`.
  - Declarar `int disparar_interrupcao(State8080 *cpu, uint8_t opcode_rst)`.
  - *Verificação:* Compilação limpa das assinaturas.

- [x] **Task 2: Implementar Tabela de Ciclos e Lógica de Interrupções em `src/cpu8080.c`**
  - Incluir tabela estática de ciclos `cycles_8080[256]` conforme Intel 8080.
  - Atualizar `init_cpu()` para inicializar `inte = 0` e `halt = 0`.
  - Implementar `EI` (`0xFB` -> `inte = 1`) e `DI` (`0xF3` -> `inte = 0`).
  - Implementar instrução `HLT` (`0x76` -> `halt = 1`).
  - Implementar `disparar_interrupcao(State8080 *cpu, uint8_t opcode_rst)` com checagem de `cpu->inte`, salvamento do PC na pilha e reset de `inte` e `halt`.
  - Atualizar `exec_instrucao()` para atribuir ciclos gastos e calibrar saltos condicionais (ex: CALL/RET condicional quando condição não é satisfeita).
  - *Verificação:* Compilação limpa de `cpu8080.c`.

- [x] **Task 3: Refatorar Loop Principal e Sincronização 60 FPS em `src/main.c`**
  - Remover loop cego `for (int i = 0; i < 2000; i++)` e `SDL_Delay(16)`.
  - Implementar controle de ciclos por frame: 16.666 ciclos até scanline 96 (`RST 1` / `0xCF`) e 16.667 ciclos até VBLANK (`RST 2` / `0xD7`).
  - Implementar sincronizador de 60 FPS de alta precisão com `SDL_GetPerformanceCounter` e `SDL_GetPerformanceFrequency`.
  - Atualizar chamada a `exec_instrucao` com ponteiro de ciclos.
  - *Verificação:* Build do executável via `makefile.ps1`.

- [x] **Task 4: Testes de Q/A e Validação**
  - **QA-S1-01:** Medir e verificar taxa de 60 FPS contínua (exibição no título da janela SDL).
  - **QA-S1-02:** Verificar ritmo da marcha dos invasores (conforme execução com clock real de 2.0 MHz).
  - **QA-S1-03:** Validar que interrupções com `inte == 0` não corrompem pilha ou fluxo (suíte automatizada `tests/test_cpu_timing.c`).
  - *Verificação:* 31/31 testes aprovados (100%), execução de 100 frames de ROM sem exceções.

- [x] **Task 5: Atualização de Documentação e DoD**
  - Atualizar status em `docs/sprints.md` e `docs/backlog.md`.
