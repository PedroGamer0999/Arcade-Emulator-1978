# SPEC-003: Sistema de Áudio SDL2 e Mixer de Efeitos Sonoros

- **Status:** Aprovado para Implementação (Sprint 2)
- **Autor:** Game Developer / Audio Systems Specialist
- **Arquivos Afetados:** [`src/io.h`](file:///D:/projetos/008-space-invaders-emulator/src/io.h), [`src/io.c`](file:///D:/projetos/008-space-invaders-emulator/src/io.c), `src/audio.h`, `src/audio.c`, [`src/main.c`](file:///D:/projetos/008-space-invaders-emulator/src/main.c)
- **Dependências:** [`SPEC-001`](SPEC-001-cpu-timing-interrupts.md)

---

## 1. Contexto e Motivação
O arcade Space Invaders original não possuía um processador de áudio digital programável (como o YM2149 ou AY-3-8910). Ele contava com uma série de circuitos eletrônicos analógicos e um gerador de efeitos sonoros Texas Instruments SN76477 dedicados na placa-mãe.

Esses geradores de som eram ativados diretamente pela CPU Intel 8080 através das **Portas de Saída 0x03 e 0x05**:
- Quando a CPU grava um byte nessas portas, uma transição de bit de `0` para `1` dispara um som correspondente.
- Atualmente, o arquivo [`src/io.c`](file:///D:/projetos/008-space-invaders-emulator/src/io.c#L28) descarta completamente as escritas nas portas 3 e 5, deixando o emulador completamente silencioso.
- Para atingir nível de produto final, precisamos integrar o subsistema de áudio da biblioteca SDL2 com um mixer de baixa latência em C puro e carregar os samples sonoros originais em formato WAV.

---

## 2. Requisitos

### Requisitos Funcionais (RF)
- **RF-01**: Detectar a transição de borda de subida (bit mudando de `0` para `1`) nas portas `0x03` e `0x05`:
  - **Porta 0x03 (Sound Bank 1):**
    - Bit 0 (`0x01`): Som contínuo da nave UFO (looping enquanto o bit permanecer ligado)
    - Bit 1 (`0x02`): Disparo do tiro do jogador (*Shot*)
    - Bit 2 (`0x04`): Morte / Destruição do jogador (*Player Die*)
    - Bit 3 (`0x08`): Explosão de um invasor atingido (*Invader Die*)
    - Bit 4 (`0x10`): Flash de tela estendido (opcional)
  - **Porta 0x05 (Sound Bank 2):**
    - Bit 0 (`0x01`): Marcha dos invasores - Som 1 (*Fast Invader 1*)
    - Bit 1 (`0x02`): Marcha dos invasores - Som 2 (*Fast Invader 2*)
    - Bit 2 (`0x04`): Marcha dos invasores - Som 3 (*Fast Invader 3*)
    - Bit 3 (`0x08`): Marcha dos invasores - Som 4 (*Fast Invader 4*)
    - Bit 4 (`0x10`): Nave-mãe UFO atingida (*UFO Hit*)
- **RF-02**: Reproduzir os samples sonoros em buffer de áudio SDL2 com latência inferior a 30 ms.
- **RF-03**: Suportar múltiplos sons tocando simultaneamente (mixagem em tempo real de até 8 canais).
- **RF-04**: O som do UFO (Porta 3, bit 0) deve parar imediatamente quando o bit voltar para `0`.
- **RF-05**: Controle de volume geral ajustável (0% a 100%) e mute via tecla <kbd>M</kbd>.

---

## 3. Especificação Técnica e Arquitetura

### 3.1 Arquitetura do Subsistema de Áudio (`src/audio.h`)
```c
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
void close_audio(void);

#endif
```

### 3.2 Integração em `src/io.c`
Adicionar campos para rastrear o estado anterior das portas de som:
```c
void io_out(InputOutput *io, uint8_t port, uint8_t value) {
    switch (port) {
        case 2:
            io->shift_offset = value & 0x07;
            break;
        case 3:
            update_sound_port3(value, io->prev_port3);
            io->prev_port3 = value;
            break;
        case 4:
            io->shift_register = (io->shift_register >> 8) | ((uint16_t)value << 8);
            break;
        case 5:
            update_sound_port5(value, io->prev_port5);
            io->prev_port5 = value;
            break;
    }
}
```

### 3.3 Mixer de Baixa Latência em `src/audio.c`
Utilizar callback do SDL:
- Frequência: 44.100 Hz, 16-bit PCM estéreo/mono.
- Tamanho de buffer: 512 ou 1024 samples (~11.6 ms a 23.2 ms de latência).
- Cada canal ativo acumula seus samples em `int32_t` e aplica *clipping clamp* para evitar distorção harmônica.

---

## 4. Critérios de Aceite (Definition of Done)
- [x] O canhão produz o efeito sonoro clássico de disparo ao apertar <kbd>Espaço</kbd>.
- [x] A marcha da esquadrilha reproduz os 4 tons alternados (Fleet 1, 2, 3, 4) conforme os invasores descem.
- [x] A nave UFO toca o som em loop apenas enquanto estiver na tela.
- [x] Tecla <kbd>M</kbd> ativa/desativa o áudio instantaneamente.
- [x] O mixer roda em thread de áudio dedicada do SDL sem bloquear o loop principal da CPU.

---

## 5. Roteiro de Testes de Q/A

| Teste | Descrição | Ação | Resultado Esperado |
| :--- | :--- | :--- | :--- |
| **TC-003-A** | Som de Tiro | Iniciar partida e disparar repetidamente. | O som de laser toca com resposta instantânea a cada disparo sem travamentos. |
| **TC-003-B** | Marcha dos Invasores | Acompanhar a movimentação da esquadrilha alienígena. | Os sons 1, 2, 3 e 4 alternam no ritmo exato dos passos dos invasores, acelerando na reta final da fase. |
| **TC-003-C** | Som da Morte | Permitir que um tiro inimigo atinja o canhão do jogador. | Som dramático de explosão toca durante a animação de destruição. |
| **TC-003-D** | Tecla Mute | Pressionar <kbd>M</kbd> durante uma partida movimentada. | Todo o áudio é silenciado imediatamente; ao pressionar novamente, os sons voltam com volume normal. |
