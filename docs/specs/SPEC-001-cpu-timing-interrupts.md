# SPEC-001: Timing Preciso de CPU (2.0 MHz) e Gestão Formal de Interrupções

- **Status:** Aprovado para Implementação (Sprint 1)
- **Autor:** Game Developer / Systems Specialist
- **Arquivos Afetados:** [`src/cpu8080.h`](file:///D:/projetos/008-space-invaders-emulator/src/cpu8080.h), [`src/cpu8080.c`](file:///D:/projetos/008-space-invaders-emulator/src/cpu8080.c), [`src/main.c`](file:///D:/projetos/008-space-invaders-emulator/src/main.c)
- **Dependências:** Nenhuma

---

## 1. Contexto e Motivação
Na versão atual do emulador ([`src/main.c#L53`](file:///D:/projetos/008-space-invaders-emulator/src/main.c#L53)), a CPU executa um loop de `2000` instruções fixas por meio frame.
No hardware real do arcade Space Invaders:
- O processador Intel 8080 opera a um clock fixo de **2.000.000 Hz (2.0 MHz)**.
- O display opera a **~60 Hz** (exatamente 59.54 Hz).
- Em cada frame de 1/60s ocorrem **33.333 ciclos de CPU**.
- O arcade gera duas interrupções de hardware:
  - **RST 1 (`0xCF`)**: Ocorre no meio da varredura da tela (scanline 96), exatamente após **16.666 ciclos**.
  - **RST 2 (`0xD7`)**: Ocorre no final da tela (VBLANK / scanline 224), após mais **16.667 ciclos**.
- O 8080 possui a flag `inte` (Interrupt Enable). Se o programa executar `DI` (Disable Interrupts), a CPU **não pode** aceitar o sinal de interrupção até que um `EI` (Enable Interrupts) seja executado.

Executar 2000 instruções cegas causa flutuações grotescas de velocidade, problemas de física e falhas quando outros jogos da placa forem carregados.

---

## 2. Requisitos

### Requisitos Funcionais (RF)
- **RF-01**: Cada instrução executada por `exec_instrucao()` deve retornar a quantidade exata de ciclos de máquina consumidos (entre 4 e 18 ciclos).
- **RF-02**: A CPU deve manter uma flag de interrupção habilitada (`uint8_t inte`).
- **RF-03**: A instrução `EI` (0xFB) deve setar `inte = 1`.
- **RF-04**: A instrução `DI` (0xF3) deve setar `inte = 0`.
- **RF-05**: Quando uma interrupção é aceita e executada, a CPU deve desabilitar interrupções automaticamente (`inte = 0`).
- **RF-06**: O loop principal deve sincronizar os quadros a 60 FPS estáveis usando temporizador de alta resolução do SDL2 (`SDL_GetPerformanceCounter`).

### Requisitos Não-Funcionais (RNF)
- **RNF-01**: Desvio máximo de clock de ±1% em relação aos 2.0 MHz nominais.
- **RNF-02**: Tabela de ciclos (`static const uint8_t cycles_8080[256]`) com complexidade de acesso $O(1)$.

---

## 3. Especificação Técnica e Arquitetura

### 3.1 Alterações em `src/cpu8080.h`
```c
typedef struct State8080 {
    uint8_t a, b, c, d, e, h, l;
    uint16_t sp, pc;
    uint8_t flags; // [S, Z, 0, A, 0, P, 1, C]
    uint8_t inte;  // 1 = Interrupções habilitadas, 0 = desabilitadas
    uint8_t halt;  // 1 = CPU em estado HALT
    uint8_t memory[0x10000];
} State8080;

// Agora retorna o número de ciclos gastos pela instrução
int exec_instrucao(State8080 *cpu, InputOutput *io, int *cycles_spent);

// Dispara interrupção respeitando a flag inte
int disparar_interrupcao(State8080 *cpu, uint8_t opcode_rst);
```

### 3.2 Tabela de Ciclos do Intel 8080 (`src/cpu8080.c`)
Criar vetor constante indexado por opcode:
```c
static const uint8_t cycles_8080[256] = {
//  0  1   2   3   4   5   6   7   8   9   A   B   C   D   E   F
    4, 10, 7,  5,  5,  5,  7,  4,  4, 10, 7,  5,  5,  5,  7,  4, // 0x00 - 0x0F
    4, 10, 7,  5,  5,  5,  7,  4,  4, 10, 7,  5,  5,  5,  7,  4, // 0x10 - 0x1F
    4, 10, 16, 5,  5,  5,  7,  4,  4, 10, 16, 5,  5,  5,  7,  4, // 0x20 - 0x2F
    4, 10, 13, 5,  10, 10, 10, 4,  4, 10, 13, 5,  5,  5,  7,  4, // 0x30 - 0x3F
    5, 5,  5,  5,  5,  5,  7,  5,  5, 5,  5,  5,  5,  5,  7,  5, // 0x40 - 0x4F
    5, 5,  5,  5,  5,  5,  7,  5,  5, 5,  5,  5,  5,  5,  7,  5, // 0x50 - 0x5F
    5, 5,  5,  5,  5,  5,  7,  5,  5, 5,  5,  5,  5,  5,  7,  5, // 0x60 - 0x6F
    7, 7,  7,  7,  7,  7,  7,  7,  5, 5,  5,  5,  5,  5,  7,  5, // 0x70 - 0x7F
    4, 4,  4,  4,  4,  4,  7,  4,  4, 4,  4,  4,  4,  4,  7,  4, // 0x80 - 0x8F
    4, 4,  4,  4,  4,  4,  7,  4,  4, 4,  4,  4,  4,  4,  7,  4, // 0x90 - 0x9F
    4, 4,  4,  4,  4,  4,  7,  4,  4, 4,  4,  4,  4,  4,  7,  4, // 0xA0 - 0xAF
    4, 4,  4,  4,  4,  4,  7,  4,  4, 4,  4,  4,  4,  4,  7,  4, // 0xB0 - 0xBF
    5, 10, 10, 10, 11, 11, 7,  11, 5, 10, 10, 10, 11, 17, 7,  11, // 0xC0 - 0xCF
    5, 10, 10, 10, 11, 11, 7,  11, 5, 10, 10, 10, 11, 17, 7,  11, // 0xD0 - 0xDF
    5, 10, 10, 18, 11, 11, 7,  11, 5, 5,  10, 4,  11, 17, 7,  11, // 0xE0 - 0xEF
    5, 10, 10, 4,  11, 11, 7,  11, 5, 5,  10, 4,  11, 17, 7,  11  // 0xF0 - 0xFF
};
```
*(Nota: Instruções condicionais onde a condição não é satisfeita gastam menos ciclos — ex: CALL condicional = 11 ciclos se falso, 17 se verdadeiro. A lógica da instrução ajustará a contagem).*

### 3.3 Loop Central de Frame em `src/main.c`
```c
#define CPU_CLOCK_HZ        2000000
#define CYCLES_PER_FRAME    (CPU_CLOCK_HZ / 60)         // 33333
#define CYCLES_PER_HALF     (CYCLES_PER_FRAME / 2)      // 16666

int ciclos_executados = 0;
while (ciclos_executados < CYCLES_PER_HALF) {
    int cycles = 0;
    exec_instrucao(&cpu, &io, &cycles);
    ciclos_executados += cycles;
}
ciclos_executados -= CYCLES_PER_HALF;
disparar_interrupcao(&cpu, 0xCF); // Mid-screen RST 1

while (ciclos_executados < CYCLES_PER_HALF) {
    int cycles = 0;
    exec_instrucao(&cpu, &io, &cycles);
    ciclos_executados += cycles;
}
ciclos_executados -= CYCLES_PER_HALF;
disparar_interrupcao(&cpu, 0xD7); // VBLANK RST 2
atualizar_video(&display, &cpu);
sincronizar_60fps();
```

---

## 4. Critérios de Aceite (Definition of Done)
- [x] O loop de `for (int i = 0; i < 2000; i++)` é completamente removido.
- [x] Toda instrução retorna a quantidade de ciclos oficiais da Intel.
- [x] Interrupções nunca corrompem a pilha se `cpu->inte == 0`.
- [x] O jogo roda em exatos 60 quadros por segundo com sincronização por clock real.

---

## 5. Roteiro de Testes de Q/A

| Teste | Descrição | Ação | Resultado Esperado |
| :--- | :--- | :--- | :--- |
| **TC-001-A** | Medição de FPS | Executar o emulador por 60 segundos com contador de FPS ativo. | Média estritamente de 59.9 ~ 60.1 FPS. Uso de CPU normalizado (sem consumir 100% de um núcleo). |
| **TC-001-B** | Integridade do `inte` | Injetar interrupção artificial logo após instrução `DI`. | A interrupção deve ser ignorada ou enfileirada, mantendo o fluxo do código íntegro. |
| **TC-001-C** | Ritmo de Jogo | Comparar a taxa de disparo e marcha dos aliens com gameplay original do arcade. | Velocidade de descida dos invasores idêntica ao arcade clássico. |
