#include <stdio.h>
#include "video.h"

int init_video(Display *display) {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("Erro ao inicializar SDL: %s\n", SDL_GetError());
        return -1;
    }

    display->janela = SDL_CreateWindow(
        "Space Invaders - Emulator",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        LARGURA_TELA * ESCALA_TELA,
        ALTURA_TELA * ESCALA_TELA,
        SDL_WINDOW_SHOWN
    );

    if (!display->janela) {
        printf("Erro ao criar janela: %s\n", SDL_GetError());
        SDL_Quit();
        return -1;
    }

    display->renderer = SDL_CreateRenderer(display->janela, -1, SDL_RENDERER_ACCELERATED);
    if (!display->renderer) {
        printf("Erro ao criar renderer: %s\n", SDL_GetError());
        SDL_DestroyWindow(display->janela);
        SDL_Quit();
        return -1;
    }

    display->textura = SDL_CreateTexture(
        display->renderer,
        SDL_PIXELFORMAT_RGBA8888,
        SDL_TEXTUREACCESS_STREAMING,
        LARGURA_TELA,
        ALTURA_TELA
    );

    if (!display->textura) {
        printf("Erro ao criar textura: %s\n", SDL_GetError());
        SDL_DestroyRenderer(display->renderer);
        SDL_DestroyWindow(display->janela);
        SDL_Quit();
        return -1;
    }

    return 0;
}

// Renderiza a VRAM (0x2400 a 0x3FFF) ajustando a rotação de 90° do arcade original
void atualizar_video(Display *display, const State8080 *cpu) {
    uint16_t vram_start = 0x2400;

    for (int col = 0; col < 224; col++) {
        for (int row = 0; row < 256; row += 8) {
            uint8_t byte = cpu->memory[vram_start + (col * 32) + (row / 8)];

            for (int bit = 0; bit < 8; bit++) {
                uint8_t pixel_ativo = (byte >> bit) & 1;

                // Mapeia as coordenadas rotacionadas para a tela na orientação correta
                int x = col;
                int y = 255 - (row + bit);

                uint32_t cor = pixel_ativo ? 0xFFFFFFFF : 0x000000FF; // Branco ou Preto
                display->pixels[y * LARGURA_TELA + x] = cor;
            }
        }
    }

    SDL_UpdateTexture(display->textura, NULL, display->pixels, LARGURA_TELA * sizeof(uint32_t));
    SDL_RenderClear(display->renderer);
    SDL_RenderCopy(display->renderer, display->textura, NULL, NULL);
    SDL_RenderPresent(display->renderer);
}

void fechar_video(Display *display) {
    if (display->textura) SDL_DestroyTexture(display->textura);
    if (display->renderer) SDL_DestroyRenderer(display->renderer);
    if (display->janela) SDL_DestroyWindow(display->janela);
    SDL_Quit();
}