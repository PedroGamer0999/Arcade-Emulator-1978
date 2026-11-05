#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "rom_manager.h"
#include "miniz.h"

/* ── CRC32 (polinômio IEEE 802.3, sem dependência externa) ─────────────────── */

static uint32_t crc32_tabela[256];
static int      crc32_tabela_inicializada = 0;

static void inicializar_tabela_crc32(void) {
    for (uint32_t i = 0; i < 256; i++) {
        uint32_t crc = i;
        for (int j = 0; j < 8; j++) {
            crc = (crc & 1) ? (0xEDB88320u ^ (crc >> 1)) : (crc >> 1);
        }
        crc32_tabela[i] = crc;
    }
    crc32_tabela_inicializada = 1;
}

uint32_t calcular_crc32(const uint8_t *dados, size_t tamanho) {
    if (!crc32_tabela_inicializada) inicializar_tabela_crc32();
    uint32_t crc = 0xFFFFFFFFu;
    for (size_t i = 0; i < tamanho; i++) {
        crc = crc32_tabela[(crc ^ dados[i]) & 0xFF] ^ (crc >> 8);
    }
    return crc ^ 0xFFFFFFFFu;
}

/* ── Utilitários internos ───────────────────────────────────────────────────── */

/* Retorna 1 se 'caminho' termina com 'sufixo' (case-insensitive ASCII) */
static int termina_com(const char *caminho, const char *sufixo) {
    size_t lc = strlen(caminho);
    size_t ls = strlen(sufixo);
    if (lc < ls) return 0;
    const char *p = caminho + lc - ls;
    for (size_t i = 0; i < ls; i++) {
        char a = (p[i] >= 'A' && p[i] <= 'Z') ? (char)(p[i] + 32) : p[i];
        char b = (sufixo[i] >= 'A' && sufixo[i] <= 'Z') ? (char)(sufixo[i] + 32) : sufixo[i];
        if (a != b) return 0;
    }
    return 1;
}

/* Carrega uma peça de ROM a partir de um buffer em memória */
static void carregar_peca_na_cpu(State8080 *cpu, const RomPiece *peca,
                                  const uint8_t *buf, size_t tam) {
    size_t bytes = tam < peca->tamanho_esperado ? tam : peca->tamanho_esperado;
    memcpy(cpu->memory + peca->offset_memoria, buf, bytes);
}

/* ── Carga a partir de pasta com arquivos soltos ───────────────────────────── */

static int carregar_de_pasta(State8080 *cpu, const RomSetDefinition *def,
                              const char *pasta) {
    char caminho[512];
    for (int i = 0; i < def->num_pecas; i++) {
        const RomPiece *p = &def->pecas[i];
        snprintf(caminho, sizeof(caminho), "%s/%s", pasta, p->nome_arquivo);

        FILE *f = fopen(caminho, "rb");
        if (!f) {
            fprintf(stderr, "[ERRO] Arquivo obrigatorio nao encontrado: %s\n", caminho);
            return -1;
        }

        fseek(f, 0, SEEK_END);
        long tam = ftell(f);
        rewind(f);

        if ((size_t)tam != p->tamanho_esperado) {
            fprintf(stderr, "[AVISO] Tamanho incorreto para %s: obtido %ld bytes, esperado %zu bytes\n",
                    p->nome_arquivo, tam, p->tamanho_esperado);
        }

        uint8_t *buf = (uint8_t *)malloc((size_t)tam);
        if (!buf) { fclose(f); return -1; }

        if (fread(buf, 1, (size_t)tam, f) != (size_t)tam) {
            fprintf(stderr, "[ERRO] Falha ao ler %s\n", caminho);
            free(buf); fclose(f);
            return -1;
        }
        fclose(f);

        uint32_t crc = calcular_crc32(buf, (size_t)tam);
        if (crc != p->crc32_esperado) {
            fprintf(stderr, "[AVISO] CRC32 incorreto para %s: obtido 0x%08X, esperado 0x%08X\n",
                    p->nome_arquivo, crc, p->crc32_esperado);
        }

        carregar_peca_na_cpu(cpu, p, buf, (size_t)tam);
        free(buf);
    }
    return 0;
}

/* ── Carga a partir de arquivo ZIP (via miniz, sem extração em disco) ──────── */

static int carregar_de_zip(State8080 *cpu, const RomSetDefinition *def,
                            const char *caminho_zip) {
    mz_zip_archive zip;
    memset(&zip, 0, sizeof(zip));

    if (!mz_zip_reader_init_file(&zip, caminho_zip, 0)) {
        fprintf(stderr, "[ERRO] Nao foi possivel abrir ZIP: %s\n", caminho_zip);
        return -1;
    }

    int resultado = 0;

    for (int i = 0; i < def->num_pecas; i++) {
        const RomPiece *p = &def->pecas[i];

        int idx = mz_zip_reader_locate_file(&zip, p->nome_arquivo, NULL, 0);
        if (idx < 0) {
            fprintf(stderr, "[ERRO] Arquivo obrigatorio nao encontrado no ZIP: %s\n",
                    p->nome_arquivo);
            resultado = -1;
            continue;
        }

        mz_zip_archive_file_stat stat;
        mz_zip_reader_file_stat(&zip, (mz_uint)idx, &stat);

        size_t tam = (size_t)stat.m_uncomp_size;
        if (tam != p->tamanho_esperado) {
            fprintf(stderr, "[AVISO] Tamanho incorreto para %s: obtido %zu bytes, esperado %zu bytes\n",
                    p->nome_arquivo, tam, p->tamanho_esperado);
        }

        uint8_t *buf = (uint8_t *)malloc(tam);
        if (!buf) { resultado = -1; continue; }

        if (!mz_zip_reader_extract_to_mem(&zip, (mz_uint)idx, buf, tam, 0)) {
            fprintf(stderr, "[ERRO] Falha ao extrair %s do ZIP\n", p->nome_arquivo);
            free(buf);
            resultado = -1;
            continue;
        }

        uint32_t crc = calcular_crc32(buf, tam);
        if (crc != p->crc32_esperado) {
            fprintf(stderr, "[AVISO] CRC32 incorreto para %s: obtido 0x%08X, esperado 0x%08X\n",
                    p->nome_arquivo, crc, p->crc32_esperado);
        }

        carregar_peca_na_cpu(cpu, p, buf, tam);
        free(buf);
    }

    mz_zip_reader_end(&zip);
    return resultado;
}

/* ── API pública ────────────────────────────────────────────────────────────── */

int carregar_romset(State8080 *cpu, const RomSetDefinition *def,
                    const char *caminho_ou_zip) {
    if (!cpu || !def || !caminho_ou_zip) return -1;

    if (termina_com(caminho_ou_zip, ".zip")) {
        return carregar_de_zip(cpu, def, caminho_ou_zip);
    }
    return carregar_de_pasta(cpu, def, caminho_ou_zip);
}

int verificar_romset(const RomSetDefinition *def, const char *caminho_ou_zip,
                     char *msg_erro, size_t max_len) {
    /* Verificação leve: usar uma CPU temporária descartada */
    State8080 cpu_tmp;
    memset(&cpu_tmp, 0, sizeof(cpu_tmp));

    /* Redireciona stderr temporariamente para capturar mensagem */
    int ret = carregar_romset(&cpu_tmp, def, caminho_ou_zip);
    if (ret != 0 && msg_erro && max_len > 0) {
        snprintf(msg_erro, max_len,
                 "Verificacao falhou para '%s' (romset: %s). "
                 "Verifique o console para detalhes de CRC32.",
                 caminho_ou_zip, def->id_jogo);
    }
    return ret;
}
