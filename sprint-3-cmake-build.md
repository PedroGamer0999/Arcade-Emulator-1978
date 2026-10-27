# Task Plan: Sprint 3 - Portabilidade e Build System CMake (v1.2)

**Task Slug:** `sprint-3-cmake-build`  
**Status:** In Progress  
**Spec Reference:** [`docs/specs/SPEC-004-cmake-crossplatform-build.md`](docs/specs/SPEC-004-cmake-crossplatform-build.md)  
**Branch:** `feature/sprint-3-cmake-build`  

---

## 🎯 Objetivo
Eliminar scripts de compilação frágeis com caminhos hardcoded e unificar a compilação do projeto com CMake 3.16+, garantindo build de 1 comando no Windows (MinGW/Clang/MSVC) e Linux (GCC/Clang) sem intervenção manual.

---

## 📋 Lista de Tarefas

- [ ] **Task 1: Criar `CMakeLists.txt` na raiz**
  - CMake 3.16+, padrão C11, flags estritas por compilador.
  - `find_package(SDL2 REQUIRED)` + fallback via `pkg-config`.
  - `GLOB_RECURSE` de `src/*.c`, suporte a Debug/Release.
  - POST_BUILD: copiar `rom/`, `sounds/` e `SDL2.dll` (Windows) para o diretório do binário.
  - *Verificação:* `cmake -B build -DCMAKE_BUILD_TYPE=Release` sem erros de configuração.

- [ ] **Task 2: Criar script de conveniência `build.bat` (Windows)**
  - 1 comando: detecta MSYS2/MinGW, cria pasta `build/`, chama CMake e compila.
  - Mensagem clara de sucesso/erro.
  - *Verificação:* Execução com código de saída 0 no PowerShell do Windows.

- [ ] **Task 3: Criar script de conveniência `build.sh` (Linux/macOS)**
  - 1 comando: detecta dependências SDL2, cria pasta `build/`, chama CMake e compila.
  - Compatível com bash e sh.
  - *Verificação:* `bash build.sh` em ambiente Linux com `libsdl2-dev` instalado.

- [ ] **Task 4: Build e Validação no Windows (QA-S3-02)**
  - Executar `cmake -B build` e `cmake --build build` no Windows via MSYS2/MinGW.
  - Verificar que `SDL2.dll`, `rom/` e `sounds/` foram copiados para `build/`.
  - *Verificação:* Executável `build/space_invaders.exe` gerado e funcional.

- [ ] **Task 5: Atualização de Documentação e DoD**
  - Atualizar `README.md` com seção de build via CMake (Windows e Linux).
  - Atualizar `docs/sprints.md` e `docs/backlog.md` com status da sprint.
  - *Verificação:* Documentação precisa e atualizada.

---

## ✅ Critérios de Conclusão (DoD)
- Build limpo: zero warnings com `-Wall -Wextra -Wpedantic`.
- `cmake -B build && cmake --build build` funciona no Windows e Linux.
- Assets copiados automaticamente para o diretório de saída.
- README atualizado com instruções claras.
