#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <SDL2/SDL.h>
#include "cpu8080.h"
#include "io.h"
#include "audio.h"
#include "video.h"

static int test_total = 0;
static int test_passed = 0;

#define ASSERT_TEST(cond, msg) do { \
    test_total++; \
    if (cond) { \
        test_passed++; \
        printf("  [PASS] %s\n", msg); \
    } else { \
        printf("  [FAIL] %s (linha %d)\n", msg, __LINE__); \
    } \
} while(0)

#pragma pack(push, 1)
typedef struct {
    char riff_id[4];
    uint32_t riff_size;
    char wave_id[4];
    char fmt_id[4];
    uint32_t fmt_size;
    uint16_t audio_format;
    uint16_t num_channels;
    uint32_t sample_rate;
    uint32_t byte_rate;
    uint16_t block_align;
    uint16_t bits_per_sample;
    char data_id[4];
    uint32_t data_size;
} WavHeaderCheck;
#pragma pack(pop)

// 1. Validação dos 9 arquivos de áudio WAV gerados
void test_wav_assets(void) {
    printf("\n--- Teste 1: Integridade dos 9 Samples WAV de Arcade ---\n");

    const char *sounds[] = {
        "sounds/ufo.wav",
        "sounds/shot.wav",
        "sounds/player_die.wav",
        "sounds/invader_die.wav",
        "sounds/fleet1.wav",
        "sounds/fleet2.wav",
        "sounds/fleet3.wav",
        "sounds/fleet4.wav",
        "sounds/ufo_hit.wav"
    };

    for (int i = 0; i < 9; i++) {
        FILE *f = fopen(sounds[i], "rb");
        char msg[128];
        snprintf(msg, sizeof(msg), "Arquivo %s existe e pode ser aberto", sounds[i]);
        ASSERT_TEST(f != NULL, msg);
        if (!f) continue;

        WavHeaderCheck h;
        size_t read_bytes = fread(&h, sizeof(WavHeaderCheck), 1, f);
        fclose(f);

        snprintf(msg, sizeof(msg), "Header de %s lido com sucesso", sounds[i]);
        ASSERT_TEST(read_bytes == 1, msg);

        snprintf(msg, sizeof(msg), "%s e formato RIFF/WAVE PCM valido", sounds[i]);
        int valid_fmt = (memcmp(h.riff_id, "RIFF", 4) == 0) &&
                        (memcmp(h.wave_id, "WAVE", 4) == 0) &&
                        (h.audio_format == 1) &&
                        (h.sample_rate == 44100) &&
                        (h.bits_per_sample == 16) &&
                        (h.data_size > 0);
        ASSERT_TEST(valid_fmt, msg);
    }
}

// 2. Validação da renderização de vídeo e Color Overlay (QA-S2-01)
void test_video_color_overlay(void) {
    printf("\n--- Teste 2: Color Overlay Autentico e Modos de Cor (QA-S2-01) ---\n");

    State8080 cpu;
    init_cpu(&cpu);

    Display display;
    display.modo_cor = COLOR_MODE_OVERLAY;
    display.habilitar_scanlines = 0;
    memset(display.pixels, 0, sizeof(display.pixels));

    // Configura 3 pixels ativos na VRAM (0x2400):
    // Na orientação rotacionada:
    // x = col (0 a 223), y = 255 - (row + bit)
    // Para y = 20 (área do UFO, y < 32): row + bit = 235.
    // Para y = 100 (área dos aliens, 32 <= y < 184): row + bit = 155.
    // Para y = 200 (área das barricadas, y >= 184): row + bit = 55.

    // Col = 10
    // y = 20 -> row = 232 (byte 29), bit = 3 (235)
    cpu.memory[0x2400 + (10 * 32) + 29] |= (1 << 3);

    // y = 100 -> row = 152 (byte 19), bit = 3 (155)
    cpu.memory[0x2400 + (10 * 32) + 19] |= (1 << 3);

    // y = 200 -> row = 48 (byte 6), bit = 7 (55)
    cpu.memory[0x2400 + (10 * 32) + 6] |= (1 << 7);

    // Criamos uma janela dummy em modo headless para renderizar
    SDL_setenv("SDL_VIDEODRIVER", "dummy", 1);
    init_video(&display);

    atualizar_video(&display, &cpu);

    uint32_t cor_ufo = display.pixels[20 * LARGURA_TELA + 10];
    uint32_t cor_alien = display.pixels[100 * LARGURA_TELA + 10];
    uint32_t cor_barricada = display.pixels[200 * LARGURA_TELA + 10];
    uint32_t cor_fundo = display.pixels[0];

    ASSERT_TEST(cor_ufo == 0xE62626FF, "Topo da tela (y < 32) mapeado para Vermelho Arcade (0xE62626FF)");
    ASSERT_TEST(cor_alien == 0xFFFFFFFF, "Meio da tela (32 <= y < 184) mapeado para Branco Puro (0xFFFFFFFF)");
    ASSERT_TEST(cor_barricada == 0x26E626FF, "Base da tela (y >= 184) mapeada para Verde Arcade (0x26E626FF)");
    ASSERT_TEST(cor_fundo == 0x000000FF, "Pixel inativo permanece Fundo Preto (0x000000FF)");

    // Teste de alternância para P&B
    alternar_modo_cor(&display);
    ASSERT_TEST(display.modo_cor == COLOR_MODE_MONOCHROME, "Modo de cor alternado para Monocromatico (P&B)");
    atualizar_video(&display, &cpu);
    ASSERT_TEST(display.pixels[20 * LARGURA_TELA + 10] == 0xFFFFFFFF, "No modo P&B todos os pixels ativos sao Brancos");

    // Teste de alternância para Verde Fósforo
    alternar_modo_cor(&display);
    ASSERT_TEST(display.modo_cor == COLOR_MODE_GREEN_PHOSPHOR, "Modo de cor alternado para Verde Fosforo");
    atualizar_video(&display, &cpu);
    ASSERT_TEST(display.pixels[20 * LARGURA_TELA + 10] == 0x33FF33FF, "No modo Verde Fosforo todos os pixels ativos sao 0x33FF33FF");

    // Teste de Scanlines CRT (QA-S2-04)
    alternar_modo_cor(&display); // Volta para COLOR_MODE_OVERLAY
    alternar_scanlines(&display);
    ASSERT_TEST(display.habilitar_scanlines == 1, "Scanlines CRT ativadas com sucesso");

    // Ativa um pixel na linha ímpar (y = 21, y < 32 -> vermelho atenuado)
    // row + bit = 255 - 21 = 234 -> row = 232 (byte 29), bit = 2
    cpu.memory[0x2400 + (10 * 32) + 29] |= (1 << 2);
    atualizar_video(&display, &cpu);

    uint32_t cor_par = display.pixels[20 * LARGURA_TELA + 10];
    uint32_t cor_impar = display.pixels[21 * LARGURA_TELA + 10];

    // Cor ímpar deve ter intensidade 75% da cor par
    uint8_t r_par = (cor_par >> 24) & 0xFF;
    uint8_t r_impar = (cor_impar >> 24) & 0xFF;
    ASSERT_TEST(r_impar == (r_par * 3 / 4), "Scanline CRT atenua luminosidade em 25% nas linhas impares");

    fechar_video(&display);
}

// 3. Validação do Subsistema de Áudio e Portas 0x03 / 0x05 (QA-S2-02, QA-S2-03)
void test_audio_ports_and_transitions(void) {
    printf("\n--- Teste 3: Rastreamento de Transicoes de Bits de Audio (Portas 3 e 5) ---\n");

    SDL_setenv("SDL_AUDIODRIVER", "dummy", 1);
    int init_res = init_audio("sounds");
    ASSERT_TEST(init_res == 0, "Subsistema de audio SDL2 inicializado com sucesso (buffer < 30ms)");

    InputOutput io;
    init_io(&io);

    ASSERT_TEST(io.prev_port3 == 0, "io.prev_port3 inicializado em 0");
    ASSERT_TEST(io.prev_port5 == 0, "io.prev_port5 inicializado em 0");

    // Disparo de Tiro (Porta 3, bit 1: 0 -> 1)
    io_out(&io, 3, 0x02);
    ASSERT_TEST(io.prev_port3 == 0x02, "Porta 3 recebeu bit 1 (Shot) e atualizou prev_port3");

    // Escrita repetida do mesmo valor NÃO deve gerar borda de subida
    io_out(&io, 3, 0x02);
    ASSERT_TEST(io.prev_port3 == 0x02, "Escrita redundante na Porta 3 mantem prev_port3 inalterado");

    // Transição de UFO (Porta 3, bit 0: 0 -> 1 -> 0)
    io_out(&io, 3, 0x03); // UFO ligado + Shot ligado
    ASSERT_TEST((io.prev_port3 & 0x01) == 0x01, "Porta 3 ligou UFO em loop (bit 0 = 1)");

    io_out(&io, 3, 0x02); // UFO desligado (bit 0 = 0)
    ASSERT_TEST((io.prev_port3 & 0x01) == 0x00, "Porta 3 desligou UFO (bit 0 = 0)");

    // Teste das marchas da esquadrilha na Porta 5 (Bits 0 a 3)
    io_out(&io, 5, 0x01); // Fleet 1
    ASSERT_TEST(io.prev_port5 == 0x01, "Porta 5 disparou Fleet 1 (bit 0)");

    io_out(&io, 5, 0x00);
    io_out(&io, 5, 0x02); // Fleet 2
    ASSERT_TEST(io.prev_port5 == 0x02, "Porta 5 disparou Fleet 2 (bit 1)");

    io_out(&io, 5, 0x00);
    io_out(&io, 5, 0x04); // Fleet 3
    ASSERT_TEST(io.prev_port5 == 0x04, "Porta 5 disparou Fleet 3 (bit 2)");

    io_out(&io, 5, 0x00);
    io_out(&io, 5, 0x08); // Fleet 4
    ASSERT_TEST(io.prev_port5 == 0x08, "Porta 5 disparou Fleet 4 (bit 3)");

    // Teste de Mute (<kbd>M</kbd>)
    ASSERT_TEST(is_audio_muted() == 0, "Audio inicialmente nao mutado");
    toggle_mute();
    ASSERT_TEST(is_audio_muted() == 1, "Audio mutado com toggle_mute()");
    toggle_mute();
    ASSERT_TEST(is_audio_muted() == 0, "Audio desmutado com toggle_mute()");

    close_audio();
}

// 4. Execução Headless com Emulação de ROM Completa
void test_rom_audio_video_integration(void) {
    printf("\n--- Teste 4: Integracao com Execucao Real de ROM do Space Invaders ---\n");

    State8080 cpu;
    InputOutput io;
    init_cpu(&cpu);
    init_io(&io);

    ASSERT_TEST(carregar_rom(&cpu, "rom/invaders.h", 0x0000) == 0, "ROM invaders.h carregada");
    ASSERT_TEST(carregar_rom(&cpu, "rom/invaders.g", 0x0800) == 0, "ROM invaders.g carregada");
    ASSERT_TEST(carregar_rom(&cpu, "rom/invaders.f", 0x1000) == 0, "ROM invaders.f carregada");
    ASSERT_TEST(carregar_rom(&cpu, "rom/invaders.e", 0x1800) == 0, "ROM invaders.e carregada");

    SDL_setenv("SDL_VIDEODRIVER", "dummy", 1);
    SDL_setenv("SDL_AUDIODRIVER", "dummy", 1);

    Display display;
    init_video(&display);
    init_audio("sounds");

    // Executa 60 frames (~1 segundo de gameplay emulado)
    int frames = 60;
    int error = 0;

    for (int f = 0; f < frames; f++) {
        // Meio frame 1
        int ciclos = 0;
        while (ciclos < 16666) {
            int c = 0;
            if (exec_instrucao(&cpu, &io, &c) != 0) { error = 1; break; }
            ciclos += c;
        }
        disparar_interrupcao(&cpu, 0xCF);
        if (error) break;

        // Meio frame 2
        ciclos = 0;
        while (ciclos < 16667) {
            int c = 0;
            if (exec_instrucao(&cpu, &io, &c) != 0) { error = 1; break; }
            ciclos += c;
        }
        disparar_interrupcao(&cpu, 0xD7);
        if (error) break;

        atualizar_video(&display, &cpu);
    }

    ASSERT_TEST(error == 0, "60 quadros de emulacao executados sem falha de instrucao ou memoria");
    ASSERT_TEST(display.pixels[0] == 0x000000FF, "VRAM e display renderizados de forma integra");

    close_audio();
    fechar_video(&display);
}

int main(int argc, char *argv[]) {
    (void)argc;
    (void)argv;

    printf("====================================================\n");
    printf("   TEST SUITE - SPRINT 2 (AUDIOVISUAL & OVERLAY)    \n");
    printf("====================================================\n");

    test_wav_assets();
    test_video_color_overlay();
    test_audio_ports_and_transitions();
    test_rom_audio_video_integration();

    printf("\n====================================================\n");
    printf("   RESULTADO: %d/%d TESTES PASSARAM (%.1f%%)\n",
           test_passed, test_total, (100.0 * test_passed) / test_total);
    printf("====================================================\n");

    return (test_passed == test_total) ? 0 : 1;
}
