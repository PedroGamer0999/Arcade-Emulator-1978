#ifndef CPU8080_H
#define CPU8080_H

#include <stdint.h>

// Declaração antecipada para resolver a dependência circular com InputOutput
typedef struct InputOutput InputOutput;

// Estrutura do estado interno da CPU Intel 8080
typedef struct State8080 {
    uint8_t a;
    uint8_t b;
    uint8_t c;
    uint8_t d;
    uint8_t e;
    uint8_t h;
    uint8_t l;
    
    uint16_t sp;
    uint16_t pc;
    
    uint8_t flags; // Bits: [S, Z, 0, A, 0, P, 1, C]
    
    uint8_t memory[0x10000]; // 64KB de memória
} State8080;

// Funções de inicialização e carga de ROM
void init_cpu(State8080 *cpu);
int carregar_rom(State8080 *cpu, const char *caminho, uint16_t endereco);

// Funções auxiliares da CPU
uint16_t get_hl(State8080 *cpu);
void set_hl(State8080 *cpu, uint16_t val);
void atualizar_flags_zsp(State8080 *cpu, uint8_t val);

// Execução de instruções
int exec_instrucao(State8080 *cpu, InputOutput *io);

#endif