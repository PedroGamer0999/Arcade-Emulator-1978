#ifndef IO_H
#define IO_H

#include <stdint.h>

typedef struct InputOutput {
    uint8_t port1; // Porta 0x01: Moeda, P1 Start, Tiro, Esquerda, Direita
    uint8_t port2; // Porta 0x02: P2 Start, Tiro, Esquerda, Direita, Vidas
    uint16_t shift_register;
    uint8_t shift_offset;
} InputOutput;

void init_io(InputOutput *io);
uint8_t io_in(InputOutput *io, uint8_t port);
void io_out(InputOutput *io, uint8_t port, uint8_t value);
void tratar_teclado(InputOutput *io, uint32_t key, int pressionado);

#endif