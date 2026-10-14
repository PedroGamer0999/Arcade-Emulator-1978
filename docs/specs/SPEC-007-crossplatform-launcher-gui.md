# SPEC-007: Launcher Gráfico Multiplataforma (Windows & Linux)

- **Status:** Aprovado para Implementação (Sprint 5)
- **Autor:** Game Developer / Frontend Specialist
- **Arquivos Afetados:** `src/launcher.h`, `src/launcher.c`, `src/nuklear_impl.h`, [`src/video.h`](file:///D:/projetos/008-space-invaders-emulator/src/video.h), [`src/video.c`](file:///D:/projetos/008-space-invaders-emulator/src/video.c), [`src/main.c`](file:///D:/projetos/008-space-invaders-emulator/src/main.c)
- **Dependências:** [`SPEC-006`](SPEC-006-multi-game-hardware-support.md)

---

## 1. Contexto e Motivação
Um dos requisitos centrais do usuário é:
> *"quem sabe criar uma janela que rode em Windows e Linux para podemos escolher o rom."*

Atualmente, se o executável for aberto pelo explorador de arquivos, ele tenta carregar compulsoriamente a ROM padrão `rom/invaders.*`. Se a ROM não existir ou se o usuário quiser jogar outro jogo suportado, ele precisaria usar linha de comando.
Para entregar um produto final polido para usuários leigos e entusiastas de retrogaming, criamos uma **janela de Launcher gráfica multiplataforma**, com visual retro-moderno de arcade, que roda de forma idêntica e nativa tanto no **Windows** quanto no **Linux** sem exigir a instalação de pacotes pesados como GTK, Qt ou .NET.

---

## 2. Requisitos

### Requisitos Funcionais (RF)
- **RF-01**: Se o emulador for iniciado sem parâmetros de ROM na linha de comando, ele deve abrir a **Janela do Launcher**.
- **RF-02**: A interface gráfica deve ser implementada em **C puro** utilizando a biblioteca **Nuklear** (single-header GUI) com renderização sobre o `SDL_Renderer` já presente no projeto.
- **RF-03**: Interface dividida em painéis:
  - **Painel Esquerdo (Lista de Jogos):** Lista interativa com todos os jogos suportados pelo emulador, exibindo título, ano e uma tag de status (`[DISPONÍVEL]` em verde ou `[NÃO INSTALADO]` em cinza/vermelho).
  - **Painel Direito (Detalhes do Jogo):** Imagem de capa/preview (screenshot do arcade), ficha técnica (fabricante, ano, chips emulados), checklist de arquivos da ROM e botão **"INICIAR JOGO"**.
- **RF-04**: Opção de alternar opções visuais (Color Overlay, Scanlines, Escala de Janela) e volume diretamente na janela do Launcher.
- **RF-05**: Transição suave: ao clicar em "INICIAR JOGO", a janela do Launcher transiciona para a tela de jogo do arcade selecionado.
- **RF-06**: Ao pressionar a tecla <kbd>ESC</kbd> durante a partida, o emulador pausa o jogo e abre um menu overlay com a opção **"Voltar ao Launcher"** para escolher outro jogo sem fechar o programa.

### Requisitos Não-Funcionais (RNF)
- **RNF-01**: 100% de paridade visual e funcional entre Windows (Windows 10/11) e Linux (X11 e Wayland via SDL2).
- **RNF-02**: Navegação completa tanto via Mouse quanto via Teclado (Setas para cima/baixo e <kbd>Enter</kbd> para selecionar).

---

## 3. Especificação Técnica e Arquitetura

### 3.1 Arquitetura do Launcher (`src/launcher.h`)
```c
#ifndef LAUNCHER_H
#define LAUNCHER_H

#include <SDL2/SDL.h>
#include "game_driver.h"

typedef struct {
    int selecionado_index;
    int total_jogos;
    const GameDriver **jogos;
    int *status_instalado; // 1 = ROM presente e íntegra, 0 = ausente
    SDL_Texture *texturas_preview[16];
    int requisitou_iniciar;
    int requisitou_sair;
} LauncherState;

int init_launcher(LauncherState *state, SDL_Renderer *renderer);
void processar_eventos_launcher(LauncherState *state, SDL_Event *event);
void renderizar_launcher(LauncherState *state, SDL_Renderer *renderer);
void fechar_launcher(LauncherState *state);

#endif
```

### 3.2 Mockup de Layout da Janela (640 x 480)
```
+-------------------------------------------------------------------------+
| [★] RETRO ARCADE 8080 EMULATOR - LAUNCHER                [_][O][X]      |
+------------------------------------+------------------------------------+
| JOGOS SUPORTADOS                   | DETALHES DO JOGO                   |
|                                    |                                    |
| > [x] Space Invaders (1978)        |  +------------------------------+  |
|   [x] Space Invaders Part II (1979)|  |                              |  |
|   [!] Lunar Rescue (1979)          |  |     [ Screenshot / Capa ]    |  |
|   [!] Balloon Bomber (1980)        |  |                              |  |
|   [x] CPUDIAG Test Suite           |  +------------------------------+  |
|                                    |                                    |
|                                    | Título: Space Invaders (Midway)    |
|                                    | Ano: 1978 | CPU: Intel 8080 @ 2MHz |
|                                    | Status: 4/4 ROMs Instaladas (OK)   |
|                                    |                                    |
|                                    |   [ INICIAR JOGO (ENTER) ]         |
+------------------------------------+------------------------------------+
| [ Atualizar ROMs ]   [ Configurações ]                 v1.2.0-release   |
+-------------------------------------------------------------------------+
```

### 3.3 Transição de Estado em `src/main.c`
```c
typedef enum {
    APP_STATE_LAUNCHER,
    APP_STATE_EMULATION,
    APP_STATE_QUIT
} AppState;

AppState estado = (argc > 1) ? APP_STATE_EMULATION : APP_STATE_LAUNCHER;

while (estado != APP_STATE_QUIT) {
    if (estado == APP_STATE_LAUNCHER) {
        // Executa loop do Launcher
        executar_launcher(&launcher, &driver_selecionado);
        if (launcher.requisitou_iniciar) {
            estado = APP_STATE_EMULATION;
        } else {
            estado = APP_STATE_QUIT;
        }
    } else if (estado == APP_STATE_EMULATION) {
        // Executa loop do Emulador com o driver selecionado
        int voltar_ao_menu = executar_emulador(driver_selecionado);
        if (voltar_ao_menu) {
            estado = APP_STATE_LAUNCHER;
        } else {
            estado = APP_STATE_QUIT;
        }
    }
}
```

---

## 4. Critérios de Aceite (Definition of Done)
- [x] O executável abre diretamente o Launcher quando acionado sem parâmetros no Windows Explorer ou terminal Linux.
- [x] A lista de jogos exibe os títulos e indica claramente se a ROM está presente ou faltando.
- [x] Pressionar <kbd>Enter</kbd> ou clicar em "INICIAR JOGO" inicia a emulação imediatamente.
- [x] Pressionar <kbd>ESC</kbd> durante o jogo permite retornar ao Launcher e alternar para outro título sem reiniciar o executável.

---

## 5. Roteiro de Testes de Q/A

| Teste | Descrição | Ação | Resultado Esperado |
| :--- | :--- | :--- | :--- |
| **TC-007-A** | Abertura do Launcher no Windows | Dar duplo clique em `space_invaders.exe`. | Abre a janela do Launcher com título retrô, botões funcionais e cursor do mouse ativo. |
| **TC-007-B** | Abertura do Launcher no Linux | Executar `./space_invaders` no terminal do Ubuntu/Fedora. | Abre a janela perfeitamente com fontes nítidas e mesmo visual do Windows. |
| **TC-007-C** | Seleção e Início de Jogo | Navegar com as setas do teclado até "Space Invaders" e apertar <kbd>Enter</kbd>. | O Launcher fecha sua UI e entra na partida com som e vídeo ativos. |
| **TC-007-D** | Troca de Jogo com <kbd>ESC</kbd> | Durante a partida, pressionar <kbd>ESC</kbd> e escolher "Menu Principal". | O jogo atual descarrega da memória e o Launcher reaparece pronto para outra escolha. |
