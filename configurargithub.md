# Guia de Configuração do GitHub: Compilação e Criação Automática de Releases

Este guia explica passo a passo como configurar o seu repositório no GitHub para que, toda vez que você criar e enviar uma **tag de versão** (por exemplo: `v0.0.1`), o GitHub compile o emulador automaticamente no **Windows** e no **Linux**, empacote os arquivos necessários (com ROMs e DLLs) e publique uma **Release pronta para download**.

---

## ⚙️ O que foi configurado no projeto?

Criamos o arquivo de automação do GitHub Actions:
- [`.github/workflows/release.yml`](.github/workflows/release.yml)

### Como ele funciona:
1. **Gatilho:** É disparado automaticamente sempre que você faz `git push` de uma tag iniciada com a letra `v` (ex: `v0.0.1`, `v0.1.0`, `v1.0.0`).
2. **Job Linux:** Inicia uma máquina virtual Ubuntu, instala o compilador e a biblioteca `libsdl2-dev`, compila o emulador e gera o arquivo `space-invaders-vX.Y.Z-linux-x64.tar.gz`.
3. **Job Windows:** Inicia uma máquina virtual Windows com MSYS2 UCRT64, compila com `clang`, inclui a `SDL2.dll` necessária e gera o arquivo `space-invaders-vX.Y.Z-windows-x64.zip`.
4. **Job Release:** Junta os pacotes gerados pelos dois sistemas operacionais e cria uma Release oficial no GitHub com os dois arquivos anexados para download direto.

---

## 📋 Passo a Passo de Configuração

### Passo 1: Inicializar o Git Local e Fazer o Primeiro Commit

Abra o terminal PowerShell na pasta do projeto (`D:\projetos\008-space-invaders-emulator`) e execute os comandos abaixo para inicializar o repositório e preparar os arquivos:

```powershell
# 1. Inicializa o repositório git local (se ainda não estiver inicializado)
git init

# 2. Adiciona todos os arquivos ao controle de versão
git add .

# 3. Faz o primeiro commit com a base do projeto e documentação
git commit -m "feat: base funcional do emulador, docs e workflow de release"
```

---

### Passo 2: Criar o Repositório no GitHub e Vincular

1. Acesse o seu [GitHub](https://github.com/) e clique em **New repository** (Novo Repositório).
2. Dê um nome ao repositório (ex: `space-invaders-emulator`).
3. Deixe o repositório como **Public** ou **Private** (ambos funcionam com GitHub Actions).
4. **NÃO marque** as opções de adicionar README, .gitignore ou License (pois você já tem esses arquivos no projeto local).
5. Clique em **Create repository**.
6. Copie a URL do repositório gerado e vincule no seu terminal local:

```powershell
# Altere a URL abaixo para a URL do SEU repositório no GitHub:
git remote add origin https://github.com/SEU_USUARIO/space-invaders-emulator.git

# Renomeia o branch principal para 'main'
git branch -M main

# Envia os arquivos para o GitHub
git push -u origin main
```

---

### Passo 3: Ativar Permissão de Escrita para o GitHub Actions (CRUCIAL ⚠️)

Por padrão de segurança, o GitHub bloqueia que robôs de automação criem Releases sem permissão explícita. Para liberar:

1. No seu repositório no GitHub, clique na aba **Settings** (Configurações) no menu superior.
2. Na barra lateral esquerda, clique na seção **Actions** e depois em **General**.
3. Role a página até encontrar a seção **"Workflow permissions"** (Permissões de fluxo de trabalho).
4. Marque a opção:
   - 🔘 **Read and write permissions** (Permissões de leitura e escrita).
5. Marque também a caixinha:
   - ☑️ **Allow GitHub Actions to create and approve pull requests**.
6. Clique no botão verde **Save** para salvar.

> ⚠️ **Atenção:** Se este passo não for feito, o GitHub Actions falhará no final do processo com erro `403 Resource not accessible by integration` ao tentar publicar os executáveis na aba de Releases.

---

### Passo 4: Como Gerar uma Nova Versão (Ex: `v0.0.1`)

Quando você quiser publicar uma versão baixável:

```powershell
# 1. Cria a tag anotada com o número da versão e uma mensagem
git tag -a v0.0.1 -m "Versao 0.0.1 beta - Base funcional com documentacao completa"

# 2. Envia a tag para o GitHub
git push origin v0.0.1
```

---

### Passo 5: Acompanhar a Compilação e Baixar os Executáveis

1. No seu navegador, abra o repositório e clique na aba **Actions**.
2. Você verá um workflow chamado **"Build and Release"** em execução.
3. Clique nele para acompanhar os logs em tempo real do Linux e do Windows compilando.
4. Em cerca de 2 a 3 minutos, ambos os builds finalizarão com um ícone verde de sucesso (✅).
5. Vá para a página inicial do seu repositório:
   - Na coluna da direita, na seção **Releases**, estará disponível a versão **Space Invaders Emulator v0.0.1**.
   - Nela você encontrará os dois pacotes prontos para download:
     - 📦 `space-invaders-v0.0.1-windows-x64.zip` (com `space_invaders.exe`, `SDL2.dll` e pasta `rom/`)
     - 📦 `space-invaders-v0.0.1-linux-x64.tar.gz` (com o binário Linux e pasta `rom/`)

---

## 🔄 Como Lançar Futuras Versões?

Conforme formos avançando nas Sprints (Sprint 1, Sprint 2, etc.):
1. Faça os commits com as melhorias: `git commit -am "feat: cpu timing 2.0mhz"`
2. Envie para o branch principal: `git push origin main`
3. Crie a nova tag e envie:
   ```powershell
   git tag -a v0.1.0 -m "Versao 0.1.0 - Sprint 1: CPU Timing a 60 FPS"
   git push origin v0.1.0
   ```
O GitHub cuidará de todo o resto automaticamente!

---

## 💻 Clonando o Repositório em Outra Máquina

Se você (ou outro desenvolvedor) clonar este projeto em um novo computador:

```bash
git clone https://github.com/SEU_USUARIO/space-invaders-emulator.git
cd space-invaders-emulator
```

Basta rodar o instalador de 1 clique:
- **No Windows:** Dê duplo clique em `setup.bat` (ou execute `.\setup.bat` no terminal).
- **No Linux:** Execute `./setup.sh`.

O instalador cuidará de:
1. Instalar o compilador e a biblioteca gráfica SDL2.
2. Inicializar o kit de agentes via `npx @vudovn/ag-kit init`.
3. Restaurar as memórias persistentes versionadas (`.agents/memory/`).
4. Configurar o VS Code sem erros de cabeçalho.
5. Fazer o primeiro build de teste do jogo.

