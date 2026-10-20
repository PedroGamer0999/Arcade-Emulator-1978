#include <stdio.h>
#include <SDL2/SDL.h>
#include "cpu8080.h"
#include "io.h"
#include "video.h"
#include "audio.h"

// Constantes de clock e temporização do Intel 8080
#define CPU_CLOCK_HZ        2000000
#define TARGET_FPS          60
#define CYCLES_PER_FRAME    (CPU_CLOCK_HZ / TARGET_FPS)    // 33.333 ciclos
#define CYCLES_PER_HALF     (CYCLES_PER_FRAME / 2)         // 16.666 ciclos

int main(int argc, char *argv[]) {
    (void)argc; (void)argv;

    State8080 cpu;
    InputOutput io;
    Display display;

    init_cpu(&cpu);
    init_io(&io);

    if (init_video(&display) != 0) return 1;
    init_audio("sounds");

    if (carregar_rom(&cpu, "rom/invaders.h", 0x0000) != 0 ||
        carregar_rom(&cpu, "rom/invaders.g", 0x0800) != 0 ||
        carregar_rom(&cpu, "rom/invaders.f", 0x1000) != 0 ||
        carregar_rom(&cpu, "rom/invaders.e", 0x1800) != 0) {
        printf("Erro ao carregar ROMs.\n");
        close_audio();
        fechar_video(&display);
        return 1;
    }

    int rodando = 1;
    SDL_Event event;

    // Frequência do contador de alta resolução do SDL
    uint64_t perf_freq = SDL_GetPerformanceFrequency();
    double target_frame_ms = 1000.0 / (double)TARGET_FPS; // ~16.6667 ms

    // Variáveis para medição e exibição de FPS
    uint64_t fps_timer = SDL_GetPerformanceCounter();
    int frame_count = 0;
    char title_buf[64];

    // Acumulador de ciclos para compensar eventuais excessos entre fatias
    int ciclos_executados = 0;

    while (rodando) {
        uint64_t frame_start = SDL_GetPerformanceCounter();

        // 1. Processa eventos de entrada (teclado e janela)
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

        // 2. Primeiro meio frame: executa ~16.666 ciclos até a metade da tela (scanline 96)
        while (ciclos_executados < CYCLES_PER_HALF) {
            int cycles = 0;
            if (exec_instrucao(&cpu, &io, &cycles) != 0) {
                rodando = 0;
                break;
            }
            ciclos_executados += cycles;
        }
        ciclos_executados -= CYCLES_PER_HALF;

        // Dispara RST 1 (0xCF) para scanline 96 (se interrupções estiverem habilitadas)
        disparar_interrupcao(&cpu, 0xCF);

        if (!rodando) break;

        // 3. Segundo meio frame: executa ~16.667 ciclos até o final da tela (VBLANK)
        while (ciclos_executados < (CYCLES_PER_FRAME - CYCLES_PER_HALF)) {
            int cycles = 0;
            if (exec_instrucao(&cpu, &io, &cycles) != 0) {
                rodando = 0;
                break;
            }
            ciclos_executados += cycles;
        }
        ciclos_executados -= (CYCLES_PER_FRAME - CYCLES_PER_HALF);

        // Dispara RST 2 (0xD7) para VBLANK (se interrupções estiverem habilitadas)
        disparar_interrupcao(&cpu, 0xD7);

        // 4. Atualiza a renderização de vídeo
        atualizar_video(&display, &cpu);

        // 5. Sincronização 60 FPS: Híbrido SDL_Delay + Busy-Wait de alta precisão
        uint64_t frame_now = SDL_GetPerformanceCounter();
        double elapsed_ms = (double)(frame_now - frame_start) * 1000.0 / (double)perf_freq;
        double remaining_ms = target_frame_ms - elapsed_ms;

        // Se sobrar tempo considerável (>2ms), dorme para poupar CPU
        if (remaining_ms > 2.0) {
            SDL_Delay((Uint32)(remaining_ms - 1.5));
        }

        // Busy-wait de precisão nos microssegundos finais para cravar exatamente 60 FPS
        while (((double)(SDL_GetPerformanceCounter() - frame_start) * 1000.0 / (double)perf_freq) < target_frame_ms) {
            // Espera ativa fina
        }

        // 6. Atualização do medidor de FPS no título da janela
        frame_count++;
        uint64_t now = SDL_GetPerformanceCounter();
        double sec_diff = (double)(now - fps_timer) / (double)perf_freq;
        if (sec_diff >= 1.0) {
            double current_fps = (double)frame_count / sec_diff;
            snprintf(title_buf, sizeof(title_buf), "Space Invaders Arcade (%.1f FPS)", current_fps);
            SDL_SetWindowTitle(display.janela, title_buf);
            frame_count = 0;
            fps_timer = now;
        }
    }

    close_audio();
    fechar_video(&display);
    return 0;
}
