#include <stdio.h>
#include <SDL2/SDL.h>
#include "cpu8080.h"
#include "io.h"
#include "video.h"

void gerar_interrupcao(State8080 *cpu, uint8_t opcode_rst) {
    cpu->sp -= 2;
    cpu->memory[cpu->sp] = cpu->pc & 0xFF;
    cpu->memory[cpu->sp + 1] = (cpu->pc >> 8) & 0xFF;
    
    if (opcode_rst == 0xCF) cpu->pc = 0x0008;      // RST 1
    else if (opcode_rst == 0xD7) cpu->pc = 0x0010; // RST 2
}

int main(int argc, char *argv[]) {
    (void)argc; (void)argv;

    State8080 cpu;
    InputOutput io;
    Display display;

    init_cpu(&cpu);
    init_io(&io);

    if (init_video(&display) != 0) return 1;

    if (carregar_rom(&cpu, "rom/invaders.h", 0x0000) != 0 ||
        carregar_rom(&cpu, "rom/invaders.g", 0x0800) != 0 ||
        carregar_rom(&cpu, "rom/invaders.f", 0x1000) != 0 ||
        carregar_rom(&cpu, "rom/invaders.e", 0x1800) != 0) {
        printf("Erro ao carregar ROMs.\n");
        fechar_video(&display);
        return 1;
    }

    int rodando = 1;
    SDL_Event event;
    uint8_t proxima_interrupcao = 0xCF;

    while (rodando) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                rodando = 0;
            } else if (event.type == SDL_KEYDOWN) {
                tratar_teclado(&io, event.key.keysym.sym, 1);
            } else if (event.type == SDL_KEYUP) {
                tratar_teclado(&io, event.key.keysym.sym, 0);
            }
        }

        // Executa instruções da CPU por meio frame
        for (int i = 0; i < 2000; i++) {
            if (exec_instrucao(&cpu, &io) != 0) {
                rodando = 0;
                break;
            }
        }

        gerar_interrupcao(&cpu, proxima_interrupcao);

        if (proxima_interrupcao == 0xCF) {
            proxima_interrupcao = 0xD7;
        } else {
            proxima_interrupcao = 0xCF;
            atualizar_video(&display, &cpu);
            SDL_Delay(16);
        }
    }

    fechar_video(&display);
    return 0;
}