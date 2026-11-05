#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <SDL2/SDL.h>
#include "cpu8080.h"
#include "io.h"
#include "video.h"
#include "audio.h"
#include "rom_manager.h"
#include "game_driver.h"

/* ── Constantes de clock e temporização do Intel 8080 ────────────────────── */
#define CPU_CLOCK_HZ     2000000
#define TARGET_FPS       60
#define CYCLES_PER_FRAME (CPU_CLOCK_HZ / TARGET_FPS)   /* 33.333 ciclos */
#define CYCLES_PER_HALF  (CYCLES_PER_FRAME / 2)        /* 16.666 ciclos */

/* ── Caminho padrão de ROMs ─────────────────────────────────────────────── */
#define ROM_PATH_DEFAULT "rom"

/* ── Impressão de ajuda ─────────────────────────────────────────────────── */
static void imprimir_ajuda(const char *exe) {
    const GameDriver **lista = NULL;
    int n = listar_drivers_disponiveis(&lista);

    printf("Uso: %s [opcoes] [caminho_rom]\n\n", exe);
    printf("Opcoes:\n");
    printf("  -game <id>    Seleciona o jogo pelo ID (padrao: invaders)\n");
    printf("  -list         Lista jogos suportados e encerra\n");
    printf("  -test         Atalho para -game cpudiag (roda CPUDIAG da pasta rom/)\n");
    printf("  -help         Exibe esta ajuda\n\n");
    printf("Jogos suportados (IDs para -game):\n");
    for (int i = 0; i < n; i++) {
        printf("  %-12s %s\n", lista[i]->id, lista[i]->nome_completo);
    }
    printf("\nExemplos:\n");
    printf("  %s                          # Space Invaders da pasta rom/\n", exe);
    printf("  %s roms/invaders.zip        # Space Invaders de arquivo ZIP\n", exe);
    printf("  %s -game lrescu roms/       # Lunar Rescue da pasta roms/\n", exe);
    printf("  %s cpudiag.bin              # Roda CPUDIAG e imprime resultados\n", exe);
}

/* ── Loop especial CPUDIAG: intercepta CALL 0x0005 para saída serial ────── */
static void cpudiag_interceptar_call(State8080 *cpu) {
    if (cpu->pc != 0x0005) return;

    if (cpu->c == 9) {
        /* Imprime string apontada por DE até encontrar '$' */
        uint16_t addr = (uint16_t)((cpu->d << 8) | cpu->e);
        while (cpu->memory[addr] != '$') {
            putchar(cpu->memory[addr++]);
        }
        fflush(stdout);
    } else if (cpu->c == 2) {
        /* Imprime caractere em E */
        putchar(cpu->e);
        fflush(stdout);
    }

    /* Simula RET: desempilha o endereço de retorno */
    cpu->pc = (uint16_t)(cpu->memory[cpu->sp] | (cpu->memory[cpu->sp + 1] << 8));
    cpu->sp += 2;
}

/* ── Entrada principal ──────────────────────────────────────────────────── */
int main(int argc, char *argv[]) {
    const char *game_id      = "invaders";
    const char *caminho_rom  = NULL;
    int         listar       = 0;

    /* ── Parse de argumentos ─────────────────────────────────────────────── */
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-help") == 0 || strcmp(argv[i], "--help") == 0) {
            imprimir_ajuda(argv[0]);
            return 0;
        } else if (strcmp(argv[i], "-list") == 0) {
            listar = 1;
        } else if (strcmp(argv[i], "-test") == 0) {
            game_id = "cpudiag";
        } else if (strcmp(argv[i], "-game") == 0 && i + 1 < argc) {
            game_id = argv[++i];
        } else {
            caminho_rom = argv[i];
        }
    }

    if (listar) {
        imprimir_ajuda(argv[0]);
        return 0;
    }

    /* ── Seleção do driver ───────────────────────────────────────────────── */
    const GameDriver *driver = NULL;

    /* Se um caminho foi fornecido, tenta detectar o driver pelo nome do arquivo */
    if (caminho_rom) {
        driver = detectar_driver_por_arquivo(caminho_rom);
    }

    /* Se não detectou pelo arquivo (ou nenhum caminho foi dado), usa o -game id */
    if (!driver) {
        driver = obter_driver_por_id(game_id);
    }

    if (!driver) {
        fprintf(stderr, "[ERRO] Driver desconhecido: '%s'. Use -list para ver opcoes.\n", game_id);
        return 1;
    }

    /* ── Caminho de ROM: detecta automático se não passado ──────────────── */
    if (!caminho_rom) {
        /* CPUDIAG usa a pasta rom/ por padrão (usuario coloca cpudiag.bin lá) */
        caminho_rom = ROM_PATH_DEFAULT;
    }

    printf("[INFO] Carregando: %s\n", driver->nome_completo);
    printf("[INFO] ROM: %s\n", caminho_rom);

    /* ── Inicialização de subsistemas ───────────────────────────────────── */
    State8080  cpu;
    InputOutput io;
    Display    display;

    init_cpu(&cpu);
    init_io(&io);

    /* ── Carga de ROMs ──────────────────────────────────────────────────── */
    if (carregar_romset(&cpu, driver->romset, caminho_rom) != 0) {
        fprintf(stderr, "[ERRO] Falha ao carregar ROM. Encerrando.\n");
        return 1;
    }

    /* ── Inicialização de hardware customizado do driver ────────────────── */
    if (driver->init_hardware) {
        driver->init_hardware(&cpu, &io);
    }

    /* ── Modo CPUDIAG: roda headless no terminal, sem janela SDL ─────────── */
    if (driver->cpudiag_mode) {
        printf("[CPUDIAG] Iniciando diagnostico da CPU 8080...\n\n");
        int ciclos = 0;
        int rodando = 1;

        while (rodando) {
            cpudiag_interceptar_call(&cpu);

            int cycles = 0;
            if (exec_instrucao(&cpu, &io, &cycles) != 0) {
                break;
            }
            ciclos += cycles;

            /* Limita a 50 milhões de ciclos (~25 segundos de CPU a 2 MHz) */
            if (ciclos > 50000000) {
                printf("\n[CPUDIAG] Limite de ciclos atingido.\n");
                rodando = 0;
            }
        }
        printf("\n[CPUDIAG] Diagnostico encerrado.\n");
        return 0;
    }

    /* ── Inicializa vídeo e áudio ────────────────────────────────────────── */
    if (init_video(&display) != 0) return 1;
    init_audio("sounds");

    /* ── Título da janela com nome do jogo ──────────────────────────────── */
    char titulo_base[128];
    snprintf(titulo_base, sizeof(titulo_base), "%s [%s]",
             driver->nome_completo, driver->ano);
    SDL_SetWindowTitle(display.janela, titulo_base);

    /* ── Loop principal de emulação ─────────────────────────────────────── */
    int      rodando        = 1;
    SDL_Event event;
    uint64_t perf_freq      = SDL_GetPerformanceFrequency();
    double   target_frame_ms = 1000.0 / (double)TARGET_FPS;
    uint64_t fps_timer      = SDL_GetPerformanceCounter();
    int      frame_count    = 0;
    char     title_buf[128];
    int      ciclos_executados = 0;

    while (rodando) {
        uint64_t frame_start = SDL_GetPerformanceCounter();

        /* 1. Eventos de entrada */
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                rodando = 0;
            } else if (event.type == SDL_KEYDOWN) {
                if (event.key.keysym.sym == SDLK_F2) {
                    alternar_modo_cor(&display);
                } else if (event.key.keysym.sym == SDLK_F3) {
                    alternar_scanlines(&display);
                } else if (event.key.keysym.sym == SDLK_m) {
                    toggle_mute();
                } else {
                    tratar_teclado(&io, event.key.keysym.sym, 1);
                }
            } else if (event.type == SDL_KEYUP) {
                tratar_teclado(&io, event.key.keysym.sym, 0);
            }
        }

        if (!rodando) break;

        /* 2. Primeiro meio frame (~16.666 ciclos → RST 1 no scanline 96) */
        while (ciclos_executados < CYCLES_PER_HALF) {
            int cycles = 0;
            if (exec_instrucao(&cpu, &io, &cycles) != 0) {
                rodando = 0;
                break;
            }
            ciclos_executados += cycles;
        }
        ciclos_executados -= CYCLES_PER_HALF;
        disparar_interrupcao(&cpu, 0xCF);  /* RST 1 */

        if (!rodando) break;

        /* 3. Segundo meio frame (~16.667 ciclos → RST 2 no VBLANK) */
        while (ciclos_executados < (CYCLES_PER_FRAME - CYCLES_PER_HALF)) {
            int cycles = 0;
            if (exec_instrucao(&cpu, &io, &cycles) != 0) {
                rodando = 0;
                break;
            }
            ciclos_executados += cycles;
        }
        ciclos_executados -= (CYCLES_PER_FRAME - CYCLES_PER_HALF);
        disparar_interrupcao(&cpu, 0xD7);  /* RST 2 */

        /* 4. Renderização de vídeo */
        atualizar_video(&display, &cpu);

        /* 5. Sincronização 60 FPS: híbrido SDL_Delay + busy-wait de precisão */
        uint64_t frame_now = SDL_GetPerformanceCounter();
        double elapsed_ms  = (double)(frame_now - frame_start) * 1000.0 / (double)perf_freq;
        double remaining_ms = target_frame_ms - elapsed_ms;

        if (remaining_ms > 2.0) {
            SDL_Delay((Uint32)(remaining_ms - 1.5));
        }
        while (((double)(SDL_GetPerformanceCounter() - frame_start) * 1000.0 / (double)perf_freq)
               < target_frame_ms) { /* busy-wait fino */ }

        /* 6. Atualização do FPS no título */
        frame_count++;
        uint64_t now = SDL_GetPerformanceCounter();
        double sec_diff = (double)(now - fps_timer) / (double)perf_freq;
        if (sec_diff >= 1.0) {
            double current_fps = (double)frame_count / sec_diff;
            snprintf(title_buf, sizeof(title_buf), "%s (%.1f FPS)",
                     driver->nome_completo, current_fps);
            SDL_SetWindowTitle(display.janela, title_buf);
            frame_count = 0;
            fps_timer   = now;
        }
    }

    close_audio();
    fechar_video(&display);
    return 0;
}
