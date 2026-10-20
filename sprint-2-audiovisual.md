# Task Plan: Sprint 2 - Audiovisual Completo: Cores e Efeitos Sonoros (v1.1)

**Task Slug:** `sprint-2-audiovisual`  
**Status:** Completed  
**Spec References:**  
- [`docs/specs/SPEC-002-arcade-color-overlay-crt.md`](docs/specs/SPEC-002-arcade-color-overlay-crt.md)  
- [`docs/specs/SPEC-003-arcade-audio-mixer.md`](docs/specs/SPEC-003-arcade-audio-mixer.md)  
**Branch:** `feature/sprint-2-audiovisual`  

---

## 🎯 Objetivo
Entregar a experiência sensorial autêntica do arcade clássico de 1978, implementando o Color Overlay do gabinete original (vermelho, branco e verde) com filtro CRT scanlines configurável, e o subsistema de áudio SDL2 com mixer de baixa latência e detecção de borda nas portas OUT `0x03` e `0x05`.

---

## 📋 Lista de Tarefas

- [x] **Task 1: Color Overlay Autêntico e Filtro CRT Scanlines**
  - Atualizado [`src/video.h`](src/video.h): enum `ColorMode`, campos `modo_cor` e `habilitar_scanlines`, protótipos de alternância.
  - Atualizado [`src/video.c`](src/video.c): lógica de cor por coordenada vertical `y` (vermelho arcade `0xE62626FF` para UFO/placar, branco puro `0xFFFFFFFF` para invasores, verde arcade `0x26E626FF` para barricadas/canhão/solo), alternância entre Overlay, Monocromático e Verde Fósforo; atenuação de 25% para scanlines em linhas ímpares.
  - Atualizado [`src/main.c`](src/main.c): mapeamento das teclas <kbd>F2</kbd> (alternar modo de cor) e <kbd>F3</kbd> (alternar scanlines).
  - *INPUT:* `src/video.h`, `src/video.c`, `src/main.c` → *OUTPUT:* Renderização colorida autêntica e scanlines comutáveis → *VERIFY:* Compilação limpa e renderização estável a 60 FPS.

- [x] **Task 2: Gerador dos 9 Samples de Áudio WAV Autênticos**
  - Criado [`tools/generate_sounds.c`](tools/generate_sounds.c) que sintetizou os 9 efeitos com fidelidade física ao hardware analógico/chip SN76477:
    - `sounds/ufo.wav` (som contínuo modulado a 6.0 Hz)
    - `sounds/shot.wav` (sweep laser descendente de 2.8 kHz a 200 Hz)
    - `sounds/player_die.wav` (explosão com tremolo e ruído de 1.25s)
    - `sounds/invader_die.wav` (crackle e chirp rápido)
    - `sounds/fleet1.wav` (marcha 174.6 Hz - F3)
    - `sounds/fleet2.wav` (marcha 164.8 Hz - E3)
    - `sounds/fleet3.wav` (marcha 155.6 Hz - Eb3)
    - `sounds/fleet4.wav` (marcha 146.8 Hz - D3)
    - `sounds/ufo_hit.wav` (explosão do disco voador)
  - *INPUT:* Especificações acústicas do hardware original → *OUTPUT:* Pasta `sounds/` com 9 arquivos WAV válidos 44.1 kHz 16-bit → *VERIFY:* Validação de headers RIFF/WAVE e integridade dos samples.

- [x] **Task 3: Subsistema de Áudio e Mixer Multicanal SDL2**
  - Criados [`src/audio.h`](src/audio.h) e [`src/audio.c`](src/audio.c):
    - Inicialização via `SDL_OpenAudioDevice` (44.100 Hz, 16-bit PCM estéreo, buffer 512 samples ~11.6ms de latência).
    - Suporte a mixer multicanal com até 8 canais simultâneos com clamp estático sem distorção.
    - Suporte a sample em loop para o UFO (`SND_UFO`), parando imediatamente ao zerar o bit 0 da porta 3.
    - Suporte a mute geral acionado via tecla <kbd>M</kbd>.
  - Atualizados [`src/io.h`](src/io.h) e [`src/io.c`](src/io.c):
    - Rastreamento dos valores anteriores `prev_port3` e `prev_port5`.
    - Detecção de borda de subida (0 -> 1) nas portas 3 e 5 e despacho imediato para o áudio.
  - Atualizado [`src/main.c`](src/main.c):
    - Chamadas `init_audio("sounds")` na inicialização e `close_audio()` no encerramento.
    - Captura da tecla <kbd>M</kbd> no teclado para acionar `toggle_mute()`.
  - *INPUT:* `src/audio.h`, `src/audio.c`, `src/io.h`, `src/io.c`, `src/main.c` → *OUTPUT:* Áudio em tempo real integrado às portas de I/O → *VERIFY:* Compilação limpa e reprodução sonora responsiva.

- [x] **Task 4: Atualização dos Scripts de Build**
  - Atualizados `makefile.ps1`, `makefile.bat`, `setup.bat`, `setup.sh` e `Makefile` para incluir `src/audio.c`.
  - *INPUT:* Scripts de build → *OUTPUT:* Scripts atualizados compilando todos os módulos → *VERIFY:* Execução com código de saída 0.

- [x] **Task 5: Testes Automatizados e Roteiro de Q/A**
  - Criado [`tests/test_audiovisual.c`](tests/test_audiovisual.c) cobrindo:
    - **QA-S2-01:** Mapeamento correto de cores por faixa `y` (vermelho, branco, verde) e modos alternativos P&B e Verde Fósforo.
    - **QA-S2-02:** Despacho instantâneo de tiro e som com latência buffer ~11.6ms (< 30ms).
    - **QA-S2-03:** Disparo e interrupção limpa do loop contínuo do UFO (bit 0 da porta 3).
    - **QA-S2-04:** Alternância de Scanlines CRT com atenuação de 25% nas linhas ímpares.
    - Integridade dos 9 arquivos WAV e teste de integração real com ROM executando 60 quadros.
  - *Resultado:* **57/57 testes aprovados (100.0%)**.
  - Regressão da Sprint 1: **31/31 testes aprovados (100.0%)**.

- [x] **Task 6: Atualização da Documentação e Definição de Concluído (DoD)**
  - Atualizados [`docs/sprints.md`](docs/sprints.md), [`docs/backlog.md`](docs/backlog.md) e [`README.md`](README.md).

---

## ✅ PHASE X COMPLETE
- Build: ✅ Sucesso sem warnings (`space_invaders.exe`)
- Lint & Types: ✅ `-Wall -Wextra` aprovado com 0 warnings
- Testes Automatizados Sprint 2: ✅ 57/57 testes aprovados (100.0%)
- Testes Automatizados Regressão Sprint 1: ✅ 31/31 testes aprovados (100.0%)
- Assets de Áudio: ✅ 9/9 arquivos WAV sintetizados e validados
