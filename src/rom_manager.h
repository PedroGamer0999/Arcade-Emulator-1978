#ifndef ROM_MANAGER_H
#define ROM_MANAGER_H

#include <stdint.h>
#include <stddef.h>
#include "cpu8080.h"

#define MAX_ROM_PIECES 8

/* Uma peça de ROM (chip EPROM individual) com metadados de validação */
typedef struct {
    const char *nome_arquivo;    /* Nome do arquivo dentro do ZIP ou pasta */
    uint16_t    offset_memoria;  /* Endereço de destino na memória da CPU */
    size_t      tamanho_esperado;/* Tamanho em bytes do dump original */
    uint32_t    crc32_esperado;  /* Hash CRC32 do dump autêntico */
} RomPiece;

/* Definição completa de um conjunto de ROMs de um jogo */
typedef struct {
    const char *id_jogo;              /* Ex: "invaders", "lrescu" */
    const char *titulo;               /* Ex: "Space Invaders (Midway 1978)" */
    int         num_pecas;            /* Quantidade de chips EPROM */
    RomPiece    pecas[MAX_ROM_PIECES];
} RomSetDefinition;

/* Calcula CRC32 padrão (polinômio 0xEDB88320) de um buffer */
uint32_t calcular_crc32(const uint8_t *dados, size_t tamanho);

/* Carrega as ROMs de 'caminho_ou_zip' (pasta ou arquivo .zip) na memória da CPU.
 * Retorna 0 em sucesso. Hashes incorretos geram aviso mas não interrompem a carga. */
int carregar_romset(State8080 *cpu, const RomSetDefinition *def, const char *caminho_ou_zip);

/* Verifica integridade do romset SEM carregar na CPU.
 * Em caso de falha, preenche 'msg_erro' com mensagem legível.
 * Retorna 0 se OK, -1 se algum arquivo está ausente ou corrompido. */
int verificar_romset(const RomSetDefinition *def, const char *caminho_ou_zip,
                     char *msg_erro, size_t max_len);

#endif /* ROM_MANAGER_H */
