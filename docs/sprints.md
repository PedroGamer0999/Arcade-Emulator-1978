# Sprints & Release Plan - Space Invaders Arcade Emulator

> Plano de iterações organizado por valor de entrega.
> Cada sprint possui um objetivo de negócio claro (GOAL), entregáveis rastreáveis e um roteiro de testes para Quality Assurance (Q/A).

---

## 🗺️ Mapa de Releases

- **Versão v1.x: "Arcade Authenticity" (Fidelidade do Core)**
  - `Sprint 1`: Estabilidade e Timing Preciso de CPU (v1.0-alpha)
  - `Sprint 2`: Audiovisual Completo: Cores e Efeitos Sonoros (v1.1)
  - `Sprint 3`: Portabilidade e Build System CMake (v1.2)
- **Versão v2.x: "Multi-Arcade Platform" (Produto Expandido)**
  - `Sprint 4`: Gerenciador de ROMs e Múltiplos Jogos 8080 (v2.0)
  - `Sprint 5`: Launcher Gráfico Multiplataforma (v2.1)
  - `Sprint 6`: Downloader Integrado, Gamepads e Save States (v2.2 - Final)

---

## 🏃 Sprint 1: Estabilidade e Timing Preciso de CPU (v1.0-alpha)

### 🎯 GOAL da Sprint
> **"Substituir o loop arbitrário por uma emulação precisa baseada no clock de 2.0 MHz do Intel 8080 e controle formal de interrupções, assegurando taxa estável de 60 FPS e compatibilidade de CPU."**

### 📦 Entregáveis
- [ ] Implementação da tabela de ciclos do Intel 8080 (`cycles_8080[256]`).
- [ ] Retorno de ciclos gastos por cada instrução em `exec_instrucao()`.
- [ ] Flags de interrupção `inte` (Interrupt Enable) e `halt` em `State8080`.
- [ ] Tratamento das instruções `EI` (0xFB) e `DI` (0xF3) e respeito ao `inte` ao despachar `RST 1` e `RST 2`.
- [ ] Loop principal de 60 FPS com sincronização por clock de alta resolução (`SDL_GetPerformanceCounter`).

### 🧪 Roteiro de Testes Q/A (Sprint 1)
1. **QA-S1-01 (Velocidade e FPS):**
   - *Procedimento:* Iniciar o emulador com contador de frames habilitado.
   - *Critério de Aceite:* O jogo deve rodar exatamente a 60 FPS ± 0.5 FPS, sem oscilações bruscas ou aceleração excessiva em CPUs modernas.
2. **QA-S1-02 (Marcha dos Invasores):**
   - *Procedimento:* Observar o movimento dos aliens no início da partida e quando restam poucos aliens.
   - *Critério de Aceite:* Os aliens devem se mover no ritmo histórico clássico; a aceleração natural conforme os aliens são destruídos deve ocorrer sem engasgos de frame.
3. **QA-S1-03 (Teste de Interrupção com DI/EI):**
   - *Procedimento:* Verificar nos logs de debug se o emulador rejeita interrupções de varredura durante seções de código protegidas por `DI`.
   - *Critério de Aceite:* Não deve ocorrer corrupção de pilha (`SP`) nem jumps indevidos.

---

## 🏃 Sprint 2: Audiovisual Completo: Cores e Efeitos Sonoros (v1.1)

### 🎯 GOAL da Sprint
> **"Entregar a experiência sensorial autêntica do arcade clássico de 1978, implementando o Color Overlay do gabinete original e os efeitos sonoros gerados pelas portas de áudio 0x03 e 0x05."**

### 📦 Entregáveis
- [ ] Colorização por faixas de coordenadas Y (Vermelho no topo para UFO, Branco no meio para aliens, Verde na base para canhão e barricadas).
- [ ] Efeito CRT opcional configurável (Scanlines sutis e ajuste de aspecto 4:3).
- [ ] Subsistema de áudio SDL2 (`SDL_OpenAudioDevice` com buffer de baixa latência).
- [ ] Banco de samples WAV dos 9 sons do arcade (UFO, tiro, morte do jogador, explosão de alien, marcha 1-4, hit no UFO).
- [ ] Monitoramento de transição de bits (0 -> 1) nas portas OUT `0x03` e `0x05`.

### 🧪 Roteiro de Testes Q/A (Sprint 2)
1. **QA-S2-01 (Fidelidade de Cores):**
   - *Procedimento:* Iniciar partida e inspecionar visualmente as 3 áreas da tela.
   - *Critério de Aceite:* A nave-mãe (UFO) no topo deve ser vermelha; os invasores e tiros brancos; as 4 barricadas, o canhão e a linha do solo verdes.
2. **QA-S2-02 (Disparo e Latência de Áudio):**
   - *Procedimento:* Pressionar <kbd>Espaço</kbd> para disparar e ouvir o retorno sonoro.
   - *Critério de Aceite:* O som do tiro deve iniciar imediatamente (< 30ms de latência percebida), sem cliques, estalos ou atrasos perceptíveis.
3. **QA-S2-03 (Loop Contínuo do UFO):**
   - *Procedimento:* Aguardar o aparecimento da nave-mãe no topo da tela.
   - *Critério de Aceite:* O som característico do UFO deve tocar em loop enquanto o bit da porta 3 estiver em 1, e cessar imediatamente quando o UFO sair ou for destruído.
4. **QA-S2-04 (Alternância de Scanlines):**
   - *Procedimento:* Pressionar tecla de atalho (ex: <kbd>F2</kbd>) para ligar/desligar scanlines.
   - *Critério de Aceite:* A imagem deve alternar entre pixels nítidos e efeito CRT retrô sem queda de FPS.

---

## 🏃 Sprint 3: Portabilidade e Build System CMake (v1.2)

### 🎯 GOAL da Sprint
> **"Eliminar scripts de compilação frágeis e unificar a compilação do projeto com CMake universal, garantindo compilação nativa de 1 comando no Windows e no Linux sem dependências rígidas de caminhos locais."**

### 📦 Entregáveis
- [ ] Arquivo `CMakeLists.txt` na raiz com suporte a alvos de debug e release.
- [ ] Detecção automática da biblioteca SDL2 via `find_package(SDL2 REQUIRED)`.
- [ ] Target multiplataforma para Windows (MinGW, Clang, MSVC) e Linux (GCC, Clang).
- [ ] Scripts unificados de atalho (`build.sh` e `build.bat` chamando o CMake).
- [ ] Configuração de warnings rigorosos (`-Wall -Wextra -Wpedantic` ou `/W4`).

### 🧪 Roteiro de Testes Q/A (Sprint 3)
1. **QA-S3-01 (Build Limpo no Linux):**
   - *Procedimento:* Em ambiente Linux/WSL, executar:
     ```bash
     mkdir build && cd build && cmake .. && make -j
     ```
   - *Critério de Aceite:* O binário `space_invaders` deve ser gerado sem erros ou warnings.
2. **QA-S3-02 (Build Limpo no Windows):**
   - *Procedimento:* No PowerShell do Windows, executar `cmake -B build` e compilar.
   - *Critério de Aceite:* O executável `space_invaders.exe` deve ser compilado e rodar perfeitamente vinculando a `SDL2.dll`.

---

## 🏃 Sprint 4: Gerenciador de ROMs e Múltiplos Jogos 8080 (v2.0)

### 🎯 GOAL da Sprint
> **"Criar um gerenciador robusto com extração de arquivos ZIP e validação de integridade por hashes CRC32/SHA-1, habilitando a execução de múltiplos jogos clássicos da placa 8080."**

### 📦 Entregáveis
- [ ] Integração da biblioteca de descompactação `miniz` em C puro (sem dependência externa).
- [ ] Módulo de cálculo de CRC32 e SHA-1 em C (`src/rom_manager.c`).
- [ ] Catálogo de jogos em C com suporte a:
  - Space Invaders (1978 - Midway / Taito)
  - Space Invaders Part II / Deluxe (1979)
  - Lunar Rescue (1979)
  - Balloon Bomber (1980)
  - CPUDIAG (ROM de diagnóstico da CPU 8080)
- [ ] Carga direta de ROMs via linha de comando: `./space_invaders <caminho_ou_zip>`.

### 🧪 Roteiro de Testes Q/A (Sprint 4)
1. **QA-S4-01 (Validação de ROM Corrompida):**
   - *Procedimento:* Fornecer um arquivo com byte corrompido para o emulador.
   - *Critério de Aceite:* O emulador deve rejeitar a carga, exibir mensagem clara indicando o hash esperado vs obtido e não abortar com crash.
2. **QA-S4-02 (Carga de Arquivo .ZIP do MAME):**
   - *Procedimento:* Colocar o arquivo `invaders.zip` original do MAME na pasta `roms/`.
   - *Critério de Aceite:* O emulador deve abrir o ZIP em memória, extrair os 4 arquivos (`invaders.h, g, f, e`) e inicializar o jogo com sucesso.
3. **QA-S4-03 (Teste de Conformidade CPUDIAG):**
   - *Procedimento:* Carregar a ROM `cpudiag.bin`.
   - *Critério de Aceite:* A ROM deve executar os testes de todas as instruções da CPU 8080 e reportar "CPU TESTS OK" no console/VRAM.

---

## 🏃 Sprint 5: Launcher Gráfico Multiplataforma (v2.1)

### 🎯 GOAL da Sprint
> **"Criar uma interface gráfica limpa e responsiva (Windows e Linux) em SDL2 para seleção de jogos, exibição de capas/informações e acesso a opções antes de iniciar a partida."**

### 📦 Entregáveis
- [ ] Janela de Launcher gráfica integrada (baseada em Nuklear / Dear ImGui sobre SDL2).
- [ ] Scanner automático da pasta `roms/` identificando jogos disponíveis vs não instalados.
- [ ] Painel de visualização com título do jogo, ano, fabricante, status da ROM e captura de tela (preview).
- [ ] Botão "Iniciar Jogo" para fazer a transição fluida do Launcher para a emulação.
- [ ] Menu In-Game acionado por <kbd>ESC</kbd> com opções: Retomar, Reiniciar, Selecionar outro jogo, Sair.

### 🧪 Roteiro de Testes Q/A (Sprint 5)
1. **QA-S5-01 (Comportamento em Windows e Linux):**
   - *Procedimento:* Abrir o executável sem parâmetros em ambos os sistemas operacionais.
   - *Critério de Aceite:* A janela do Launcher deve abrir centralizada, responsiva ao mouse e teclado, com renderização consistente em ambas as plataformas.
2. **QA-S5-02 (Detecção Dinâmica de ROMs):**
   - *Procedimento:* Adicionar uma nova ROM (`lrescu.zip`) na pasta `roms/` e clicar em "Atualizar Lista".
   - *Critério de Aceite:* O jogo Lunar Rescue deve mudar o status de "Não Encontrado" para "Pronto para Jogar".
3. **QA-S5-03 (Alternância de Jogo sem Fechar o App):**
   - *Procedimento:* Durante o Space Invaders, pressionar <kbd>ESC</kbd>, escolher "Selecionar outro jogo" e iniciar o Space Invaders Part II.
   - *Critério de Aceite:* O estado da CPU e memória deve ser resetado limpo, sem vazamento de memória ou textura residual, iniciando o novo jogo instantaneamente.

---

## 🏃 Sprint 6: Downloader Integrado, Gamepads e Save States (v2.2 - Final)

### 🎯 GOAL da Sprint
> **"Completar o produto final com comodidade total: download/instalação de ROMs permitidas com 1 clique, suporte universal a controles USB/Bluetooth, configuração de DIP switches e save states."**

### 📦 Entregáveis
- [ ] Módulo de download HTTP em C com suporte a download em background e barra de progresso.
- [ ] Botão "Instalar / Baixar" no Launcher para jogos disponíveis em repositórios autorizados (Internet Archive / Homebrew).
- [ ] Suporte a Gamepads via `SDL_GameController` (D-Pad, botões de Tiro e Moeda).
- [ ] Janela/Menu de configuração de DIP Switches (Vidas: 3, 4, 5, 6; Pontuação de Vida Extra; Moedas por partida).
- [ ] Sistema de Save State e Load State com teclas rápidas (<kbd>F5</kbd> Salvar / <kbd>F7</kbd> Carregar).

### 🧪 Roteiro de Testes Q/A (Sprint 6)
1. **QA-S6-01 (Download e Instalação com 1 Clique):**
   - *Procedimento:* Clicar em "Baixar" em um jogo não instalado.
   - *Critério de Aceite:* O arquivo deve ser baixado para a pasta `roms/`, validado via hash CRC32 e o botão deve se transformar em "Jogar Agora".
2. **QA-S6-02 (Detecção e Mapeamento de Gamepad):**
   - *Procedimento:* Conectar controle USB (ex: Xbox, PlayStation ou genérico).
   - *Critério de Aceite:* O controle deve ser reconhecido imediatamente via evento `SDL_CONTROLLERDEVICEADDED` e controlar a nave e tiros perfeitamente.
3. **QA-S6-03 (Consistência de Save State / Load State):**
   - *Procedimento:* Jogar até a onda 2 com pontuação alta, pressionar <kbd>F5</kbd>, deixar o canhão ser destruído e pressionar <kbd>F7</kbd>.
   - *Critério de Aceite:* A CPU, memória, pontuação, posições dos invasores e estado do áudio devem ser restaurados exatamente ao momento do save.
