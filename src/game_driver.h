#ifndef GAME_DRIVER_H
#define GAME_DRIVER_H

#include <stdint.h>
#include "cpu8080.h"
#include "io.h"
#include "rom_manager.h"

/* Interface abstrata de driver de jogo da placa 8080 */
typedef struct GameDriver {
    const char *id;            /* Identificador curto: "invaders", "lrescu", etc. */
    const char *nome_completo; /* Nome de exibição: "Space Invaders (Midway 1978)" */
    const char *ano;
    const char *fabricante;
    const RomSetDefinition *romset;

    /* Callbacks de hardware (podem ser NULL para usar defaults do Space Invaders) */
    void     (*init_hardware)(State8080 *cpu, InputOutput *io);
    uint8_t  (*io_in)(InputOutput *io, uint8_t port);
    void     (*io_out)(InputOutput *io, uint8_t port, uint8_t val);
    uint32_t (*color_map)(int x, int y, uint8_t pixel_ativo);

    /* Flag especial: interceptar CALL 0x0005 para saída serial (CPUDIAG) */
    int cpudiag_mode;
} GameDriver;

/* Retorna o driver correspondente ao id, ou NULL se não encontrado */
const GameDriver *obter_driver_por_id(const char *id);

/* Detecta o driver pelo nome do arquivo (ex: "lrescu.zip" → driver lrescu) */
const GameDriver *detectar_driver_por_arquivo(const char *caminho);

/* Preenche 'lista' com ponteiro para o array interno; retorna número de drivers */
int listar_drivers_disponiveis(const GameDriver ***lista);

#endif /* GAME_DRIVER_H */
