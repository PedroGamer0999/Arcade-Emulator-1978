# SPEC-004: Build System Universal Multiplataforma (CMake)

- **Status:** Aprovado para Implementação (Sprint 3)
- **Autor:** DevOps & Systems Specialist
- **Arquivos Afetados:** `CMakeLists.txt`, `Makefile`, [`makefile.bat`](file:///D:/projetos/008-space-invaders-emulator/makefile.bat), [`makefile.ps1`](file:///D:/projetos/008-space-invaders-emulator/makefile.ps1), `setup.sh`
- **Dependências:** Nenhuma

---

## 1. Contexto e Motivação
Atualmente, o projeto possui três formas distintas de compilação:
- [`makefile.ps1`](file:///D:/projetos/008-space-invaders-emulator/makefile.ps1) com caminhos absolutos atrelados a `C:\msys64\ucrt64\bin`.
- [`makefile.bat`](file:///D:/projetos/008-space-invaders-emulator/makefile.bat) com script em lote simples para Windows.
- [`Makefile`](file:///D:/projetos/008-space-invaders-emulator/Makefile) voltado para Linux sem suporte nativo a cópia de dependências.

Essa disparidade quebra a portabilidade quando outro desenvolvedor ou usuário tenta compilar no Linux (Debian, Ubuntu, Fedora, Arch) ou no Windows via Visual Studio (MSVC), CLion ou MinGW independente.
A solução padrão da indústria de software em C é a adoção do **CMake**, que gera nativamente os arquivos de projeto para qualquer compilador e sistema operacional.

---

## 2. Requisitos

### Requisitos Funcionais (RF)
- **RF-01**: Criar um arquivo `CMakeLists.txt` na raiz compatível com CMake 3.16+.
- **RF-02**: Detectar automaticamente a biblioteca SDL2 no sistema usando `find_package(SDL2 REQUIRED)` ou pacote `pkg-config`.
- **RF-03**: No Windows, copiar automaticamente a biblioteca `SDL2.dll` e a pasta de assets (`rom/`, `sounds/`) para o diretório de saída do binário executável após a compilação.
- **RF-04**: Suportar alvos de compilação `Debug` (com símbolos `-g` e sanitizers) e `Release` (otimizado `-O2` ou `-O3`).
- **RF-05**: Fornecer scripts de conveniência de 1 comando chamando o CMake.

### Requisitos Não-Funcionais (RNF)
- **RNF-01**: Padrão de linguagem C fixado em C11 (`set(CMAKE_C_STANDARD 11)`).
- **RNF-02**: Flags de aviso ativadas: `-Wall -Wextra -Wpedantic` no GCC/Clang e `/W4` no MSVC.

---

## 3. Especificação Técnica e Arquitetura

### 3.1 Estrutura Proposta do `CMakeLists.txt`
```cmake
cmake_minimum_required(VERSION 3.16)
project(SpaceInvadersEmulator C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Opções de compilação e avisos
if(MSVC)
    add_compile_options(/W4)
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# Localização de dependências
find_package(SDL2 REQUIRED)

# Fontes do projeto
file(GLOB_RECURSE SOURCES "src/*.c")
file(GLOB_RECURSE HEADERS "src/*.h")

add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

target_include_directories(${PROJECT_NAME} PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${SDL2_INCLUDE_DIRS}
)

target_link_libraries(${PROJECT_NAME} PRIVATE 
    ${SDL2_LIBRARIES}
)

# No Windows MinGW, garantir SDL2main se necessário
if(MINGW)
    target_link_libraries(${PROJECT_NAME} PRIVATE mingw32 SDL2main)
endif()

# Copiar assets para a pasta do binário
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_CURRENT_SOURCE_DIR}/rom $<TARGET_FILE_DIR:${PROJECT_NAME}>/rom
)

if(WIN32 AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/SDL2.dll")
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_CURRENT_SOURCE_DIR}/SDL2.dll $<TARGET_FILE_DIR:${PROJECT_NAME}>/SDL2.dll
    )
endif()
```

---

## 4. Critérios de Aceite (Definition of Done)
- [x] O comando `cmake -B build && cmake --build build` compila o projeto no Linux sem intervenção manual.
- [x] No Windows, o script de 1 clique compila e gera um executável funcional acompanhado de suas DLLs.
- [x] Compilação limpa (zero warnings com flags estritas).
- [x] Documentação do `README.md` atualizada com instruções claras para Windows e Linux.

---

## 5. Roteiro de Testes de Q/A

| Teste | Descrição | Ação | Resultado Esperado |
| :--- | :--- | :--- | :--- |
| **TC-004-A** | Build no Ubuntu / Debian | Executar `cmake -B build && cmake --build build` em ambiente Linux limpo com `libsdl2-dev` instalado. | Executável `space_invaders` gerado com sucesso e executável diretamente. |
| **TC-004-B** | Build no Windows (MSYS2 / Clang) | Executar `.\build.bat` no terminal do Windows. | Cria a pasta `build/`, compila os objetos e copia `SDL2.dll` automaticamente. |
| **TC-004-C** | Build no Windows (Visual Studio) | Abrir a pasta raiz no Visual Studio ou VS Code com extensão CMake Tools. | Detecta o CMakeLists.txt automaticamente e compila o target Release sem erros de linker. |
