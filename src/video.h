#ifndef VIDEO_H
#define VIDEO_H

#include <SDL2/SDL.h>
#include <stdint.h>
#include "cpu8080.h"

#define LARGURA_TELA 224
#define ALTURA_TELA  256
#define ESCALA_TELA  2   // Ajuste para aumentar o tamanho da janela (224x256 -> 448x512)

typedef struct {
    SDL_Window *janela;
    SDL_Renderer *renderer;
    SDL_Texture *textura;
    uint32_t pixels[LARGURA_TELA * ALTURA_TELA];
} Display;

int init_video(Display *display);
void atualizar_video(Display *display, const State8080 *cpu);
void fechar_video(Display *display);

#endif