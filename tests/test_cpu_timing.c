#include <stdio.h>
#include <assert.h>
#include <string.h>
#include "cpu8080.h"
#include "io.h"

static int test_total = 0;
static int test_passed = 0;

#define ASSERT_TEST(cond, msg) do { \
    test_total++; \
    if (cond) { \
        test_passed++; \
        printf("  [PASS] %s\n", msg); \
    } else { \
        printf("  [FAIL] %s (linha %d)\n", msg, __LINE__); \
    } \
} while(0)

void test_initial_state() {
    printf("\n--- Teste 1: Estado Inicial da CPU ---\n");
    State8080 cpu;
    init_cpu(&cpu);

    ASSERT_TEST(cpu.inte == 0, "Flag inte inicializada em 0");
    ASSERT_TEST(cpu.halt == 0, "Flag halt inicializada em 0");
    ASSERT_TEST(cpu.pc == 0, "PC inicializado em 0");
    ASSERT_TEST(cpu.sp == 0, "SP inicializado em 0");
}

void test_ei_di_instructions() {
    printf("\n--- Teste 2: Instrucoes EI (0xFB) e DI (0xF3) ---\n");
    State8080 cpu;
    InputOutput io;
    init_cpu(&cpu);
    init_io(&io);

    int cycles = 0;

    // Coloca instrucao EI (0xFB) no PC 0
    cpu.memory[0] = 0xFB;
    exec_instrucao(&cpu, &io, &cycles);
    ASSERT_TEST(cpu.inte == 1, "EI habilita interrupcoes (inte = 1)");
    ASSERT_TEST(cycles == 4, "EI consome 4 ciclos");
    ASSERT_TEST(cpu.pc == 1, "PC avancou 1 byte para EI");

    // Coloca instrucao DI (0xF3) no PC 1
    cpu.memory[1] = 0xF3;
    exec_instrucao(&cpu, &io, &cycles);
    ASSERT_TEST(cpu.inte == 0, "DI desabilita interrupcoes (inte = 0)");
    ASSERT_TEST(cycles == 4, "DI consome 4 ciclos");
    ASSERT_TEST(cpu.pc == 2, "PC avancou 1 byte para DI");
}

void test_interrupt_handling() {
    printf("\n--- Teste 3: Gestao de Interrupcoes (QA-S1-03) ---\n");
    State8080 cpu;
    init_cpu(&cpu);

    cpu.pc = 0x0500;
    cpu.sp = 0x2400;

    // Teste 3A: Interrupcao com inte == 0 deve ser REJEITADA
    int res = disparar_interrupcao(&cpu, 0xCF); // RST 1
    ASSERT_TEST(res == 0, "Interrupcao rejeitada quando inte == 0");
    ASSERT_TEST(cpu.pc == 0x0500, "PC inalterado quando interrupcao rejeitada");
    ASSERT_TEST(cpu.sp == 0x2400, "SP inalterado quando interrupcao rejeitada");

    // Teste 3B: Interrupcao RST 1 com inte == 1
    cpu.inte = 1;
    res = disparar_interrupcao(&cpu, 0xCF);
    ASSERT_TEST(res == 1, "RST 1 aceito quando inte == 1");
    ASSERT_TEST(cpu.inte == 0, "inte desabilitado automaticamente apos interrupcao");
    ASSERT_TEST(cpu.pc == 0x0008, "PC desviado para 0x0008 (vetor RST 1)");
    ASSERT_TEST(cpu.sp == 0x23FE, "SP decrementado em 2");
    ASSERT_TEST(cpu.memory[0x23FE] == 0x00 && cpu.memory[0x23FF] == 0x05, "Endereco de retorno 0x0500 salvo na pilha");

    // Teste 3C: Interrupcao RST 2 com inte == 1
    cpu.inte = 1;
    cpu.pc = 0x0650;
    res = disparar_interrupcao(&cpu, 0xD7);
    ASSERT_TEST(res == 1, "RST 2 aceito quando inte == 1");
    ASSERT_TEST(cpu.inte == 0, "inte desabilitado automaticamente apos RST 2");
    ASSERT_TEST(cpu.pc == 0x0010, "PC desviado para 0x0010 (vetor RST 2)");
}

void test_conditional_cycles() {
    printf("\n--- Teste 4: Fidelidade Total de Ciclos Condicionais ---\n");
    State8080 cpu;
    InputOutput io;
    init_cpu(&cpu);
    init_io(&io);

    int cycles = 0;

    // Teste 4A: CALL condicional CNZ (0xC4) quando Z == 1 (condicao FALSA)
    cpu.pc = 0x0100;
    cpu.flags |= (1 << 6); // Z = 1
    cpu.memory[0x0100] = 0xC4; // CNZ 0x0200
    cpu.memory[0x0101] = 0x00;
    cpu.memory[0x0102] = 0x02;
    exec_instrucao(&cpu, &io, &cycles);
    ASSERT_TEST(cycles == 11, "CNZ com condicao FALSA consome exatamente 11 ciclos");
    ASSERT_TEST(cpu.pc == 0x0103, "PC avanca 3 bytes quando CNZ nao e tomado");

    // Teste 4B: CALL condicional CNZ (0xC4) quando Z == 0 (condicao VERDADEIRA)
    cpu.pc = 0x0100;
    cpu.sp = 0x2400;
    cpu.flags &= ~(1 << 6); // Z = 0
    exec_instrucao(&cpu, &io, &cycles);
    ASSERT_TEST(cycles == 17, "CNZ com condicao VERDADEIRA consome exatamente 17 ciclos");
    ASSERT_TEST(cpu.pc == 0x0200, "PC salta para o destino quando CNZ e tomado");

    // Teste 4C: RET condicional RNZ (0xC0) quando Z == 1 (condicao FALSA)
    cpu.pc = 0x0200;
    cpu.flags |= (1 << 6); // Z = 1
    cpu.memory[0x0200] = 0xC0; // RNZ
    exec_instrucao(&cpu, &io, &cycles);
    ASSERT_TEST(cycles == 5, "RNZ com condicao FALSA consome exatamente 5 ciclos");
    ASSERT_TEST(cpu.pc == 0x0201, "PC avanca 1 byte quando RNZ nao e tomado");

    // Teste 4D: RET condicional RNZ (0xC0) quando Z == 0 (condicao VERDADEIRA)
    cpu.pc = 0x0200;
    cpu.flags &= ~(1 << 6); // Z = 0
    exec_instrucao(&cpu, &io, &cycles);
    ASSERT_TEST(cycles == 11, "RNZ com condicao VERDADEIRA consome exatamente 11 ciclos");
    ASSERT_TEST(cpu.pc == 0x0103, "PC restaurado da pilha pelo RET");
}

void test_rom_execution_headless() {
    printf("\n--- Teste 5: Execucao Headless de ROM do Space Invaders (100 quadros) ---\n");
    State8080 cpu;
    InputOutput io;
    init_cpu(&cpu);
    init_io(&io);

    if (carregar_rom(&cpu, "rom/invaders.h", 0x0000) != 0 ||
        carregar_rom(&cpu, "rom/invaders.g", 0x0800) != 0 ||
        carregar_rom(&cpu, "rom/invaders.f", 0x1000) != 0 ||
        carregar_rom(&cpu, "rom/invaders.e", 0x1800) != 0) {
        ASSERT_TEST(0, "Falha ao carregar ROMs do jogo");
        return;
    }

    int total_frames = 100;
    int error = 0;
    long long total_cycles = 0;

    for (int frame = 0; frame < total_frames; frame++) {
        // Meio frame 1: 16.666 ciclos
        int ciclos = 0;
        while (ciclos < 16666) {
            int c = 0;
            if (exec_instrucao(&cpu, &io, &c) != 0) {
                error = 1;
                break;
            }
            ciclos += c;
        }
        total_cycles += ciclos;
        disparar_interrupcao(&cpu, 0xCF); // RST 1

        if (error) break;

        // Meio frame 2: 16.667 ciclos
        ciclos = 0;
        while (ciclos < 16667) {
            int c = 0;
            if (exec_instrucao(&cpu, &io, &c) != 0) {
                error = 1;
                break;
            }
            ciclos += c;
        }
        total_cycles += ciclos;
        disparar_interrupcao(&cpu, 0xD7); // RST 2

        if (error) break;
    }

    ASSERT_TEST(error == 0, "100 frames executados sem erros de opcodes");
    ASSERT_TEST(total_cycles >= (100 * 33333), "Contagem de ciclos acumulados consistente com clock de 2.0 MHz");
    printf("  [INFO] Ciclos totais executados em 100 frames: %lld (esperado: ~%d)\n", total_cycles, 100 * 33333);
}

int main() {
    printf("====================================================\n");
    printf("   TEST SUITE - SPRINT 1 (CPU TIMING & INTERRUPTS)  \n");
    printf("====================================================\n");

    test_initial_state();
    test_ei_di_instructions();
    test_interrupt_handling();
    test_conditional_cycles();
    test_rom_execution_headless();

    printf("\n====================================================\n");
    printf("   RESULTADO: %d/%d TESTES PASSARAM (%.1f%%)\n",
           test_passed, test_total, (100.0 * test_passed) / test_total);
    printf("====================================================\n");

    return (test_passed == test_total) ? 0 : 1;
}
