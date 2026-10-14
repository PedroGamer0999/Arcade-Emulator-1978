# SPEC-006: Suporte a Múltiplos Jogos da Placa 8080 (Game Drivers)

- **Status:** Aprovado para Implementação (Sprint 4)
- **Autor:** Game Developer / Systems Specialist
- **Arquivos Afetados:** `src/game_driver.h`, `src/game_driver.c`, [`src/io.h`](file:///D:/projetos/008-space-invaders-emulator/src/io.h), [`src/io.c`](file:///D:/projetos/008-space-invaders-emulator/src/io.c), [`src/video.c`](file:///D:/projetos/008-space-invaders-emulator/src/video.c), [`src/main.c`](file:///D:/projetos/008-space-invaders-emulator/src/main.c)
- **Dependências:** [`SPEC-005`](SPEC-005-rom-manager-checksums.md)

---

## 1. Contexto e Motivação
A placa de arcade desenvolvida pela Taito e licenciada pela Midway para o Space Invaders tornou-se a plataforma padrão para vários outros títulos de arcade do final dos anos 70 e início dos anos 80:
- Todos compartilham a mesma **CPU Intel 8080 a 2.0 MHz**.
- Todos utilizam a mesma faixa de **VRAM (0x2400 a 0x3FFF)** com orientação vertical de 224x256 pixels.
- Quase todos utilizam o mesmo chip acelerador de rotação **Fujitsu MB14241 Shift Register** (Portas 2, 3 e 4).

No entanto, cada jogo possui pequenas variações no mapeamento de portas de entrada (bits de botões), esquema de cores (ex: Lunar Rescue possui cores dinâmicas via PROM de cor) e endereçamento de memória de ROM estendida.
Para transformar o emulador em uma **plataforma multijogos profissional**, desacoplamos a máquina em uma arquitetura de **Game Drivers**.

---

## 2. Requisitos

### Requisitos Funcionais (RF)
- **RF-01**: Criar uma interface abstrata de driver (`GameDriver`) contendo ponteiros de função para inicialização, ciclo de clock, I/O e renderização customizada.
- **RF-02**: Implementar suporte aos seguintes títulos clássicos:
  1. **Space Invaders (1978)** (`invaders`)
  2. **Space Invaders Part II / Deluxe (1979)** (`invadpt2`)
  3. **Lunar Rescue (1979)** (`lrescu`)
  4. **Balloon Bomber (1980)** (`ballbomb`)
  5. **Space Chaser (1979)** (`schaser`)
  6. **CPUDIAG (Microcosm 8080 Diagnostic)** (`cpudiag`) - Suíte de testes automatizada que valida cada instrução da CPU.
- **RF-03**: Suporte especial ao driver `cpudiag`:
  - Interceptar a chamada de saída de console do CPUDIAG (instrução `CALL 0x0005`) para imprimir mensagens de diagnóstico ("CPU TESTS OK" ou instrução falha) diretamente no terminal/stdout.
- **RF-04**: O usuário pode selecionar o jogo via argumento na linha de comando (`./space_invaders -game lrescu` ou `./space_invaders roms/lrescu.zip`) ou via Launcher.

---

## 3. Especificação Técnica e Arquitetura

### 3.1 Estrutura do Game Driver (`src/game_driver.h`)
```c
#ifndef GAME_DRIVER_H
#define GAME_DRIVER_H

#include "cpu8080.h"
#include "io.h"
#include "video.h"
#include "rom_manager.h"

typedef struct GameDriver {
    const char *id;             // Ex: "invaders", "lrescu", "cpudiag"
    const char *nome_completo;  // Ex: "Lunar Rescue (Taito 1979)"
    const char *ano;
    const char *fabricante;
    const RomSetDefinition *romset;
    
    // Callbacks de Hardware
    void (*init_hardware)(State8080 *cpu, InputOutput *io);
    uint8_t (*io_in)(InputOutput *io, uint8_t port);
    void (*io_out)(InputOutput *io, uint8_t port, uint8_t val);
    uint32_t (*color_map)(int x, int y, uint8_t pixel_ativo);
} GameDriver;

// Registro global de drivers disponíveis
const GameDriver* obter_driver_por_id(const char *id);
const GameDriver* detectar_driver_por_arquivo(const char *caminho);
int listar_drivers_disponiveis(const GameDriver ***lista);

#endif
```

### 3.2 Tabela Comparativa de Mapeamento de Hardware

| Jogo | ROM Size | Shift Reg? | Mapeamento de Cores | Particularidades de I/O |
| :--- | :---: | :---: | :--- | :--- |
| **Space Invaders** | 8 KB (0x0000 - 0x1FFF) | Sim | Overlay Y (Vermelho/Branco/Verde) | Porta 1 e 2 padrão |
| **Space Invaders Part II** | 10 KB (0x0000 - 0x27FF) | Sim | Overlay similar + novas naves | Suporte a pontuação com nome |
| **Lunar Rescue** | 12 KB (0x0000 - 0x2FFF) | Sim | Paleta colorida via Color PROM | Barricadas viram plataformas |
| **Balloon Bomber** | 10 KB (0x0000 - 0x27FF) | Sim | Overlay com nuvens | Canhão antiaéreo móvel |
| **CPUDIAG** | 1.5 KB (carregado em 0x0100) | Não | Saída serial em 0x0005 | Sobrescreve SP e testa opcodes |

### 3.3 Suporte ao `CPUDIAG` (Validação de CPU 8080)
Para rodar a suíte oficial de teste do 8080:
```c
// Em main.c / game_driver.c para o driver CPUDIAG:
if (cpu->pc == 0x0005) {
    if (cpu->c == 9) { // Imprimir string apontada por DE até o caractere '$'
        uint16_t addr = (cpu->d << 8) | cpu->e;
        while (cpu->memory[addr] != '$') {
            putchar(cpu->memory[addr++]);
        }
    } else if (cpu->c == 2) { // Imprimir caractere em E
        putchar(cpu->e);
    }
    // Retorna da chamada simulada
    cpu->pc = cpu->memory[cpu->sp] | (cpu->memory[cpu->sp + 1] << 8);
    cpu->sp += 2;
}
```

---

## 4. Critérios de Aceite (Definition of Done)
- [x] O comando `./space_invaders -test` ou `./space_invaders roms/cpudiag.bin` roda o CPUDIAG e imprime a mensagem `CPU TESTS OK`.
- [x] O emulador carrega com sucesso as ROMs de `invadpt2.zip` e `lrescu.zip` respeitando os tamanhos de memória estendidos.
- [x] Os controles e shift register funcionam nos novos jogos sem colisão de código.

---

## 5. Roteiro de Testes de Q/A

| Teste | Descrição | Ação | Resultado Esperado |
| :--- | :--- | :--- | :--- |
| **TC-006-A** | Execução do CPUDIAG | Executar `./space_invaders cpudiag.bin`. | O terminal imprime os testes passo a passo e finaliza com `CPU TESTS OK`. Zero opcodes inválidos reportados. |
| **TC-006-B** | Space Invaders Part II | Carregar `roms/invadpt2.zip`. | A tela de demonstração do Space Invaders Deluxe aparece, com invasores se dividindo e novas animações. |
| **TC-006-C** | Lunar Rescue | Carregar `roms/lrescu.zip`. | O jogo de resgate lunar inicia, a nave desce desviando de meteoros e o canhão responde aos comandos. |
