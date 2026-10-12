# Copynator 📋

**Copynator** é um gerenciador de área de transferência moderno e leve para Linux, inspirado no histórico de área de transferência do Windows 11 (`Win + V`). Construído em **C++** com **GTK4**, integra-se perfeitamente ao GNOME e ao tema do sistema (modo claro/escuro e cores de destaque).

---

## ✨ Recursos

- 🪟 **Interface Flutuante estilo Windows 11 (Sem Barra de Título)**:
  - Totalmente sem decorações/barra de título do sistema operacional (`undecorated`), com cantos arredondados e sombra suave.
  - Abre instantaneamente ao acionar o atalho ou clicar no ícone.
  - Fecha automaticamente ao clicar fora da janela, ao pressionar `Esc` ou ao escolher um item da lista.
- 🎯 **Posicionamento Inteligente**:
  - **Pelo atalho (`Shift + Super + V`)**: Abre exatamente na **posição do cursor do mouse**, respeitando os limites da tela (sempre visível por completo, sem ser cortada).
  - **Pelo ícone na barra**: Abre perfeitamente no **centro da tela**.
- 📌 **Ícone na Área de Indicadores (Junto ao Wifi, Volume e Bateria)**:
  - Não polui a grade de aplicativos nem a dash (`NoDisplay=true`); a janela só existe enquanto está aberta.
  - Extensão nativa do GNOME Shell em `~/.local/share/gnome-shell/extensions/copynator@bruno.local`.
  - Suporte adicional a StatusNotifierItem / AppIndicator (KDE, XFCE, Cinnamon...), implementado diretamente via D-Bus. É desativado automaticamente quando a extensão do GNOME está ativa, evitando ícone duplicado.
  - **Clique esquerdo**: Abre/alterna a janela no centro da tela.
  - **Clique direito**: Menu rápido com opções de limpeza e atalho.
- 🎨 **Respeita o Tema do Sistema**:
  - Utiliza **Libadwaita** para acompanhar o tema escuro/claro, tipografia e bordas arredondadas do seu sistema.
- 💾 **Histórico Persistente**:
  - Armazenamento local rápido e confiável em SQLite (`~/.local/share/copynator/history.db`).
  - Deduplicação inteligente: copiar o mesmo texto novamente move-o para o topo sem duplicar.
- 📌 **Fixar Itens**:
  - Fixe textos ou códigos frequentes para que permaneçam sempre acessíveis e nunca sejam apagados na limpeza geral.
- 🔍 **Busca em Tempo Real**:
  - Filtre rapidamente qualquer item do histórico enquanto digita.
- 🏷️ **Categorização e Badges**:
  - Identifica automaticamente links (`http/https`), blocos de código e textos comuns com etiquetas e contadores de caracteres.
  - Exibe data/hora relativa amigável (*Agora*, *há 5 min*, *ontem*).
- 🔒 **Privacidade**:
  - Senhas copiadas de gerenciadores de senha (marcadas com `x-kde-passwordManagerHint`) não entram no histórico.
- 🚀 **Performance & Eficiência**:
  - Monitoramento orientado a eventos, sem polling: XFixes (via GDK) no X11 e `Meta.Selection` (pela extensão do GNOME Shell) no Wayland.

---

## 🚀 Como Executar e Usar

### Dependências (Arch Linux)
```bash
sudo pacman -S --needed base-devel cmake ninja gtkmm-4.0 libadwaita sqlite gtest
```

### 1. Compilar e Instalar
```bash
# Configura a instalação na pasta do usuário (~/.local)
cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=$HOME/.local

# Compila e roda os testes
cmake --build build
ctest --test-dir build

# Instala o binário, o .desktop, o autostart e a extensão do GNOME Shell
cmake --install build
```

São instalados:

| Arquivo | Destino |
|---|---|
| Executável | `~/.local/bin/copynator` |
| Entrada da aplicação (`NoDisplay=true`) | `~/.local/share/applications/io.github.copynator.desktop` |
| Autostart do daemon (`--background`) | `~/.config/autostart/io.github.copynator.desktop` |
| Extensão do GNOME Shell | `~/.local/share/gnome-shell/extensions/copynator@bruno.local/` |

### 2. Ativar a extensão e o atalho Shift + Super + V no GNOME
```bash
gnome-extensions enable copynator@bruno.local
```
> No **Wayland**, o GNOME Shell só carrega extensões recém-instaladas após encerrar e iniciar a sessão novamente.

O Copynator inclui um configurador automático do atalho (ele também avisa se outro atalho já usa a mesma combinação):
```bash
copynator --setup-shortcut
```
Para verificar o daemon, o monitoramento e o atalho:
```bash
copynator --status
```

### 3. Executar o Copynator
- **Pelo atalho do teclado**: Pressione `Shift + Super + V` em qualquer janela (abre onde o mouse estiver, sem cortar a tela)!
- **Pelo ícone na barra superior**: Clique no ícone de prancheta junto aos indicadores de sistema (abre no centro da tela)!
- **Pelo terminal**:
  ```bash
  copynator               # Abre na posição do cursor
  copynator --center      # Abre no centro da tela
  copynator --background  # Inicia em segundo plano (modo daemon)
  copynator --clear       # Limpa o histórico não fixado
  copynator --status      # Mostra o estado do daemon e do atalho
  copynator --help        # Lista todas as opções
  ```

> Se o daemon já estiver rodando, qualquer chamada `copynator ...` é repassada a ele (instância única).

---

## ⌨️ Navegação pelo Teclado

- **Digitar**: Filtra a lista instantaneamente.
- **Seta para baixo (`↓`)**: Move o foco do campo de busca para o primeiro item da lista.
- **Setas `↑` / `↓`**: Navega entre os itens do histórico.
- **`Enter`** ou **clique**: Copia o item, fecha a janela e **cola** no aplicativo que estava em foco (`Ctrl+V`, ou `Ctrl+Shift+V` em terminais), como no Win+V. A colagem automática é feita pela extensão do GNOME Shell.
- **`Ctrl + P`**: Fixa/desafixa o item em foco.
- **`Delete`**: Exclui o item em foco.
- **`Esc`**: Fecha o painel.
- **Clique fora**: Fecha o painel automaticamente.

---

## 🐧 X11 e Wayland

| Recurso | X11 | Wayland (GNOME) |
|---|---|---|
| Monitorar a área de transferência em segundo plano | Daemon (XFixes via GDK) | Extensão do GNOME Shell |
| Abrir na posição do cursor | Extensão do GNOME Shell | Extensão do GNOME Shell |
| Ícone na barra superior | Extensão (GNOME) ou SNI | Extensão (GNOME) ou SNI |
| Colar automaticamente o item escolhido | Extensão do GNOME Shell | Extensão do GNOME Shell |

No Wayland, por segurança, aplicativos comuns não podem ler a área de transferência em segundo plano, consultar a posição do cursor nem posicionar as próprias janelas. Apenas o GNOME Shell pode fazer isso, por isso a **extensão é necessária no Wayland**. Sem ela, a janela abre na posição escolhida pelo GNOME e só registra cópias feitas enquanto está em foco.

---

## 🏗️ Arquitetura

```
┌───────────── GNOME Shell (extensão JS) ─────────────┐
│ indicator.js        ícone no painel (clique esq./dir.)│
│ clipboardWatcher.js observa a área de transferência   │
│ windowPlacer.js     posiciona a janela (cursor/centro)│
│ paster.js           cola o item escolhido (Ctrl+V)    │
│ daemonClient.js     cliente D-Bus do daemon           │
└──────────────────────┬───────────────────────────────┘
                       │ D-Bus: io.github.copynator.Daemon
┌──────────────────────▼───────────────────────────────┐
│ copynator (C++23, daemon de instância única)          │
│  app/       composição e ciclo de vida                │
│  ui/        janela, linhas, CSS (gtkmm + libadwaita)  │
│  service/   interface D-Bus (servidor e cliente)      │
│  platform/  monitor GDK, atalho GNOME, bandeja SNI    │
│  storage/   repositório SQLite                        │
│  core/      domínio e regras (sem GTK, testável)      │
└───────────────────────────────────────────────────────┘
```

As dependências apontam sempre para o `core`. Ele define as portas (`IHistoryRepository`, `IClipboardMonitor`) que as outras camadas implementam.

### Interface D-Bus

| Membro | Descrição |
|---|---|
| `Show(s placement)` | Mostra a janela (`"cursor"` ou `"center"`) |
| `Toggle(s placement)` | Mostra ou esconde a janela |
| `Hide()` | Esconde a janela |
| `AddEntry(s content)` | Registra um texto copiado |
| `Clear() → u` | Limpa o histórico não fixado |
| `GetStatus() → (s, u, b, s)` | Versão, total de itens, janela visível e monitor em uso |
| sinal `WindowShown(s placement)` | Emitido ao mostrar a janela, para a extensão posicioná-la |
| sinal `PasteRequested()` | Emitido após copiar o item escolhido, para a extensão colá-lo na janela anterior |

### Versionamento

O projeto segue `RELEASE.FEATURE.HOTFIX`. A versão é definida uma única vez em `project(VERSION ...)` no `CMakeLists.txt` e propagada para o binário e para a extensão.

### Migração da versão anterior

Bancos criados pela versão anterior do Copynator (tabela `clipboard_history`) são importados automaticamente na primeira execução. A tabela antiga é preservada.
