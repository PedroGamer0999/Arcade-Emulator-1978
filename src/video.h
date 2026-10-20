#ifndef VIDEO_H
#define VIDEO_H

#include <SDL2/SDL.h>
#include <stdint.h>
#include "cpu8080.h"

#define LARGURA_TELA 224
#define ALTURA_TELA  256
#define ESCALA_TELA  2   // Ajuste para aumentar o tamanho da janela (224x256 -> 448x512)

typedef enum {
    COLOR_MODE_OVERLAY = 0,
    COLOR_MODE_MONOCHROME,
    COLOR_MODE_GREEN_PHOSPHOR,
    COLOR_MODE_COUNT
} ColorMode;

typedef struct {
    SDL_Window *janela;
    SDL_Renderer *renderer;
    SDL_Texture *textura;
    uint32_t pixels[LARGURA_TELA * ALTURA_TELA];
    ColorMode modo_cor;
    int habilitar_scanlines;
} Display;

int init_video(Display *display);
void atualizar_video(Display *display, const State8080 *cpu);
void alternar_modo_cor(Display *display);
void alternar_scanlines(Display *display);
void fechar_video(Display *display);

#endif
