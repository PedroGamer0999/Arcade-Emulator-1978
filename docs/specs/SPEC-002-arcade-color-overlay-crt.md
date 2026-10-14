# SPEC-002: Color Overlay Autêntico do Arcade e Filtro CRT

- **Status:** Aprovado para Implementação (Sprint 2)
- **Autor:** Game Developer / Graphics Specialist
- **Arquivos Afetados:** [`src/video.h`](file:///D:/projetos/008-space-invaders-emulator/src/video.h), [`src/video.c`](file:///D:/projetos/008-space-invaders-emulator/src/video.c)
- **Dependências:** [`SPEC-001`](SPEC-001-cpu-timing-interrupts.md)

---

## 1. Contexto e Motivação
O gabinete original de Space Invaders (1978) não possuía um monitor colorido. Ele utilizava um monitor CRT monocromático preto e branco de alta persistência montado na vertical, sobre o qual a Taito / Midway colava **tiras transparentes de celofane colorido (Color Overlay)** para simular cores:
1. **Topo (Vermelho):** Onde aparece a nave-mãe misteriosa (UFO / Saucer).
2. **Centro (Branco):** Onde marcha a frota de 55 invasores.
3. **Base (Verde):** Onde ficam as 4 barricadas de defesa, o canhão laser do jogador e a linha de solo.

Atualmente, o emulador renderiza tudo em branco puro (`0xFFFFFFFF`). Para entregar um produto final autêntico, é indispensável reproduzir fielmente o overlay de cores original, além de oferecer um filtro opcional de linhas de varredura (CRT Scanlines) e preservação do aspecto 3:4 original.

---

## 2. Requisitos

### Requisitos Funcionais (RF)
- **RF-01**: O renderizador de vídeo deve aplicar cores aos pixels ativos de acordo com a posição vertical `y` (na tela já rotacionada para visualização correta de 224 x 256):
  - `y` de `0` a `31`: **Vermelho Arcade** (`0xCC2222FF` ou `#CC2222`) - Área da nave UFO e placares.
  - `y` de `32` a `183`: **Branco Puro** (`0xFFFFFFFF`) - Área dos invasores.
  - `y` de `184` a `239`: **Verde Arcade** (`0x22CC22FF` ou `#22CC22`) - Barricadas e Canhão.
  - `y` de `240` a `255`: **Branco ou Verde** conforme o gabinete (Verde para a linha de solo e naves reservas).
- **RF-02**: Permitir alternar os modos de exibição em tempo de execução via tecla <kbd>F2</kbd>:
  - Modo 1: **Arcade Color Overlay** (Padrão)
  - Modo 2: **Monocromático Original (P&B)**
  - Modo 3: **Verde Fósforo CRT (Estilo Game Boy / Terminal Antigo)**
- **RF-03**: Oferecer efeito de scanlines CRT ativável via tecla <kbd>F3</kbd> (escurecimento suave de linhas horizontais alternadas).
- **RF-04**: Suporte a redimensionamento de janela (2x = 448x512, 3x = 672x768, 4x = 896x1024 ou Fullscreen) preservando o aspect ratio exato de 224:256 (aproximadamente 3:4 vertical) sem distorcer os sprites.

---

## 3. Especificação Técnica e Arquitetura

### 3.1 Alterações em `src/video.h`
```c
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
    int escala;
} Display;

void alternar_modo_cor(Display *display);
void alternar_scanlines(Display *display);
```

### 3.2 Lógica de Mapeamento de Cor em `src/video.c`
```c
static inline uint32_t obter_cor_pixel(int y, ColorMode modo) {
    if (modo == COLOR_MODE_MONOCHROME) {
        return 0xFFFFFFFF; // Branco
    }
    if (modo == COLOR_MODE_GREEN_PHOSPHOR) {
        return 0x33FF33FF; // Verde fósforo P1
    }

    // COLOR_MODE_OVERLAY (Gabinete Original 1978)
    if (y < 32) {
        return 0xE62626FF; // Vermelho intenso (UFO)
    } else if (y >= 184 && y < 240) {
        return 0x26E626FF; // Verde brilhante (Barricadas e canhão)
    } else if (y >= 240) {
        return 0x26E626FF; // Verde (Linha de base e naves extras)
    }

    return 0xFFFFFFFF; // Branco (Invasores)
}
```

### 3.3 Renderização com Scanlines
Ao desenhar na textura ou pós-processar:
```c
if (display->habilitar_scanlines && (y % 2 == 1)) {
    // Atenua a luminosidade do pixel em 25% para criar a linha de varredura
    uint8_t r = ((cor >> 24) & 0xFF) * 3 / 4;
    uint8_t g = ((cor >> 16) & 0xFF) * 3 / 4;
    uint8_t b = ((cor >> 8) & 0xFF) * 3 / 4;
    cor = (r << 24) | (g << 16) | (b << 8) | 0xFF;
}
```

---

## 4. Critérios de Aceite (Definition of Done)
- [x] O jogo exibe as três faixas clássicas de cores (vermelho, branco, verde) no modo padrão.
- [x] Tecla <kbd>F2</kbd> alterna ciclicamente entre Color Overlay, Monocromático e Verde Fósforo sem engasgos.
- [x] Tecla <kbd>F3</kbd> liga e desliga o efeito de scanlines.
- [x] A taxa de quadros permanece fixa em 60 FPS independentemente do modo visual ativado.

---

## 5. Roteiro de Testes de Q/A

| Teste | Descrição | Ação | Resultado Esperado |
| :--- | :--- | :--- | :--- |
| **TC-002-A** | Verificação de Faixas de Cores | Iniciar o jogo e observar canhão, barricadas e topo. | Barricadas e canhão devem ser verdes; aliens centrais brancos; topo vermelho quando o UFO surgir. |
| **TC-002-B** | Alternância de Paletas (<kbd>F2</kbd>) | Pressionar <kbd>F2</kbd> repetidamente durante o jogo. | A paleta muda suavemente para P&B, depois verde monocromático e volta ao Color Overlay. |
| **TC-002-C** | Scanlines (<kbd>F3</kbd>) | Ativar scanlines com <kbd>F3</kbd> em escala 3x ou 4x. | Linhas pretas/atenuadas suaves horizontais aparecem criando o aspecto de tela CRT sem borrar o texto. |
