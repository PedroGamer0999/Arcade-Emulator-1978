# SPEC-009: Gamepad, DIP Switches e Save States

- **Status:** Aprovado para Implementação (Sprint 6)
- **Autor:** Game Developer / Emulation Features Specialist
- **Arquivos Afetados:** [`src/io.h`](file:///D:/projetos/008-space-invaders-emulator/src/io.h), [`src/io.c`](file:///D:/projetos/008-space-invaders-emulator/src/io.c), `src/savestate.h`, `src/savestate.c`, [`src/main.c`](file:///D:/projetos/008-space-invaders-emulator/src/main.c)
- **Dependências:** [`SPEC-007`](SPEC-007-crossplatform-launcher-gui.md)

---

## 1. Contexto e Motivação
Para que o emulador atinja a qualidade esperada de um produto comercial ou comunitário consagrado, ele precisa oferecer comodidades modernas de jogabilidade que vão além do teclado básico:
1. **Suporte a Gamepads/Controles USB e Bluetooth:** O jogador deve poder plugar qualquer controle de Xbox, PlayStation ou joystick arcade genérico e jogar confortavelmente no sofá.
2. **Configuração de DIP Switches:** As máquinas arcade reais possuíam pequenas chaves físicas na placa-mãe (DIP switches) que o operador do fliperama configurava para definir número de vidas (3, 4, 5 ou 6), pontuação para vida extra e exibição de créditos. Atualmente, esses valores estão fixos em [`src/io.c`](file:///D:/projetos/008-space-invaders-emulator/src/io.c#L7).
3. **Save States e Load States:** Poder salvar o estado exato da partida no meio da ação e restaurar imediatamente ao pressionar uma tecla (<kbd>F5</kbd> para Salvar e <kbd>F7</kbd> para Carregar).

---

## 2. Requisitos

### Requisitos Funcionais (RF)
- **RF-01**: Suporte automático a joysticks e gamepads via subsistema `SDL_GameController`:
  - Reconhecimento dinâmico de conexão/desconexão (*hotplug*).
  - D-Pad ou Analógico Esquerdo: Mover canhão para Esquerda / Direita.
  - Botão <kbd>A</kbd> / <kbd>X</kbd>: Disparar (Tiro).
  - Botão <kbd>Select</kbd> / <kbd>Share</kbd>: Inserir Moeda (Coin).
  - Botão <kbd>Start</kbd>: 1 Player Start.
- **RF-02**: Mapeamento customizável de teclas do teclado e botões de controle armazenado em arquivo de configuração `config.ini`.
- **RF-03**: Emulação de DIP Switches através dos bits da **Porta 0x02**:
  - Bits 0 e 1: Número de naves do jogador (`00` = 3 naves, `01` = 4 naves, `10` = 5 naves, `11` = 6 naves).
  - Bit 3: Bônus de vida extra (`0` = 1500 pontos, `1` = 1000 pontos).
  - Bit 7: Exibição de moeda na tela de demonstração (`0` = Ligado, `1` = Desligado).
- **RF-04**: Sistema de Save State serializando:
  - Registradores e memória de 64 KB da CPU (`State8080`).
  - Estado do Shift Register e portas de I/O (`InputOutput`).
  - Ciclos de clock acumulados e próxima interrupção pendente.
- **RF-05**: Atalhos rápidos globais:
  - <kbd>F5</kbd>: Quick Save State (salva em `saves/{jogo}.state`).
  - <kbd>F7</kbd>: Quick Load State (restaura o jogo instantaneamente).
  - Mensagem em texto na tela (OSD toast): "Estado Salvo no Slot 1" ou "Estado Carregado".

---

## 3. Especificação Técnica e Arquitetura

### 3.1 Serialização de Estado (`src/savestate.h`)
```c
#ifndef SAVESTATE_H
#define SAVESTATE_H

#include "cpu8080.h"
#include "io.h"

#define SAVE_STATE_MAGIC 0x30385653 // "SV80" em Little Endian
#define SAVE_STATE_VERSION 1

typedef struct {
    uint32_t magic;
    uint32_t versao;
    char id_jogo[16];
    State8080 cpu;
    InputOutput io;
    uint8_t proxima_interrupcao;
    int ciclos_executados;
} SaveStateHeader;

int salvar_estado(const char *caminho, const char *id_jogo, const State8080 *cpu, const InputOutput *io, uint8_t proxima_int, int ciclos);
int carregar_estado(const char *caminho, const char *id_jogo, State8080 *cpu, InputOutput *io, uint8_t *proxima_int, int *ciclos);

#endif
```

### 3.2 Integração com Gamepad em `src/main.c`
```c
SDL_GameController *controlador = NULL;

if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_GAMECONTROLLER) < 0) { ... }

// No loop de eventos:
case SDL_CONTROLLERDEVICEADDED:
    if (!controlador) {
        controlador = SDL_GameControllerOpen(event.cdevice.which);
    }
    break;

case SDL_CONTROLLERBUTTONDOWN:
case SDL_CONTROLLERBUTTONUP:
    tratar_gamepad(&io, event.cbutton.button, event.cbutton.state);
    break;
```

---

## 4. Critérios de Aceite (Definition of Done)
- [x] O controle USB/Bluetooth é detectado e responde ao D-pad e botões de disparo sem atraso.
- [x] Alterar o número de vidas nos DIP switches (ex: para 5 vidas) reflete diretamente no início da partida.
- [x] Salvar com <kbd>F5</kbd> e restaurar com <kbd>F7</kbd> restaura fielmente os invasores e o canhão exatamente na mesma posição e pontuação.
- [x] Arquivo de save state protegido com verificação de integridade mágica (`SV80`).

---

## 5. Roteiro de Testes de Q/A

| Teste | Descrição | Ação | Resultado Esperado |
| :--- | :--- | :--- | :--- |
| **TC-009-A** | Hotplug de Controle | Iniciar o emulador, conectar um controle de Xbox/PlayStation na porta USB e pressionar os botões. | O controle assume os comandos imediatamente sem reiniciar o jogo. |
| **TC-009-B** | Teste de DIP Switches (Vidas) | Configurar DIP switch para 6 naves no menu/launcher e iniciar jogo. | A base da tela exibe 5 naves de reserva ao iniciar a primeira vida. |
| **TC-009-C** | Ciclo de Save State / Load State | Chegar à onda 3, salvar com <kbd>F5</kbd>, perder todas as vidas até o Game Over e pressionar <kbd>F7</kbd>. | O jogo volta instantaneamente para a onda 3 com a mesma quantidade de aliens e vidas salvas. |
