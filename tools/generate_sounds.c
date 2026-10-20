#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <string.h>

#define SAMPLE_RATE 44100
#define PI 3.14159265358979323846

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
} WavHeader;
#pragma pack(pop)

static void salvar_wav(const char *caminho, const int16_t *samples, int num_samples) {
    FILE *f = fopen(caminho, "wb");
    if (!f) {
        printf("Erro ao criar arquivo: %s\n", caminho);
        return;
    }

    WavHeader h;
    memcpy(h.riff_id, "RIFF", 4);
    h.data_size = (uint32_t)(num_samples * sizeof(int16_t));
    h.riff_size = (uint32_t)(sizeof(WavHeader) - 8 + h.data_size);
    memcpy(h.wave_id, "WAVE", 4);
    memcpy(h.fmt_id, "fmt ", 4);
    h.fmt_size = 16;
    h.audio_format = 1; // PCM
    h.num_channels = 1; // Mono
    h.sample_rate = SAMPLE_RATE;
    h.bits_per_sample = 16;
    h.byte_rate = SAMPLE_RATE * h.num_channels * (h.bits_per_sample / 8);
    h.block_align = h.num_channels * (h.bits_per_sample / 8);
    memcpy(h.data_id, "data", 4);

    fwrite(&h, sizeof(WavHeader), 1, f);
    fwrite(samples, sizeof(int16_t), num_samples, f);
    fclose(f);
    printf("Gerado: %s (%d samples)\n", caminho, num_samples);
}

// Pseudo-aleatório determinístico para ruído
static double ruido(void) {
    return ((double)rand() / (double)RAND_MAX) * 2.0 - 1.0;
}

// Clamp para int16
static inline int16_t clamp16(double val) {
    if (val > 32767.0) return 32767;
    if (val < -32768.0) return -32768;
    return (int16_t)val;
}

// 1. UFO Sound (Loop perfeito, ~0.5s de duração)
static void gerar_ufo(const char *path) {
    double duration = 0.5; // Exatamente 3 ciclos de LFO a 6Hz
    int n = (int)(duration * SAMPLE_RATE);
    int16_t *buf = (int16_t*)malloc(n * sizeof(int16_t));

    double phase = 0.0;
    for (int i = 0; i < n; i++) {
        double t = (double)i / SAMPLE_RATE;
        // LFO modula a frequência de 400Hz a 550Hz a 6.0Hz
        double freq = 475.0 + 75.0 * sin(2.0 * PI * 6.0 * t);
        phase += 2.0 * PI * freq / SAMPLE_RATE;
        if (phase > 2.0 * PI) phase -= 2.0 * PI;

        // Onda senoidal levemente distorcida (arcade buzzer)
        double s = sin(phase) + 0.3 * sin(2.0 * phase);
        buf[i] = clamp16(s * 18000.0);
    }
    salvar_wav(path, buf, n);
    free(buf);
}

// 2. Shot Sound (Laser do jogador)
static void gerar_shot(const char *path) {
    double duration = 0.32;
    int n = (int)(duration * SAMPLE_RATE);
    int16_t *buf = (int16_t*)malloc(n * sizeof(int16_t));

    double phase = 0.0;
    for (int i = 0; i < n; i++) {
        double t = (double)i / SAMPLE_RATE;
        // Sweep descendente exponencial de 2800 Hz ate 200 Hz
        double freq = 200.0 + 2600.0 * exp(-12.0 * t);
        phase += 2.0 * PI * freq / SAMPLE_RATE;

        // Onda dente de serra + onda quadrada
        double saw = 2.0 * (phase / (2.0 * PI) - floor(phase / (2.0 * PI) + 0.5));
        double env = exp(-7.5 * t);
        double s = saw * env;

        buf[i] = clamp16(s * 25000.0);
    }
    salvar_wav(path, buf, n);
    free(buf);
}

// 3. Player Die Sound (Explosão do jogador com tremolo descendente)
static void gerar_player_die(const char *path) {
    double duration = 1.25;
    int n = (int)(duration * SAMPLE_RATE);
    int16_t *buf = (int16_t*)malloc(n * sizeof(int16_t));

    double phase = 0.0;
    for (int i = 0; i < n; i++) {
        double t = (double)i / SAMPLE_RATE;
        // Modulação de tom grave com ruído e batimento
        double freq = 120.0 - 80.0 * (t / duration);
        phase += 2.0 * PI * freq / SAMPLE_RATE;

        double tremolo = 0.5 + 0.5 * sin(2.0 * PI * 16.0 * t);
        double rumble = sin(phase);
        double n_sample = ruido();
        double env = exp(-2.2 * t);

        double s = (0.6 * rumble + 0.4 * n_sample) * tremolo * env;
        buf[i] = clamp16(s * 24000.0);
    }
    salvar_wav(path, buf, n);
    free(buf);
}

// 4. Invader Die Sound (Pop/explosão alienígena rápida)
static void gerar_invader_die(const char *path) {
    double duration = 0.16;
    int n = (int)(duration * SAMPLE_RATE);
    int16_t *buf = (int16_t*)malloc(n * sizeof(int16_t));

    double phase = 0.0;
    for (int i = 0; i < n; i++) {
        double t = (double)i / SAMPLE_RATE;
        double freq = 800.0 * exp(-20.0 * t) + 120.0;
        phase += 2.0 * PI * freq / SAMPLE_RATE;

        double n_sample = ruido();
        double chirp = sin(phase);
        double env = exp(-18.0 * t);

        double s = (0.5 * n_sample + 0.5 * chirp) * env;
        buf[i] = clamp16(s * 26000.0);
    }
    salvar_wav(path, buf, n);
    free(buf);
}

// 5-8. Fleet March Sounds (Marcha alienígena 1 a 4)
static void gerar_fleet(const char *path, double freq_base) {
    double duration = 0.12;
    int n = (int)(duration * SAMPLE_RATE);
    int16_t *buf = (int16_t*)malloc(n * sizeof(int16_t));

    double phase = 0.0;
    for (int i = 0; i < n; i++) {
        double t = (double)i / SAMPLE_RATE;
        // Pitch levemente descendente no impacto inicial
        double f = freq_base * (1.0 + 0.3 * exp(-50.0 * t));
        phase += 2.0 * PI * f / SAMPLE_RATE;

        // Onda quadrada analógica com harmônicos
        double sq = (sin(phase) >= 0.0) ? 1.0 : -1.0;
        double env = exp(-14.0 * t);

        buf[i] = clamp16(sq * env * 22000.0);
    }
    salvar_wav(path, buf, n);
    free(buf);
}

// 9. UFO Hit Sound (Impacto na nave-mãe)
static void gerar_ufo_hit(const char *path) {
    double duration = 0.45;
    int n = (int)(duration * SAMPLE_RATE);
    int16_t *buf = (int16_t*)malloc(n * sizeof(int16_t));

    double phase = 0.0;
    for (int i = 0; i < n; i++) {
        double t = (double)i / SAMPLE_RATE;
        double freq = 1200.0 * exp(-10.0 * t) + 150.0;
        phase += 2.0 * PI * freq / SAMPLE_RATE;

        double n_sample = ruido();
        double sweep = sin(phase);
        double env = exp(-5.5 * t);

        double s = (0.6 * n_sample + 0.4 * sweep) * env;
        buf[i] = clamp16(s * 26000.0);
    }
    salvar_wav(path, buf, n);
    free(buf);
}

int main(void) {
    srand(1978); // Semente fixa do ano de lançamento para reprodutibilidade

    printf("Gerando banco de sons autenticos do Space Invaders (1978)...\n");

    gerar_ufo("sounds/ufo.wav");
    gerar_shot("sounds/shot.wav");
    gerar_player_die("sounds/player_die.wav");
    gerar_invader_die("sounds/invader_die.wav");

    // As 4 notas da marcha alienígena (descendente F3 -> E3 -> Eb3 -> D3)
    gerar_fleet("sounds/fleet1.wav", 174.6);
    gerar_fleet("sounds/fleet2.wav", 164.8);
    gerar_fleet("sounds/fleet3.wav", 155.6);
    gerar_fleet("sounds/fleet4.wav", 146.8);

    gerar_ufo_hit("sounds/ufo_hit.wav");

    printf("Todos os 9 arquivos WAV foram gerados com sucesso na pasta sounds/!\n");
    return 0;
}
