#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "game_driver.h"

/* ════════════════════════════════════════════════════════════════════════════
 * ROMSET DEFINITIONS — Hashes CRC32 oficiais dos dumps autênticos
 * ════════════════════════════════════════════════════════════════════════════ */

/* Space Invaders — Midway 1978 (8 KB, 4 chips EPROM 2 KB cada) */
static const RomSetDefinition ROMSET_INVADERS = {
    .id_jogo   = "invaders",
    .titulo    = "Space Invaders (Midway 1978)",
    .num_pecas = 4,
    .pecas = {
        { "invaders.h", 0x0000, 2048, 0x734F5AD8u },
        { "invaders.g", 0x0800, 2048, 0x6BFACA4Au },
        { "invaders.f", 0x1000, 2048, 0x0CCEAD96u },
        { "invaders.e", 0x1800, 2048, 0x14E538B0u }
    }
};

/* Space Invaders Part II / Deluxe — Taito 1979 (10 KB) */
static const RomSetDefinition ROMSET_INVADPT2 = {
    .id_jogo   = "invadpt2",
    .titulo    = "Space Invaders Part II (Taito 1979)",
    .num_pecas = 5,
    .pecas = {
        { "invadpt2.h", 0x0000, 2048, 0x3A9FE8C4u },
        { "invadpt2.g", 0x0800, 2048, 0x4B6A6F0Bu },
        { "invadpt2.f", 0x1000, 2048, 0xBF4DB433u },
        { "invadpt2.e", 0x1800, 2048, 0x31CF2691u },
        { "invadpt2.d", 0x2000, 2048, 0x13DBE3ECu }
    }
};

/* Lunar Rescue — Taito 1979 (12 KB) */
static const RomSetDefinition ROMSET_LRESCU = {
    .id_jogo   = "lrescu",
    .titulo    = "Lunar Rescue (Taito 1979)",
    .num_pecas = 6,
    .pecas = {
        { "lrescue.1",  0x0000, 2048, 0x1A956088u },
        { "lrescue.2",  0x0800, 2048, 0x0EA71297u },
        { "lrescue.3",  0x1000, 2048, 0xD14878E0u },
        { "lrescue.4",  0x1800, 2048, 0x798BA0D9u },
        { "lrescue.5",  0x2000, 2048, 0x73A01E4Au },
        { "lrescue.6",  0x2800, 2048, 0x1E3D03D2u }
    }
};

/* Balloon Bomber — Taito 1980 (10 KB) */
static const RomSetDefinition ROMSET_BALLBOMB = {
    .id_jogo   = "ballbomb",
    .titulo    = "Balloon Bomber (Taito 1980)",
    .num_pecas = 5,
    .pecas = {
        { "tn01",  0x0000, 2048, 0x5C89B2C8u },
        { "tn02",  0x0800, 2048, 0x3B85B913u },
        { "tn03",  0x1000, 2048, 0x85E7C7E8u },
        { "tn04",  0x1800, 2048, 0x6EB9BC7Bu },
        { "tn05c", 0x2000, 2048, 0x3F8B7E37u }
    }
};

/* Space Chaser — Taito 1979 (8 KB) */
static const RomSetDefinition ROMSET_SCHASER = {
    .id_jogo   = "schaser",
    .titulo    = "Space Chaser (Taito 1979)",
    .num_pecas = 4,
    .pecas = {
        { "sc0",  0x0000, 2048, 0x3BECF672u },
        { "sc1",  0x0800, 2048, 0xFE1920D4u },
        { "sc2",  0x1000, 2048, 0x5A1046C7u },
        { "sc3",  0x1800, 2048, 0x9F5EE51Du }
    }
};

/* CPUDIAG — Microcosm 1980 (diagnóstico de CPU 8080, carregado em 0x0100) */
static const RomSetDefinition ROMSET_CPUDIAG = {
    .id_jogo   = "cpudiag",
    .titulo    = "8080 CPU Diagnostic (Microcosm 1980)",
    .num_pecas = 1,
    .pecas = {
        { "cpudiag.bin", 0x0100, 1536, 0x00000000u }  /* CRC não-validado: é ROM de teste pública */
    }
};

/* ════════════════════════════════════════════════════════════════════════════
 * COLOR MAPS — Mapeamento de cores por coordenada Y (herança arcade)
 * ════════════════════════════════════════════════════════════════════════════ */

/* Color map padrão do Space Invaders (overlay de celofane do gabinete original) */
static uint32_t color_map_invaders(int x, int y, uint8_t pixel_ativo) {
    (void)x;
    if (!pixel_ativo) return 0xFF000000u;      /* preto */
    if (y < 32)       return 0xFF2626E6u;      /* vermelho arcade (UFO / placar) */
    if (y < 64)       return 0xFFFFFFFFu;      /* branco (invasores superiores) */
    if (y > 184 && y < 240) return 0xFF26E626u;/* verde arcade (barricadas) */
    if (y >= 240)     return 0xFF26E626u;      /* verde arcade (canhão / solo) */
    return 0xFFFFFFFFu;                        /* branco (invasores centrais) */
}

/* Color map do Lunar Rescue (cores planas sem overlay fixo) */
static uint32_t color_map_lrescu(int x, int y, uint8_t pixel_ativo) {
    (void)x; (void)y;
    if (!pixel_ativo) return 0xFF000000u;
    return 0xFF00FFFFu;  /* ciano — aproximação do PROM de cor do Lunar Rescue */
}

/* Color map genérico para jogos sem overlay especial */
static uint32_t color_map_white(int x, int y, uint8_t pixel_ativo) {
    (void)x; (void)y;
    return pixel_ativo ? 0xFFFFFFFFu : 0xFF000000u;
}

/* ════════════════════════════════════════════════════════════════════════════
 * CALLBACKS DE I/O
 * ════════════════════════════════════════════════════════════════════════════ */

/* init_hardware padrão: nenhuma customização necessária */
static void init_hw_default(State8080 *cpu, InputOutput *io) {
    (void)cpu; (void)io;
}

/* init_hardware do CPUDIAG: configura SP e PC conforme especificação da ROM */
static void init_hw_cpudiag(State8080 *cpu, InputOutput *io) {
    (void)io;
    /* CPUDIAG carrega em 0x0100; requer SP apontando para 0x6AD */
    cpu->pc = 0x0100;
    cpu->sp = 0x06AD;
    /* Injetar instrução JMP 0x0100 em 0x0000 para evitar execução de lixo */
    cpu->memory[0x0000] = 0xC3;
    cpu->memory[0x0001] = 0x00;
    cpu->memory[0x0002] = 0x01;
    /* Patch: redirecionar saída serial em 0x0005 — o driver intercepta via cpudiag_mode */
    cpu->memory[0x0005] = 0xD3; /* OUT 0x00 — instrução benigna no endereço de saída */
    cpu->memory[0x0006] = 0x00;
}

/* ════════════════════════════════════════════════════════════════════════════
 * REGISTRO GLOBAL DE DRIVERS
 * ════════════════════════════════════════════════════════════════════════════ */

static const GameDriver DRIVERS[] = {
    {
        .id            = "invaders",
        .nome_completo = "Space Invaders (Midway 1978)",
        .ano           = "1978",
        .fabricante    = "Midway",
        .romset        = &ROMSET_INVADERS,
        .init_hardware = init_hw_default,
        .io_in         = NULL,   /* usa io_in padrão de io.c */
        .io_out        = NULL,
        .color_map     = color_map_invaders,
        .cpudiag_mode  = 0
    },
    {
        .id            = "invadpt2",
        .nome_completo = "Space Invaders Part II (Taito 1979)",
        .ano           = "1979",
        .fabricante    = "Taito",
        .romset        = &ROMSET_INVADPT2,
        .init_hardware = init_hw_default,
        .io_in         = NULL,
        .io_out        = NULL,
        .color_map     = color_map_invaders,
        .cpudiag_mode  = 0
    },
    {
        .id            = "lrescu",
        .nome_completo = "Lunar Rescue (Taito 1979)",
        .ano           = "1979",
        .fabricante    = "Taito",
        .romset        = &ROMSET_LRESCU,
        .init_hardware = init_hw_default,
        .io_in         = NULL,
        .io_out        = NULL,
        .color_map     = color_map_lrescu,
        .cpudiag_mode  = 0
    },
    {
        .id            = "ballbomb",
        .nome_completo = "Balloon Bomber (Taito 1980)",
        .ano           = "1980",
        .fabricante    = "Taito",
        .romset        = &ROMSET_BALLBOMB,
        .init_hardware = init_hw_default,
        .io_in         = NULL,
        .io_out        = NULL,
        .color_map     = color_map_white,
        .cpudiag_mode  = 0
    },
    {
        .id            = "schaser",
        .nome_completo = "Space Chaser (Taito 1979)",
        .ano           = "1979",
        .fabricante    = "Taito",
        .romset        = &ROMSET_SCHASER,
        .init_hardware = init_hw_default,
        .io_in         = NULL,
        .io_out        = NULL,
        .color_map     = color_map_white,
        .cpudiag_mode  = 0
    },
    {
        .id            = "cpudiag",
        .nome_completo = "8080 CPU Diagnostic (Microcosm 1980)",
        .ano           = "1980",
        .fabricante    = "Microcosm",
        .romset        = &ROMSET_CPUDIAG,
        .init_hardware = init_hw_cpudiag,
        .io_in         = NULL,
        .io_out        = NULL,
        .color_map     = color_map_white,
        .cpudiag_mode  = 1
    }
};

#define NUM_DRIVERS ((int)(sizeof(DRIVERS) / sizeof(DRIVERS[0])))

/* ════════════════════════════════════════════════════════════════════════════
 * API PÚBLICA
 * ════════════════════════════════════════════════════════════════════════════ */

const GameDriver *obter_driver_por_id(const char *id) {
    if (!id) return NULL;
    for (int i = 0; i < NUM_DRIVERS; i++) {
        if (strcmp(DRIVERS[i].id, id) == 0) return &DRIVERS[i];
    }
    return NULL;
}

const GameDriver *detectar_driver_por_arquivo(const char *caminho) {
    if (!caminho) return NULL;

    /* Extrai basename: último '/' ou '\' */
    const char *base = caminho;
    for (const char *p = caminho; *p; p++) {
        if (*p == '/' || *p == '\\') base = p + 1;
    }

    /* Remove extensão .zip / .bin para obter o id candidato */
    char id_buf[64];
    strncpy(id_buf, base, sizeof(id_buf) - 1);
    id_buf[sizeof(id_buf) - 1] = '\0';
    char *ponto = strrchr(id_buf, '.');
    if (ponto) *ponto = '\0';

    /* Busca exata pelo id */
    const GameDriver *drv = obter_driver_por_id(id_buf);
    if (drv) return drv;

    /* Busca por prefixo (ex: "invaders_mw" → "invaders") */
    for (int i = 0; i < NUM_DRIVERS; i++) {
        if (strncmp(DRIVERS[i].id, id_buf, strlen(DRIVERS[i].id)) == 0) {
            return &DRIVERS[i];
        }
    }

    return NULL;
}

int listar_drivers_disponiveis(const GameDriver ***lista) {
    static const GameDriver *ptrs[NUM_DRIVERS + 1];
    for (int i = 0; i < NUM_DRIVERS; i++) ptrs[i] = &DRIVERS[i];
    ptrs[NUM_DRIVERS] = NULL;
    if (lista) *lista = ptrs;
    return NUM_DRIVERS;
}
