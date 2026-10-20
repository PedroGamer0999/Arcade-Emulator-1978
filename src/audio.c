#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "audio.h"

#define MAX_MIX_SAMPLES 4096

typedef struct {
    int active;
    SoundID id;
    int loop;
    uint32_t position; // posicao em bytes no buffer do som
} Channel;

static SDL_AudioDeviceID audio_device = 0;
static SDL_AudioSpec device_spec;

static uint8_t *sound_buffers[SND_COUNT] = {0};
static uint32_t sound_lengths[SND_COUNT] = {0};

static Channel channels[MAX_CHANNELS];
static int audio_muted = 0;

static const char *sound_filenames[SND_COUNT] = {
    "ufo.wav",
    "shot.wav",
    "player_die.wav",
    "invader_die.wav",
    "fleet1.wav",
    "fleet2.wav",
    "fleet3.wav",
    "fleet4.wav",
    "ufo_hit.wav"
};

static void audio_callback(void *userdata, Uint8 *stream, int len) {
    (void)userdata;
    memset(stream, 0, len);

    if (audio_muted) {
        return;
    }

    int num_samples = len / (int)sizeof(int16_t);
    if (num_samples > MAX_MIX_SAMPLES) {
        num_samples = MAX_MIX_SAMPLES;
    }

    static int32_t mix_buffer[MAX_MIX_SAMPLES];
    memset(mix_buffer, 0, num_samples * sizeof(int32_t));

    for (int ch = 0; ch < MAX_CHANNELS; ch++) {
        if (!channels[ch].active) continue;

        SoundID id = channels[ch].id;
        if (id >= SND_COUNT) {
            channels[ch].active = 0;
            continue;
        }

        const uint8_t *buf = sound_buffers[id];
        uint32_t snd_len = sound_lengths[id];
        uint32_t pos = channels[ch].position;

        if (!buf || snd_len == 0) {
            channels[ch].active = 0;
            continue;
        }

        if (channels[ch].loop) {
            for (int i = 0; i < num_samples; i++) {
                const int16_t *sample_ptr = (const int16_t *)(buf + pos);
                mix_buffer[i] += *sample_ptr;
                pos += sizeof(int16_t);
                if (pos + sizeof(int16_t) > snd_len) {
                    pos = 0;
                }
            }
            channels[ch].position = pos;
        } else {
            const int16_t *src = (const int16_t *)(buf + pos);
            uint32_t remaining_bytes = snd_len - pos;
            int samples_to_mix = num_samples;
            int remaining_samples = (int)(remaining_bytes / sizeof(int16_t));

            if (samples_to_mix > remaining_samples) {
                samples_to_mix = remaining_samples;
            }

            for (int i = 0; i < samples_to_mix; i++) {
                mix_buffer[i] += src[i];
            }

            channels[ch].position += (uint32_t)(samples_to_mix * sizeof(int16_t));
            if (channels[ch].position >= snd_len) {
                channels[ch].active = 0;
            }
        }
    }

    int16_t *out = (int16_t *)stream;
    for (int i = 0; i < num_samples; i++) {
        int32_t val = mix_buffer[i];
        if (val > 32767) val = 32767;
        else if (val < -32768) val = -32768;
        out[i] = (int16_t)val;
    }
}

static int carregar_som(SoundID id, const char *filepath) {
    SDL_AudioSpec wav_spec;
    Uint8 *wav_buf = NULL;
    Uint32 wav_len = 0;

    if (!SDL_LoadWAV(filepath, &wav_spec, &wav_buf, &wav_len)) {
        printf("Aviso: Nao foi possivel carregar som %s: %s\n", filepath, SDL_GetError());
        return -1;
    }

    SDL_AudioCVT cvt;
    if (SDL_BuildAudioCVT(&cvt, wav_spec.format, wav_spec.channels, wav_spec.freq,
                          device_spec.format, device_spec.channels, device_spec.freq) < 0) {
        printf("Erro ao construir conversao de audio para %s: %s\n", filepath, SDL_GetError());
        SDL_FreeWAV(wav_buf);
        return -1;
    }

    if (cvt.needed) {
        cvt.buf = (Uint8 *)malloc(wav_len * (size_t)cvt.len_mult);
        if (!cvt.buf) {
            SDL_FreeWAV(wav_buf);
            return -1;
        }
        memcpy(cvt.buf, wav_buf, wav_len);
        cvt.len = (int)wav_len;
        SDL_ConvertAudio(&cvt);
        SDL_FreeWAV(wav_buf);

        sound_buffers[id] = cvt.buf;
        sound_lengths[id] = (uint32_t)cvt.len_cvt;
    } else {
        sound_buffers[id] = (Uint8 *)malloc(wav_len);
        if (sound_buffers[id]) {
            memcpy(sound_buffers[id], wav_buf, wav_len);
            sound_lengths[id] = wav_len;
        }
        SDL_FreeWAV(wav_buf);
    }

    return 0;
}

int init_audio(const char *sounds_dir) {
    if (SDL_InitSubSystem(SDL_INIT_AUDIO) < 0) {
        printf("Erro ao inicializar subsistema de audio SDL: %s\n", SDL_GetError());
        return -1;
    }

    SDL_AudioSpec desired;
    SDL_zero(desired);
    desired.freq = 44100;
    desired.format = AUDIO_S16SYS;
    desired.channels = 2; // Stereo
    desired.samples = 512; // ~11.6ms de latencia
    desired.callback = audio_callback;
    desired.userdata = NULL;

    audio_device = SDL_OpenAudioDevice(NULL, 0, &desired, &device_spec, 0);
    if (audio_device == 0) {
        printf("Erro ao abrir dispositivo de audio SDL: %s\n", SDL_GetError());
        return -1;
    }

    for (int i = 0; i < MAX_CHANNELS; i++) {
        channels[i].active = 0;
        channels[i].id = SND_COUNT;
        channels[i].loop = 0;
        channels[i].position = 0;
    }

    char path[256];
    for (int i = 0; i < SND_COUNT; i++) {
        snprintf(path, sizeof(path), "%s/%s", sounds_dir ? sounds_dir : "sounds", sound_filenames[i]);
        carregar_som((SoundID)i, path);
    }

    SDL_PauseAudioDevice(audio_device, 0);
    return 0;
}

void play_sound(SoundID id, int loop) {
    if (!audio_device || id >= SND_COUNT || !sound_buffers[id]) return;

    SDL_LockAudioDevice(audio_device);

    // Se o som já estiver tocando, reinicia
    for (int i = 0; i < MAX_CHANNELS; i++) {
        if (channels[i].active && channels[i].id == id) {
            channels[i].position = 0;
            channels[i].loop = loop;
            SDL_UnlockAudioDevice(audio_device);
            return;
        }
    }

    int slot = -1;
    for (int i = 0; i < MAX_CHANNELS; i++) {
        if (!channels[i].active) {
            slot = i;
            break;
        }
    }

    if (slot == -1) {
        for (int i = 0; i < MAX_CHANNELS; i++) {
            if (!channels[i].loop) {
                slot = i;
                break;
            }
        }
    }

    if (slot != -1) {
        channels[slot].active = 1;
        channels[slot].id = id;
        channels[slot].loop = loop;
        channels[slot].position = 0;
    }

    SDL_UnlockAudioDevice(audio_device);
}

void stop_sound(SoundID id) {
    if (!audio_device) return;

    SDL_LockAudioDevice(audio_device);
    for (int i = 0; i < MAX_CHANNELS; i++) {
        if (channels[i].active && channels[i].id == id) {
            channels[i].active = 0;
        }
    }
    SDL_UnlockAudioDevice(audio_device);
}

void update_sound_port3(uint8_t value, uint8_t previous_value) {
    uint8_t rising_edge = value & ~previous_value;

    // Bit 0: UFO looping
    if (rising_edge & 0x01) {
        play_sound(SND_UFO, 1);
    } else if ((previous_value & 0x01) && !(value & 0x01)) {
        stop_sound(SND_UFO);
    }

    // Bit 1: Tiro
    if (rising_edge & 0x02) {
        play_sound(SND_SHOT, 0);
    }

    // Bit 2: Morte do jogador
    if (rising_edge & 0x04) {
        play_sound(SND_PLAYER_DIE, 0);
    }

    // Bit 3: Explosão de alien
    if (rising_edge & 0x08) {
        play_sound(SND_INVADER_DIE, 0);
    }
}

void update_sound_port5(uint8_t value, uint8_t previous_value) {
    uint8_t rising_edge = value & ~previous_value;

    // Bit 0: Marcha 1
    if (rising_edge & 0x01) {
        play_sound(SND_FLEET1, 0);
    }

    // Bit 1: Marcha 2
    if (rising_edge & 0x02) {
        play_sound(SND_FLEET2, 0);
    }

    // Bit 2: Marcha 3
    if (rising_edge & 0x04) {
        play_sound(SND_FLEET3, 0);
    }

    // Bit 3: Marcha 4
    if (rising_edge & 0x08) {
        play_sound(SND_FLEET4, 0);
    }

    // Bit 4: Impacto no UFO
    if (rising_edge & 0x10) {
        play_sound(SND_UFO_HIT, 0);
    }
}

void toggle_mute(void) {
    audio_muted = !audio_muted;
    printf("[Audio] %s\n", audio_muted ? "Silenciado (MUTE)" : "Ativado");
}

int is_audio_muted(void) {
    return audio_muted;
}

void close_audio(void) {
    if (audio_device) {
        SDL_CloseAudioDevice(audio_device);
        audio_device = 0;
    }
    for (int i = 0; i < SND_COUNT; i++) {
        if (sound_buffers[i]) {
            free(sound_buffers[i]);
            sound_buffers[i] = NULL;
            sound_lengths[i] = 0;
        }
    }
}
