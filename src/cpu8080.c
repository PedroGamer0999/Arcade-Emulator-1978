#include <stdio.h>  // Adicionado para FILE, fopen, fread, etc.
#include <string.h> // Para memset
#include "cpu8080.h"
#include "io.h"

typedef struct State8080 State8080;

void init_cpu(State8080 *cpu)
{
    memset(cpu->memory, 0, sizeof(cpu->memory)); // Initialize memory to zero
    cpu->a = 0;
    cpu->b = 0;
    cpu->c = 0;
    cpu->d = 0;
    cpu->e = 0;
    cpu->h = 0;
    cpu->l = 0;
    cpu->sp = 0;
    cpu->pc = 0;
    cpu->flags = 0;
}

int carregar_rom(State8080 *cpu, const char *caminho_arquivo, uint16_t offset)
{
    FILE *arquivo = fopen(caminho_arquivo, "rb");
    if (!arquivo)
    {
        return -1; // Error opening file
    }

    fseek(arquivo, 0, SEEK_END);
    long tamanho_arquivo = ftell(arquivo);
    fseek(arquivo, 0, SEEK_SET);

   if ((size_t)(offset + tamanho_arquivo) > sizeof(cpu->memory))
    {
        fclose(arquivo);
        return -2; // File is too large for memory
    }

    fread(cpu->memory + offset, 1, tamanho_arquivo, arquivo);
    fclose(arquivo);
    return 0; // Success
}

// Atualiza as flags Z (Zero), S (Sign) e P (Paridade) baseadas em um resultado de 8 bits
void atualizar_flags_zsp(State8080 *cpu, uint8_t valor) {
    // Flag Z: 1 se for zero
    // Flag S: 1 se o bit 7 (sinal) estiver ligado
    // Flag P: 1 se o numero de bits '1' for par
    int bits1 = 0;
    for (int i = 0; i < 8; i++) {
        if ((valor >> i) & 1) bits1++;
    }
    
    // Supondo que cpu->flags use a estrutura de bits do 8080:
    // Bit 0: CY, Bit 2: P, Bit 6: Z, Bit 7: S
    if (valor == 0) cpu->flags |= (1 << 6); else cpu->flags &= ~(1 << 6);
    if (valor & 0x80) cpu->flags |= (1 << 7); else cpu->flags &= ~(1 << 7);
    if (bits1 % 2 == 0) cpu->flags |= (1 << 2); else cpu->flags &= ~(1 << 2);
}
uint16_t get_hl(State8080 *cpu) {
    return (cpu->h << 8) | cpu->l;
}

void set_hl(State8080 *cpu, uint16_t valor) {
    cpu->h = (valor >> 8) & 0xFF;
    cpu->l = valor & 0xFF;
}
int exec_instrucao(State8080 *cpu, InputOutput *io)
{
    uint8_t opcode = cpu->memory[cpu->pc];

    switch (opcode)
    {
        // --- 0x00: NOP ---
        case 0x00: // NOP
            cpu->pc += 1;
            break;

        // --- 0xC3: Salto Incondicional ---
        case 0xC3: // JMP addr
        {
            uint16_t addr = cpu->memory[cpu->pc + 1] | (cpu->memory[cpu->pc + 2] << 8);
            cpu->pc = addr;
            break;
        }

        // --- 1. Incrementos e Decrementos de 8 bits (INR / DCR) ---
        case 0x04: cpu->b++; atualizar_flags_zsp(cpu, cpu->b); cpu->pc += 1; break; // INR B
        case 0x0C: cpu->c++; atualizar_flags_zsp(cpu, cpu->c); cpu->pc += 1; break; // INR C
        case 0x14: cpu->d++; atualizar_flags_zsp(cpu, cpu->d); cpu->pc += 1; break; // INR D
        case 0x1C: cpu->e++; atualizar_flags_zsp(cpu, cpu->e); cpu->pc += 1; break; // INR E
        case 0x24: cpu->h++; atualizar_flags_zsp(cpu, cpu->h); cpu->pc += 1; break; // INR H
        case 0x2C: cpu->l++; atualizar_flags_zsp(cpu, cpu->l); cpu->pc += 1; break; // INR L
        case 0x3C: cpu->a++; atualizar_flags_zsp(cpu, cpu->a); cpu->pc += 1; break; // INR A

        case 0x05: cpu->b--; atualizar_flags_zsp(cpu, cpu->b); cpu->pc += 1; break; // DCR B
        case 0x0D: cpu->c--; atualizar_flags_zsp(cpu, cpu->c); cpu->pc += 1; break; // DCR C
        case 0x15: cpu->d--; atualizar_flags_zsp(cpu, cpu->d); cpu->pc += 1; break; // DCR D
        case 0x1D: cpu->e--; atualizar_flags_zsp(cpu, cpu->e); cpu->pc += 1; break; // DCR E
        case 0x25: cpu->h--; atualizar_flags_zsp(cpu, cpu->h); cpu->pc += 1; break; // DCR H
        case 0x2D: cpu->l--; atualizar_flags_zsp(cpu, cpu->l); cpu->pc += 1; break; // DCR L
        case 0x3D: cpu->a--; atualizar_flags_zsp(cpu, cpu->a); cpu->pc += 1; break; // DCR A

        // --- 2. Incrementos e Decrementos de 16 bits (INX / DCX) ---
        case 0x03: // INX B
        {
            uint16_t bc = (cpu->b << 8) | cpu->c;
            bc++;
            cpu->b = (bc >> 8) & 0xFF;
            cpu->c = bc & 0xFF;
            cpu->pc += 1;
            break;
        }
        case 0x13: // INX D
        {
            uint16_t de = (cpu->d << 8) | cpu->e;
            de++;
            cpu->d = (de >> 8) & 0xFF;
            cpu->e = de & 0xFF;
            cpu->pc += 1;
            break;
        }
        case 0x23: set_hl(cpu, get_hl(cpu) + 1); cpu->pc += 1; break; // INX H
        case 0x33: cpu->sp++; cpu->pc += 1; break;                   // INX SP

        case 0x0B: // DCX B
        {
            uint16_t bc = (cpu->b << 8) | cpu->c;
            bc--;
            cpu->b = (bc >> 8) & 0xFF;
            cpu->c = bc & 0xFF;
            cpu->pc += 1;
            break;
        }
        case 0x1B: // DCX D
        {
            uint16_t de = (cpu->d << 8) | cpu->e;
            de--;
            cpu->d = (de >> 8) & 0xFF;
            cpu->e = de & 0xFF;
            cpu->pc += 1;
            break;
        }
        case 0x2B: set_hl(cpu, get_hl(cpu) - 1); cpu->pc += 1; break; // DCX H
        case 0x3B: cpu->sp--; cpu->pc += 1; break;                   // DCX SP

        // --- ADI d8 (Add Immediate to Accumulator) ---
        case 0xC6:
        {
            uint8_t imediato = cpu->memory[cpu->pc + 1];
            uint16_t resultado = (uint16_t)cpu->a + imediato;

            cpu->a = (uint8_t)(resultado & 0xFF);
            atualizar_flags_zsp(cpu, cpu->a);

            if (resultado > 0xFF) {
                cpu->flags |= (1 << 0);
            } else {
                cpu->flags &= ~(1 << 0);
            }

            cpu->pc += 2;
            break;
        }

        // --- RET (Return from Subroutine) ---
        case 0xC9:
            cpu->pc = cpu->memory[cpu->sp] | (cpu->memory[cpu->sp + 1] << 8);
            cpu->sp += 2;
            break;

        // --- CALL addr (Call Subroutine) ---
        case 0xCD:
        {
            uint16_t ret_addr = cpu->pc + 3;
            cpu->sp -= 2;
            cpu->memory[cpu->sp] = ret_addr & 0xFF;
            cpu->memory[cpu->sp + 1] = (ret_addr >> 8) & 0xFF;
            
            uint16_t target_addr = cpu->memory[cpu->pc + 1] | (cpu->memory[cpu->pc + 2] << 8);
            cpu->pc = target_addr;
            break;
        }

        // --- PUSH B / POP B ---
        case 0xC5: // PUSH B
            cpu->sp -= 2;
            cpu->memory[cpu->sp] = cpu->c;
            cpu->memory[cpu->sp + 1] = cpu->b;
            cpu->pc += 1;
            break;

        case 0xC1: // POP B
            cpu->c = cpu->memory[cpu->sp];
            cpu->b = cpu->memory[cpu->sp + 1];
            cpu->sp += 2;
            cpu->pc += 1;
            break;

        // --- RST (Restart / Software Interrupts) ---
        case 0xC7: // RST 0
            cpu->sp -= 2;
            cpu->memory[cpu->sp] = (cpu->pc + 1) & 0xFF;
            cpu->memory[cpu->sp + 1] = ((cpu->pc + 1) >> 8) & 0xFF;
            cpu->pc = 0x0000;
            break;

        case 0xCF: // RST 1
            cpu->sp -= 2;
            cpu->memory[cpu->sp] = (cpu->pc + 1) & 0xFF;
            cpu->memory[cpu->sp + 1] = ((cpu->pc + 1) >> 8) & 0xFF;
            cpu->pc = 0x0008;
            break;

        case 0xD7: // RST 2
            cpu->sp -= 2;
            cpu->memory[cpu->sp] = (cpu->pc + 1) & 0xFF;
            cpu->memory[cpu->sp + 1] = ((cpu->pc + 1) >> 8) & 0xFF;
            cpu->pc = 0x0010;
            break;

        case 0xDF: // RST 3
            cpu->sp -= 2;
            cpu->memory[cpu->sp] = (cpu->pc + 1) & 0xFF;
            cpu->memory[cpu->sp + 1] = ((cpu->pc + 1) >> 8) & 0xFF;
            cpu->pc = 0x0018;
            break;

        case 0xE7: // RST 4
            cpu->sp -= 2;
            cpu->memory[cpu->sp] = (cpu->pc + 1) & 0xFF;
            cpu->memory[cpu->sp + 1] = ((cpu->pc + 1) >> 8) & 0xFF;
            cpu->pc = 0x0020;
            break;

        case 0xEF: // RST 5
            cpu->sp -= 2;
            cpu->memory[cpu->sp] = (cpu->pc + 1) & 0xFF;
            cpu->memory[cpu->sp + 1] = ((cpu->pc + 1) >> 8) & 0xFF;
            cpu->pc = 0x0028;
            break;

        case 0xF7: // RST 6
            cpu->sp -= 2;
            cpu->memory[cpu->sp] = (cpu->pc + 1) & 0xFF;
            cpu->memory[cpu->sp + 1] = ((cpu->pc + 1) >> 8) & 0xFF;
            cpu->pc = 0x0030;
            break;

        case 0xFF: // RST 7
            cpu->sp -= 2;
            cpu->memory[cpu->sp] = (cpu->pc + 1) & 0xFF;
            cpu->memory[cpu->sp + 1] = ((cpu->pc + 1) >> 8) & 0xFF;
            cpu->pc = 0x0038;
            break;

        // --- STA / LDA ---
        case 0x32: // STA addr
        {
            uint16_t addr = cpu->memory[cpu->pc + 1] | (cpu->memory[cpu->pc + 2] << 8);
            cpu->memory[addr] = cpu->a;
            cpu->pc += 3;
            break;
        }

        case 0x3A: // LDA addr
        {
            uint16_t addr = cpu->memory[cpu->pc + 1] | (cpu->memory[cpu->pc + 2] << 8);
            cpu->a = cpu->memory[addr];
            cpu->pc += 3;
            break;
        }

        // --- NOP e NOPs Não Documentados ---
        case 0x08:
        case 0x10:
        case 0x18:
        case 0x20:
        case 0x28:
        case 0x30:
        case 0x38:
            cpu->pc += 1;
            break;

        // --- Rotações e Shifts ---
        case 0x07: // RLC
        {
            uint8_t bit7 = (cpu->a & 0x80) >> 7;
            cpu->a = (cpu->a << 1) | bit7;
            if (bit7) cpu->flags |= (1 << 0);
            else cpu->flags &= ~(1 << 0);
            cpu->pc += 1;
            break;
        }

        case 0x0F: // RRC
        {
            uint8_t bit0 = cpu->a & 0x01;
            cpu->a = (cpu->a >> 1) | (bit0 << 7);
            if (bit0) cpu->flags |= (1 << 0);
            else cpu->flags &= ~(1 << 0);
            cpu->pc += 1;
            break;
        }

        case 0x17: // RAL
        {
            uint8_t old_carry = cpu->flags & 0x01;
            uint8_t bit7 = (cpu->a & 0x80) >> 7;
            cpu->a = (cpu->a << 1) | old_carry;
            if (bit7) cpu->flags |= (1 << 0);
            else cpu->flags &= ~(1 << 0);
            cpu->pc += 1;
            break;
        }

        case 0x1F: // RAR
        {
            uint8_t old_carry = cpu->flags & 0x01;
            uint8_t bit0 = cpu->a & 0x01;
            cpu->a = (cpu->a >> 1) | (old_carry << 7);
            if (bit0) cpu->flags |= (1 << 0);
            else cpu->flags &= ~(1 << 0);
            cpu->pc += 1;
            break;
        }

        // --- Lógica (ORA / ORI / XRA / CMA / ANA / ANI) ---
        case 0xB0: cpu->a |= cpu->b; cpu->flags &= ~(1 << 0); atualizar_flags_zsp(cpu, cpu->a); cpu->pc += 1; break;
        case 0xB1: cpu->a |= cpu->c; cpu->flags &= ~(1 << 0); atualizar_flags_zsp(cpu, cpu->a); cpu->pc += 1; break;
        case 0xB2: cpu->a |= cpu->d; cpu->flags &= ~(1 << 0); atualizar_flags_zsp(cpu, cpu->a); cpu->pc += 1; break;
        case 0xB3: cpu->a |= cpu->e; cpu->flags &= ~(1 << 0); atualizar_flags_zsp(cpu, cpu->a); cpu->pc += 1; break;
        case 0xB4: cpu->a |= cpu->h; cpu->flags &= ~(1 << 0); atualizar_flags_zsp(cpu, cpu->a); cpu->pc += 1; break;
        case 0xB5: cpu->a |= cpu->l; cpu->flags &= ~(1 << 0); atualizar_flags_zsp(cpu, cpu->a); cpu->pc += 1; break;
        case 0xB6: cpu->a |= cpu->memory[get_hl(cpu)]; cpu->flags &= ~(1 << 0); atualizar_flags_zsp(cpu, cpu->a); cpu->pc += 1; break;
        case 0xB7: cpu->a |= cpu->a; cpu->flags &= ~(1 << 0); atualizar_flags_zsp(cpu, cpu->a); cpu->pc += 1; break;

        case 0xF6: // ORI d8
            cpu->a |= cpu->memory[cpu->pc + 1];
            cpu->flags &= ~(1 << 0);
            atualizar_flags_zsp(cpu, cpu->a);
            cpu->pc += 2;
            break;

        case 0xA8: cpu->a ^= cpu->b; cpu->flags &= ~(1 << 0); atualizar_flags_zsp(cpu, cpu->a); cpu->pc += 1; break;
        case 0xA9: cpu->a ^= cpu->c; cpu->flags &= ~(1 << 0); atualizar_flags_zsp(cpu, cpu->a); cpu->pc += 1; break;
        case 0xAA: cpu->a ^= cpu->d; cpu->flags &= ~(1 << 0); atualizar_flags_zsp(cpu, cpu->a); cpu->pc += 1; break;
        case 0xAB: cpu->a ^= cpu->e; cpu->flags &= ~(1 << 0); atualizar_flags_zsp(cpu, cpu->a); cpu->pc += 1; break;
        case 0xAC: cpu->a ^= cpu->h; cpu->flags &= ~(1 << 0); atualizar_flags_zsp(cpu, cpu->a); cpu->pc += 1; break;
        case 0xAD: cpu->a ^= cpu->l; cpu->flags &= ~(1 << 0); atualizar_flags_zsp(cpu, cpu->a); cpu->pc += 1; break;
        case 0xAE: cpu->a ^= cpu->memory[get_hl(cpu)]; cpu->flags &= ~(1 << 0); atualizar_flags_zsp(cpu, cpu->a); cpu->pc += 1; break;
        case 0xAF: cpu->a ^= cpu->a; cpu->flags &= ~(1 << 0); atualizar_flags_zsp(cpu, cpu->a); cpu->pc += 1; break;

        case 0x2F: cpu->a = ~cpu->a; cpu->pc += 1; break; // CMA

        case 0xA0: cpu->a &= cpu->b; cpu->flags &= ~(1 << 0); atualizar_flags_zsp(cpu, cpu->a); cpu->pc += 1; break;
        case 0xA1: cpu->a &= cpu->c; cpu->flags &= ~(1 << 0); atualizar_flags_zsp(cpu, cpu->a); cpu->pc += 1; break;
        case 0xA2: cpu->a &= cpu->d; cpu->flags &= ~(1 << 0); atualizar_flags_zsp(cpu, cpu->a); cpu->pc += 1; break;
        case 0xA3: cpu->a &= cpu->e; cpu->flags &= ~(1 << 0); atualizar_flags_zsp(cpu, cpu->a); cpu->pc += 1; break;
        case 0xA4: cpu->a &= cpu->h; cpu->flags &= ~(1 << 0); atualizar_flags_zsp(cpu, cpu->a); cpu->pc += 1; break;
        case 0xA5: cpu->a &= cpu->l; cpu->flags &= ~(1 << 0); atualizar_flags_zsp(cpu, cpu->a); cpu->pc += 1; break;
        case 0xA6: cpu->a &= cpu->memory[get_hl(cpu)]; cpu->flags &= ~(1 << 0); atualizar_flags_zsp(cpu, cpu->a); cpu->pc += 1; break;
        case 0xA7: cpu->a &= cpu->a; cpu->flags &= ~(1 << 0); atualizar_flags_zsp(cpu, cpu->a); cpu->pc += 1; break;

        case 0xE6: // ANI
            cpu->a &= cpu->memory[cpu->pc + 1];
            cpu->flags &= ~(1 << 0);
            atualizar_flags_zsp(cpu, cpu->a);
            cpu->pc += 2;
            break;

        // --- MOV B, r / MOV C, r / MOV D, r / MOV E, r / MOV H, r / MOV L, r / MOV A, r ---
        case 0x40: cpu->b = cpu->b; cpu->pc += 1; break;
        case 0x41: cpu->b = cpu->c; cpu->pc += 1; break;
        case 0x42: cpu->b = cpu->d; cpu->pc += 1; break;
        case 0x43: cpu->b = cpu->e; cpu->pc += 1; break;
        case 0x44: cpu->b = cpu->h; cpu->pc += 1; break;
        case 0x45: cpu->b = cpu->l; cpu->pc += 1; break;
        case 0x46: cpu->b = cpu->memory[get_hl(cpu)]; cpu->pc += 1; break;
        case 0x47: cpu->b = cpu->a; cpu->pc += 1; break;

        case 0x48: cpu->c = cpu->b; cpu->pc += 1; break;
        case 0x49: cpu->c = cpu->c; cpu->pc += 1; break;
        case 0x4A: cpu->c = cpu->d; cpu->pc += 1; break;
        case 0x4B: cpu->c = cpu->e; cpu->pc += 1; break;
        case 0x4C: cpu->c = cpu->h; cpu->pc += 1; break;
        case 0x4D: cpu->c = cpu->l; cpu->pc += 1; break;
        case 0x4E: cpu->c = cpu->memory[get_hl(cpu)]; cpu->pc += 1; break;
        case 0x4F: cpu->c = cpu->a; cpu->pc += 1; break;

        case 0x50: cpu->d = cpu->b; cpu->pc += 1; break;
        case 0x51: cpu->d = cpu->c; cpu->pc += 1; break;
        case 0x52: cpu->d = cpu->d; cpu->pc += 1; break;
        case 0x53: cpu->d = cpu->e; cpu->pc += 1; break;
        case 0x54: cpu->d = cpu->h; cpu->pc += 1; break;
        case 0x55: cpu->d = cpu->l; cpu->pc += 1; break;
        case 0x56: cpu->d = cpu->memory[get_hl(cpu)]; cpu->pc += 1; break;
        case 0x57: cpu->d = cpu->a; cpu->pc += 1; break;

        case 0x58: cpu->e = cpu->b; cpu->pc += 1; break;
        case 0x59: cpu->e = cpu->c; cpu->pc += 1; break;
        case 0x5A: cpu->e = cpu->d; cpu->pc += 1; break;
        case 0x5B: cpu->e = cpu->e; cpu->pc += 1; break;
        case 0x5C: cpu->e = cpu->h; cpu->pc += 1; break;
        case 0x5D: cpu->e = cpu->l; cpu->pc += 1; break;
        case 0x5E: cpu->e = cpu->memory[get_hl(cpu)]; cpu->pc += 1; break;
        case 0x5F: cpu->e = cpu->a; cpu->pc += 1; break;

        case 0x60: cpu->h = cpu->b; cpu->pc += 1; break;
        case 0x61: cpu->h = cpu->c; cpu->pc += 1; break;
        case 0x62: cpu->h = cpu->d; cpu->pc += 1; break;
        case 0x63: cpu->h = cpu->e; cpu->pc += 1; break;
        case 0x64: cpu->h = cpu->h; cpu->pc += 1; break;
        case 0x65: cpu->h = cpu->l; cpu->pc += 1; break;
        case 0x66: cpu->h = cpu->memory[get_hl(cpu)]; cpu->pc += 1; break;
        case 0x67: cpu->h = cpu->a; cpu->pc += 1; break;

        case 0x68: cpu->l = cpu->b; cpu->pc += 1; break;
        case 0x69: cpu->l = cpu->c; cpu->pc += 1; break;
        case 0x6A: cpu->l = cpu->d; cpu->pc += 1; break;
        case 0x6B: cpu->l = cpu->e; cpu->pc += 1; break;
        case 0x6C: cpu->l = cpu->h; cpu->pc += 1; break;
        case 0x6D: cpu->l = cpu->l; cpu->pc += 1; break;
        case 0x6E: cpu->l = cpu->memory[get_hl(cpu)]; cpu->pc += 1; break;
        case 0x6F: cpu->l = cpu->a; cpu->pc += 1; break;

        case 0x78: cpu->a = cpu->b; cpu->pc += 1; break;
        case 0x79: cpu->a = cpu->c; cpu->pc += 1; break;
        case 0x7A: cpu->a = cpu->d; cpu->pc += 1; break;
        case 0x7B: cpu->a = cpu->e; cpu->pc += 1; break;
        case 0x7C: cpu->a = cpu->h; cpu->pc += 1; break;
        case 0x7D: cpu->a = cpu->l; cpu->pc += 1; break;
        case 0x7E: cpu->a = cpu->memory[get_hl(cpu)]; cpu->pc += 1; break;
        case 0x7F: cpu->a = cpu->a; cpu->pc += 1; break;

        // --- LXI / LHLD / SHLD ---
        case 0x01: cpu->c = cpu->memory[cpu->pc + 1]; cpu->b = cpu->memory[cpu->pc + 2]; cpu->pc += 3; break;
        case 0x11: cpu->e = cpu->memory[cpu->pc + 1]; cpu->d = cpu->memory[cpu->pc + 2]; cpu->pc += 3; break;
        case 0x21: cpu->l = cpu->memory[cpu->pc + 1]; cpu->h = cpu->memory[cpu->pc + 2]; cpu->pc += 3; break;
        case 0x31: cpu->sp = cpu->memory[cpu->pc + 1] | (cpu->memory[cpu->pc + 2] << 8); cpu->pc += 3; break;

        case 0x2A: // LHLD addr
        {
            uint16_t addr = cpu->memory[cpu->pc + 1] | (cpu->memory[cpu->pc + 2] << 8);
            cpu->l = cpu->memory[addr];
            cpu->h = cpu->memory[addr + 1];
            cpu->pc += 3;
            break;
        }

        case 0x22: // SHLD addr
        {
            uint16_t addr = cpu->memory[cpu->pc + 1] | (cpu->memory[cpu->pc + 2] << 8);
            cpu->memory[addr] = cpu->l;
            cpu->memory[addr + 1] = cpu->h;
            cpu->pc += 3;
            break;
        }

        // --- LDAX / STAX ---
        case 0x0A: cpu->a = cpu->memory[(cpu->b << 8) | cpu->c]; cpu->pc += 1; break;
        case 0x1A: cpu->a = cpu->memory[(cpu->d << 8) | cpu->e]; cpu->pc += 1; break;
        case 0x02: cpu->memory[(cpu->b << 8) | cpu->c] = cpu->a; cpu->pc += 1; break;
        case 0x12: cpu->memory[(cpu->d << 8) | cpu->e] = cpu->a; cpu->pc += 1; break;

        // --- DAA ---
        case 0x27:
        {
            uint8_t lsb = cpu->a & 0x0F;
            uint8_t msb = (cpu->a & 0xF0) >> 4;
            uint8_t add = 0;

            if (lsb > 9) add += 0x06;
            if (msb > 9 || (msb >= 9 && lsb > 9)) {
                add += 0x60;
                cpu->flags |= (1 << 0);
            }

            uint16_t res = cpu->a + add;
            cpu->a = (uint8_t)(res & 0xFF);
            atualizar_flags_zsp(cpu, cpu->a);
            cpu->pc += 1;
            break;
        }

        // --- PILHA E TROCAS ---
        case 0xD5: cpu->sp -= 2; cpu->memory[cpu->sp] = cpu->e; cpu->memory[cpu->sp + 1] = cpu->d; cpu->pc += 1; break;
        case 0xE5: cpu->sp -= 2; cpu->memory[cpu->sp] = cpu->l; cpu->memory[cpu->sp + 1] = cpu->h; cpu->pc += 1; break;
        case 0xF5: cpu->sp -= 2; cpu->memory[cpu->sp] = cpu->flags; cpu->memory[cpu->sp + 1] = cpu->a; cpu->pc += 1; break;
        case 0xD1: cpu->e = cpu->memory[cpu->sp]; cpu->d = cpu->memory[cpu->sp + 1]; cpu->sp += 2; cpu->pc += 1; break;
        case 0xE1: cpu->l = cpu->memory[cpu->sp]; cpu->h = cpu->memory[cpu->sp + 1]; cpu->sp += 2; cpu->pc += 1; break;
        case 0xF1: cpu->flags = cpu->memory[cpu->sp]; cpu->a = cpu->memory[cpu->sp + 1]; cpu->sp += 2; cpu->pc += 1; break;

        case 0xEB:
        {
            uint8_t tmp_h = cpu->h, tmp_l = cpu->l;
            cpu->h = cpu->d; cpu->l = cpu->e;
            cpu->d = tmp_h; cpu->e = tmp_l;
            cpu->pc += 1;
            break;
        }

        case 0xE3:
        {
            uint8_t tmp_l = cpu->memory[cpu->sp];
            uint8_t tmp_h = cpu->memory[cpu->sp + 1];
            cpu->memory[cpu->sp] = cpu->l;
            cpu->memory[cpu->sp + 1] = cpu->h;
            cpu->l = tmp_l; cpu->h = tmp_h;
            cpu->pc += 1;
            break;
        }

        case 0xF9: cpu->sp = get_hl(cpu); cpu->pc += 1; break;

        // --- SALTOS CONDICIONAIS ---
        case 0xC2:
            if (!(cpu->flags & (1 << 6))) cpu->pc = cpu->memory[cpu->pc + 1] | (cpu->memory[cpu->pc + 2] << 8);
            else cpu->pc += 3;
            break;
        case 0xCA:
            if (cpu->flags & (1 << 6)) cpu->pc = cpu->memory[cpu->pc + 1] | (cpu->memory[cpu->pc + 2] << 8);
            else cpu->pc += 3;
            break;
        case 0xD2:
            if (!(cpu->flags & (1 << 0))) cpu->pc = cpu->memory[cpu->pc + 1] | (cpu->memory[cpu->pc + 2] << 8);
            else cpu->pc += 3;
            break;
        case 0xDA:
            if (cpu->flags & (1 << 0)) cpu->pc = cpu->memory[cpu->pc + 1] | (cpu->memory[cpu->pc + 2] << 8);
            else cpu->pc += 3;
            break;

        // --- RETORNOS CONDICIONAIS ---
        case 0xC0:
            if (!(cpu->flags & (1 << 6))) {
                cpu->pc = cpu->memory[cpu->sp] | (cpu->memory[cpu->sp + 1] << 8);
                cpu->sp += 2;
            } else cpu->pc += 1;
            break;
        case 0xC8:
            if (cpu->flags & (1 << 6)) {
                cpu->pc = cpu->memory[cpu->sp] | (cpu->memory[cpu->sp + 1] << 8);
                cpu->sp += 2;
            } else cpu->pc += 1;
            break;
        case 0xD0:
            if (!(cpu->flags & (1 << 0))) {
                cpu->pc = cpu->memory[cpu->sp] | (cpu->memory[cpu->sp + 1] << 8);
                cpu->sp += 2;
            } else cpu->pc += 1;
            break;
        case 0xD8:
            if (cpu->flags & (1 << 0)) {
                cpu->pc = cpu->memory[cpu->sp] | (cpu->memory[cpu->sp + 1] << 8);
                cpu->sp += 2;
            } else cpu->pc += 1;
            break;

        // --- ARITMÉTICA (ADD / SUB / CPI) ---
        case 0x80: cpu->a += cpu->b; atualizar_flags_zsp(cpu, cpu->a); cpu->pc += 1; break;
        case 0x81: cpu->a += cpu->c; atualizar_flags_zsp(cpu, cpu->a); cpu->pc += 1; break;
        case 0x82: cpu->a += cpu->d; atualizar_flags_zsp(cpu, cpu->a); cpu->pc += 1; break;
        case 0x83: cpu->a += cpu->e; atualizar_flags_zsp(cpu, cpu->a); cpu->pc += 1; break;
        case 0x84: cpu->a += cpu->h; atualizar_flags_zsp(cpu, cpu->a); cpu->pc += 1; break;
        case 0x85: cpu->a += cpu->l; atualizar_flags_zsp(cpu, cpu->a); cpu->pc += 1; break;
        case 0x86: cpu->a += cpu->memory[get_hl(cpu)]; atualizar_flags_zsp(cpu, cpu->a); cpu->pc += 1; break;
        case 0x87: cpu->a += cpu->a; atualizar_flags_zsp(cpu, cpu->a); cpu->pc += 1; break;

        case 0x90: cpu->a -= cpu->b; atualizar_flags_zsp(cpu, cpu->a); cpu->pc += 1; break;
        case 0x91: cpu->a -= cpu->c; atualizar_flags_zsp(cpu, cpu->a); cpu->pc += 1; break;
        case 0x92: cpu->a -= cpu->d; atualizar_flags_zsp(cpu, cpu->a); cpu->pc += 1; break;
        case 0x93: cpu->a -= cpu->e; atualizar_flags_zsp(cpu, cpu->a); cpu->pc += 1; break;
        case 0x94: cpu->a -= cpu->h; atualizar_flags_zsp(cpu, cpu->a); cpu->pc += 1; break;
        case 0x95: cpu->a -= cpu->l; atualizar_flags_zsp(cpu, cpu->a); cpu->pc += 1; break;
        case 0x96: cpu->a -= cpu->memory[get_hl(cpu)]; atualizar_flags_zsp(cpu, cpu->a); cpu->pc += 1; break;
        case 0x97: cpu->a -= cpu->a; atualizar_flags_zsp(cpu, cpu->a); cpu->pc += 1; break;

        case 0xFE: // CPI d8
        {
            uint8_t imediato = cpu->memory[cpu->pc + 1];
            uint8_t res = cpu->a - imediato;
            atualizar_flags_zsp(cpu, res);
            if (cpu->a < imediato) cpu->flags |= (1 << 0);
            else cpu->flags &= ~(1 << 0);
            cpu->pc += 2;
            break;
        }

        // --- INTERRUPÇÕES E I/O ---
        case 0xFB: cpu->pc += 1; break; // EI
        case 0xF3: cpu->pc += 1; break; // DI
        case 0xDB: // IN port
        {
            uint8_t port = cpu->memory[cpu->pc + 1];
            cpu->a = io_in(io, port);
            cpu->pc += 2;
            break;
        }

        case 0xD3: // OUT port
        {
            uint8_t port = cpu->memory[cpu->pc + 1];
            io_out(io, port, cpu->a);
            cpu->pc += 2;
            break;
        }

        // --- ADC / SBB ---
        case 0x88: { uint8_t cy = (cpu->flags & 1); uint16_t res = cpu->a + cpu->b + cy; cpu->a = res & 0xFF; atualizar_flags_zsp(cpu, cpu->a); if (res > 0xFF) cpu->flags |= 1; else cpu->flags &= ~1; cpu->pc += 1; break; }
        case 0x89: { uint8_t cy = (cpu->flags & 1); uint16_t res = cpu->a + cpu->c + cy; cpu->a = res & 0xFF; atualizar_flags_zsp(cpu, cpu->a); if (res > 0xFF) cpu->flags |= 1; else cpu->flags &= ~1; cpu->pc += 1; break; }
        case 0x8A: { uint8_t cy = (cpu->flags & 1); uint16_t res = cpu->a + cpu->d + cy; cpu->a = res & 0xFF; atualizar_flags_zsp(cpu, cpu->a); if (res > 0xFF) cpu->flags |= 1; else cpu->flags &= ~1; cpu->pc += 1; break; }
        case 0x8B: { uint8_t cy = (cpu->flags & 1); uint16_t res = cpu->a + cpu->e + cy; cpu->a = res & 0xFF; atualizar_flags_zsp(cpu, cpu->a); if (res > 0xFF) cpu->flags |= 1; else cpu->flags &= ~1; cpu->pc += 1; break; }
        case 0x8C: { uint8_t cy = (cpu->flags & 1); uint16_t res = cpu->a + cpu->h + cy; cpu->a = res & 0xFF; atualizar_flags_zsp(cpu, cpu->a); if (res > 0xFF) cpu->flags |= 1; else cpu->flags &= ~1; cpu->pc += 1; break; }
        case 0x8D: { uint8_t cy = (cpu->flags & 1); uint16_t res = cpu->a + cpu->l + cy; cpu->a = res & 0xFF; atualizar_flags_zsp(cpu, cpu->a); if (res > 0xFF) cpu->flags |= 1; else cpu->flags &= ~1; cpu->pc += 1; break; }
        case 0x8E: { uint8_t cy = (cpu->flags & 1); uint16_t res = cpu->a + cpu->memory[get_hl(cpu)] + cy; cpu->a = res & 0xFF; atualizar_flags_zsp(cpu, cpu->a); if (res > 0xFF) cpu->flags |= 1; else cpu->flags &= ~1; cpu->pc += 1; break; }
        case 0x8F: { uint8_t cy = (cpu->flags & 1); uint16_t res = cpu->a + cpu->a + cy; cpu->a = res & 0xFF; atualizar_flags_zsp(cpu, cpu->a); if (res > 0xFF) cpu->flags |= 1; else cpu->flags &= ~1; cpu->pc += 1; break; }

        case 0x98: { uint8_t cy = (cpu->flags & 1); uint16_t res = cpu->a - cpu->b - cy; cpu->a = res & 0xFF; atualizar_flags_zsp(cpu, cpu->a); if (res > 0xFF) cpu->flags |= 1; else cpu->flags &= ~1; cpu->pc += 1; break; }
        case 0x99: { uint8_t cy = (cpu->flags & 1); uint16_t res = cpu->a - cpu->c - cy; cpu->a = res & 0xFF; atualizar_flags_zsp(cpu, cpu->a); if (res > 0xFF) cpu->flags |= 1; else cpu->flags &= ~1; cpu->pc += 1; break; }
        case 0x9A: { uint8_t cy = (cpu->flags & 1); uint16_t res = cpu->a - cpu->d - cy; cpu->a = res & 0xFF; atualizar_flags_zsp(cpu, cpu->a); if (res > 0xFF) cpu->flags |= 1; else cpu->flags &= ~1; cpu->pc += 1; break; }
        case 0x9B: { uint8_t cy = (cpu->flags & 1); uint16_t res = cpu->a - cpu->e - cy; cpu->a = res & 0xFF; atualizar_flags_zsp(cpu, cpu->a); if (res > 0xFF) cpu->flags |= 1; else cpu->flags &= ~1; cpu->pc += 1; break; }
        case 0x9C: { uint8_t cy = (cpu->flags & 1); uint16_t res = cpu->a - cpu->h - cy; cpu->a = res & 0xFF; atualizar_flags_zsp(cpu, cpu->a); if (res > 0xFF) cpu->flags |= 1; else cpu->flags &= ~1; cpu->pc += 1; break; }
        case 0x9D: { uint8_t cy = (cpu->flags & 1); uint16_t res = cpu->a - cpu->l - cy; cpu->a = res & 0xFF; atualizar_flags_zsp(cpu, cpu->a); if (res > 0xFF) cpu->flags |= 1; else cpu->flags &= ~1; cpu->pc += 1; break; }
        case 0x9E: { uint8_t cy = (cpu->flags & 1); uint16_t res = cpu->a - cpu->memory[get_hl(cpu)] - cy; cpu->a = res & 0xFF; atualizar_flags_zsp(cpu, cpu->a); if (res > 0xFF) cpu->flags |= 1; else cpu->flags &= ~1; cpu->pc += 1; break; }
        case 0x9F: { uint8_t cy = (cpu->flags & 1); uint16_t res = cpu->a - cpu->a - cy; cpu->a = res & 0xFF; atualizar_flags_zsp(cpu, cpu->a); if (res > 0xFF) cpu->flags |= 1; else cpu->flags &= ~1; cpu->pc += 1; break; }

        // --- DAD (Add Register Pair to HL) ---
        case 0x09: { uint32_t res = (uint32_t)get_hl(cpu) + ((cpu->b << 8) | cpu->c); set_hl(cpu, res & 0xFFFF); if (res > 0xFFFF) cpu->flags |= 1; else cpu->flags &= ~1; cpu->pc += 1; break; }
        case 0x19: { uint32_t res = (uint32_t)get_hl(cpu) + ((cpu->d << 8) | cpu->e); set_hl(cpu, res & 0xFFFF); if (res > 0xFFFF) cpu->flags |= 1; else cpu->flags &= ~1; cpu->pc += 1; break; }
        case 0x29: { uint32_t res = (uint32_t)get_hl(cpu) + get_hl(cpu); set_hl(cpu, res & 0xFFFF); if (res > 0xFFFF) cpu->flags |= 1; else cpu->flags &= ~1; cpu->pc += 1; break; }
        // --- MVI r, d8 (Move Immediate) ---
        case 0x06: // MVI B, d8
            cpu->b = cpu->memory[cpu->pc + 1];
            cpu->pc += 2;
            break;

        case 0x0E: // MVI C, d8
            cpu->c = cpu->memory[cpu->pc + 1];
            cpu->pc += 2;
            break;

        case 0x16: // MVI D, d8
            cpu->d = cpu->memory[cpu->pc + 1];
            cpu->pc += 2;
            break;

        case 0x1E: // MVI E, d8
            cpu->e = cpu->memory[cpu->pc + 1];
            cpu->pc += 2;
            break;

        case 0x26: // MVI H, d8
            cpu->h = cpu->memory[cpu->pc + 1];
            cpu->pc += 2;
            break;

        case 0x2E: // MVI L, d8
            cpu->l = cpu->memory[cpu->pc + 1];
            cpu->pc += 2;
            break;

        case 0x36: // MVI M, d8
            cpu->memory[get_hl(cpu)] = cpu->memory[cpu->pc + 1];
            cpu->pc += 2;
            break;

        case 0x3E: // MVI A, d8
            cpu->a = cpu->memory[cpu->pc + 1];
            cpu->pc += 2;
            break;
        // --- MOV M, r (Move Register to Memory) ---
        case 0x70: cpu->memory[get_hl(cpu)] = cpu->b; cpu->pc += 1; break; // MOV M, B
        case 0x71: cpu->memory[get_hl(cpu)] = cpu->c; cpu->pc += 1; break; // MOV M, C
        case 0x72: cpu->memory[get_hl(cpu)] = cpu->d; cpu->pc += 1; break; // MOV M, D
        case 0x73: cpu->memory[get_hl(cpu)] = cpu->e; cpu->pc += 1; break; // MOV M, E
        case 0x74: cpu->memory[get_hl(cpu)] = cpu->h; cpu->pc += 1; break; // MOV M, H
        case 0x75: cpu->memory[get_hl(cpu)] = cpu->l; cpu->pc += 1; break; // MOV M, L
        case 0x76: cpu->pc += 1; break;                                    // HLT (Halt)
        case 0x77: cpu->memory[get_hl(cpu)] = cpu->a; cpu->pc += 1; break; // MOV M, A
        case 0x39: { // DAD SP
            uint32_t res = (uint32_t)get_hl(cpu) + cpu->sp;
            set_hl(cpu, res & 0xFFFF);
            if (res > 0xFFFF) cpu->flags |= 1; else cpu->flags &= ~1;
            cpu->pc += 1;
            break;
        }
        // --- DCR M (Decrement Memory pointed by HL) ---
        case 0x35:
        {
            uint16_t end_hl = get_hl(cpu);
            cpu->memory[end_hl]--;
            atualizar_flags_zsp(cpu, cpu->memory[end_hl]);
            cpu->pc += 1;
            break;
        }
        // --- INR M (Increment Memory pointed by HL) ---
        case 0x34:
        {
            uint16_t end_hl = get_hl(cpu);
            cpu->memory[end_hl]++;
            atualizar_flags_zsp(cpu, cpu->memory[end_hl]);
            cpu->pc += 1;
            break;
        }
        // --- STC / CMC (Set/Complement Carry) ---
        case 0x37: // STC (Set Carry)
            cpu->flags |= (1 << 0);
            cpu->pc += 1;
            break;

        case 0x3F: // CMC (Complement Carry)
            cpu->flags ^= (1 << 0);
            cpu->pc += 1;
            break;
        // --- CALLs Condicionais ---
        case 0xC4: // CNZ addr (Call if Not Zero)
        {
            uint16_t target = cpu->memory[cpu->pc + 1] | (cpu->memory[cpu->pc + 2] << 8);
            if (!(cpu->flags & (1 << 6))) { // Se Z == 0
                uint16_t ret_addr = cpu->pc + 3;
                cpu->sp -= 2;
                cpu->memory[cpu->sp] = ret_addr & 0xFF;
                cpu->memory[cpu->sp + 1] = (ret_addr >> 8) & 0xFF;
                cpu->pc = target;
            } else {
                cpu->pc += 3;
            }
            break;
        }

        case 0xCC: // CZ addr (Call if Zero)
        {
            uint16_t target = cpu->memory[cpu->pc + 1] | (cpu->memory[cpu->pc + 2] << 8);
            if (cpu->flags & (1 << 6)) { // Se Z == 1
                uint16_t ret_addr = cpu->pc + 3;
                cpu->sp -= 2;
                cpu->memory[cpu->sp] = ret_addr & 0xFF;
                cpu->memory[cpu->sp + 1] = (ret_addr >> 8) & 0xFF;
                cpu->pc = target;
            } else {
                cpu->pc += 3;
            }
            break;
        }

        case 0xD4: // CNC addr (Call if No Carry)
        {
            uint16_t target = cpu->memory[cpu->pc + 1] | (cpu->memory[cpu->pc + 2] << 8);
            if (!(cpu->flags & (1 << 0))) { // Se CY == 0
                uint16_t ret_addr = cpu->pc + 3;
                cpu->sp -= 2;
                cpu->memory[cpu->sp] = ret_addr & 0xFF;
                cpu->memory[cpu->sp + 1] = (ret_addr >> 8) & 0xFF;
                cpu->pc = target;
            } else {
                cpu->pc += 3;
            }
            break;
        }

        case 0xDC: // CC addr (Call if Carry)
        {
            uint16_t target = cpu->memory[cpu->pc + 1] | (cpu->memory[cpu->pc + 2] << 8);
            if (cpu->flags & (1 << 0)) { // Se CY == 1
                uint16_t ret_addr = cpu->pc + 3;
                cpu->sp -= 2;
                cpu->memory[cpu->sp] = ret_addr & 0xFF;
                cpu->memory[cpu->sp + 1] = (ret_addr >> 8) & 0xFF;
                cpu->pc = target;
            } else {
                cpu->pc += 3;
            }
            break;
        }
        // --- PCHL (Load PC with HL / Jump to address in HL) ---
        case 0xE9:
            cpu->pc = get_hl(cpu);
            break;
        // --- SUI d8 (Subtract Immediate) ---
        case 0xD6:
        {
            uint8_t imediato = cpu->memory[cpu->pc + 1];
            uint16_t res = (uint16_t)cpu->a - imediato;
            
            // Atualiza a flag Carry (1 se houve borrow/subtração com resultado negativo)
            if (cpu->a < imediato) cpu->flags |= (1 << 0);
            else cpu->flags &= ~(1 << 0);

            cpu->a = (uint8_t)(res & 0xFF);
            atualizar_flags_zsp(cpu, cpu->a);
            cpu->pc += 2;
            break;
        }
        // --- 1. SALTOS CONDICIONAIS FALTANTES (JM, JP, JPE, JPO) ---
        case 0xFA: // JM addr (Jump if Minus / Sign = 1)
            if (cpu->flags & (1 << 7)) cpu->pc = cpu->memory[cpu->pc + 1] | (cpu->memory[cpu->pc + 2] << 8);
            else cpu->pc += 3;
            break;

        case 0xF2: // JP addr (Jump if Positive / Sign = 0)
            if (!(cpu->flags & (1 << 7))) cpu->pc = cpu->memory[cpu->pc + 1] | (cpu->memory[cpu->pc + 2] << 8);
            else cpu->pc += 3;
            break;

        case 0xEA: // JPE addr (Jump if Parity Even / Parity = 1)
            if (cpu->flags & (1 << 2)) cpu->pc = cpu->memory[cpu->pc + 1] | (cpu->memory[cpu->pc + 2] << 8);
            else cpu->pc += 3;
            break;

        case 0xE2: // JPO addr (Jump if Parity Odd / Parity = 0)
            if (!(cpu->flags & (1 << 2))) cpu->pc = cpu->memory[cpu->pc + 1] | (cpu->memory[cpu->pc + 2] << 8);
            else cpu->pc += 3;
            break;

        // --- 2. CALLS CONDICIONAIS FALTANTES (CM, CP, CPE, CPO) ---
        case 0xFC: // CM addr (Call if Minus)
        {
            uint16_t target = cpu->memory[cpu->pc + 1] | (cpu->memory[cpu->pc + 2] << 8);
            if (cpu->flags & (1 << 7)) {
                uint16_t ret_addr = cpu->pc + 3;
                cpu->sp -= 2;
                cpu->memory[cpu->sp] = ret_addr & 0xFF;
                cpu->memory[cpu->sp + 1] = (ret_addr >> 8) & 0xFF;
                cpu->pc = target;
            } else cpu->pc += 3;
            break;
        }

        case 0xF4: // CP addr (Call if Plus)
        {
            uint16_t target = cpu->memory[cpu->pc + 1] | (cpu->memory[cpu->pc + 2] << 8);
            if (!(cpu->flags & (1 << 7))) {
                uint16_t ret_addr = cpu->pc + 3;
                cpu->sp -= 2;
                cpu->memory[cpu->sp] = ret_addr & 0xFF;
                cpu->memory[cpu->sp + 1] = (ret_addr >> 8) & 0xFF;
                cpu->pc = target;
            } else cpu->pc += 3;
            break;
        }

        case 0xEC: // CPE addr (Call if Parity Even)
        {
            uint16_t target = cpu->memory[cpu->pc + 1] | (cpu->memory[cpu->pc + 2] << 8);
            if (cpu->flags & (1 << 2)) {
                uint16_t ret_addr = cpu->pc + 3;
                cpu->sp -= 2;
                cpu->memory[cpu->sp] = ret_addr & 0xFF;
                cpu->memory[cpu->sp + 1] = (ret_addr >> 8) & 0xFF;
                cpu->pc = target;
            } else cpu->pc += 3;
            break;
        }

        case 0xE4: // CPO addr (Call if Parity Odd)
        {
            uint16_t target = cpu->memory[cpu->pc + 1] | (cpu->memory[cpu->pc + 2] << 8);
            if (!(cpu->flags & (1 << 2))) {
                uint16_t ret_addr = cpu->pc + 3;
                cpu->sp -= 2;
                cpu->memory[cpu->sp] = ret_addr & 0xFF;
                cpu->memory[cpu->sp + 1] = (ret_addr >> 8) & 0xFF;
                cpu->pc = target;
            } else cpu->pc += 3;
            break;
        }

        // --- 3. RETORNOS CONDICIONAIS FALTANTES (RM, RP, RPE, RPO) ---
        case 0xF8: // RM (Return if Minus)
            if (cpu->flags & (1 << 7)) {
                cpu->pc = cpu->memory[cpu->sp] | (cpu->memory[cpu->sp + 1] << 8);
                cpu->sp += 2;
            } else cpu->pc += 1;
            break;

        case 0xF0: // RP (Return if Positive)
            if (!(cpu->flags & (1 << 7))) {
                cpu->pc = cpu->memory[cpu->sp] | (cpu->memory[cpu->sp + 1] << 8);
                cpu->sp += 2;
            } else cpu->pc += 1;
            break;

        case 0xE8: // RPE (Return if Parity Even)
            if (cpu->flags & (1 << 2)) {
                cpu->pc = cpu->memory[cpu->sp] | (cpu->memory[cpu->sp + 1] << 8);
                cpu->sp += 2;
            } else cpu->pc += 1;
            break;

        case 0xE0: // RPO (Return if Parity Odd)
            if (!(cpu->flags & (1 << 2))) {
                cpu->pc = cpu->memory[cpu->sp] | (cpu->memory[cpu->sp + 1] << 8);
                cpu->sp += 2;
            } else cpu->pc += 1;
            break;

        // --- 4. COMPARAÇÕES RESTANTES (CMP r / CMP M) ---
        case 0xBF: case 0xB8: case 0xB9: case 0xBA: case 0xBB: case 0xBC: case 0xBD:
        {
            uint8_t reg_val = 0;
            if (opcode == 0xBF) reg_val = cpu->a;
            else if (opcode == 0xB8) reg_val = cpu->b;
            else if (opcode == 0xB9) reg_val = cpu->c;
            else if (opcode == 0xBA) reg_val = cpu->d;
            else if (opcode == 0xBB) reg_val = cpu->e;
            else if (opcode == 0xBC) reg_val = cpu->h;
            else if (opcode == 0xBD) reg_val = cpu->l;
            
            uint8_t res = cpu->a - reg_val;
            atualizar_flags_zsp(cpu, res);
            if (cpu->a < reg_val) cpu->flags |= (1 << 0);
            else cpu->flags &= ~(1 << 0);
            cpu->pc += 1;
            break;
        }

        case 0xBE: // CMP M
        {
            uint8_t val = cpu->memory[get_hl(cpu)];
            uint8_t res = cpu->a - val;
            atualizar_flags_zsp(cpu, res);
            if (cpu->a < val) cpu->flags |= (1 << 0);
            else cpu->flags &= ~(1 << 0);
            cpu->pc += 1;
            break;
        }
        // --- 1. SUBTRAÇÕES COM BORROW (SBI / SBB) ---
        case 0xDE: // SBI d8 (Subtract Immediate with Borrow)
        {
            uint8_t imediato = cpu->memory[cpu->pc + 1];
            uint8_t carry = (cpu->flags & 1);
            uint16_t res = (uint16_t)cpu->a - imediato - carry;

            if ((uint32_t)cpu->a < (uint32_t)imediato + carry) cpu->flags |= 1;
            else cpu->flags &= ~1;

            cpu->a = (uint8_t)(res & 0xFF);
            atualizar_flags_zsp(cpu, cpu->a);
            cpu->pc += 2;
            break;
        }
        default:
            printf("[ERRO] Opcode nao implementado: 0x%02X no PC: 0x%04X\n", opcode, cpu->pc);
            return -1; // Retorna erro
    }

    return 0; // Retorna 0 para SUCESSO
}
