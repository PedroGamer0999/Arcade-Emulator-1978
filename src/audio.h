#ifndef AUDIO_H
#define AUDIO_H

#include <stdint.h>
#include <SDL2/SDL.h>

#define MAX_CHANNELS 8

typedef enum {
    SND_UFO = 0,
    SND_SHOT,
    SND_PLAYER_DIE,
    SND_INVADER_DIE,
    SND_FLEET1,
    SND_FLEET2,
    SND_FLEET3,
    SND_FLEET4,
    SND_UFO_HIT,
    SND_COUNT
} SoundID;

int init_audio(const char *sounds_dir);
void play_sound(SoundID id, int loop);
void stop_sound(SoundID id);
void update_sound_port3(uint8_t value, uint8_t previous_value);
void update_sound_port5(uint8_t value, uint8_t previous_value);
void toggle_mute(void);
int is_audio_muted(void);
void close_audio(void);

#endif
