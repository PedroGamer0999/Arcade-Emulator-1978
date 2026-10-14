#include <stdio.h>
#include "io.h"
#include <SDL2/SDL.h>

void init_io(InputOutput *io) {
    // O bit 3 da Porta 1 deve iniciar em 1 para indicar funcionamento normal
    io->port1 = 0x08; 
    io->port2 = 0x00;
    io->shift_register = 0;
    io->shift_offset = 0;
}

uint8_t io_in(InputOutput *io, uint8_t port) {
    switch (port) {
        case 1:
            return io->port1;
        case 2:
            return io->port2;
        case 3: {
            uint16_t res = io->shift_register >> (8 - io->shift_offset);
            return (uint8_t)(res & 0xFF);
        }
        default:
            return 0x00;
    }
}

void io_out(InputOutput *io, uint8_t port, uint8_t value) {
    switch (port) {
        case 2:
            io->shift_offset = value & 0x07;
            break;
        case 4:
            io->shift_register = (io->shift_register >> 8) | ((uint16_t)value << 8);
            break;
        default:
            break;
    }
}

// Mapeamento de Teclas -> Bits do Space Invaders
void tratar_teclado(InputOutput *io, uint32_t key, int pressionado) {
    switch (key) {
        case SDLK_c: // 'C' ou 'ENTER' -> Inserir Moeda (Bit 0 da Porta 1)
        case SDLK_RETURN:
            if (pressionado) io->port1 |= 0x01; else io->port1 &= ~0x01;
            break;

        case SDLK_1: // '1' -> Botão 1 Player Start (Bit 2 da Porta 1)
            if (pressionado) io->port1 |= 0x04; else io->port1 &= ~0x04;
            break;

        case SDLK_SPACE: // 'Espaço' -> Atirar (Bit 4 da Porta 1 e Porta 2)
            if (pressionado) {
                io->port1 |= 0x10;
                io->port2 |= 0x10;
            } else {
                io->port1 &= ~0x10;
                io->port2 &= ~0x10;
            }
            break;

        case SDLK_LEFT: // 'Seta Esquerda' -> Mover para Esquerda (Bit 5 da Porta 1 e Porta 2)
            if (pressionado) {
                io->port1 |= 0x20;
                io->port2 |= 0x20;
            } else {
                io->port1 &= ~0x20;
                io->port2 &= ~0x20;
            }
            break;

        case SDLK_RIGHT: // 'Seta Direita' -> Mover para Direita (Bit 6 da Porta 1 e Porta 2)
            if (pressionado) {
                io->port1 |= 0x40;
                io->port2 |= 0x40;
            } else {
                io->port1 &= ~0x40;
                io->port2 &= ~0x40;
            }
            break;
    }
}