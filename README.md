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
  - Não polui a dock/dash do sistema (`NoDisplay=true`, `skip_taskbar`).
  - Extensão nativa do GNOME Shell em `~/.local/share/gnome-shell/extensions/copynator@bruno.local`.
  - Suporte adicional a StatusNotifierItem / AppIndicator via `ksni`.
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
- 🚀 **Performance & Eficiência**:
  - Monitoramento contínuo em tempo real via extensões XFixes/Wayland com fallback leve, consumindo menos de 0.01% de CPU em segundo plano.

---

## 🚀 Como Executar e Usar

### 1. Compilar e Instalar
```bash
# Compilar binário otimizado
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build

# Criar link simbólico para a sua pasta de binários (~/.local/bin)
mkdir -p ~/.local/bin
ln -sf $(pwd)/build/copynator ~/.local/bin/copynator
```

### 2. Ativar o Atalho Shift + Super + V no GNOME
O Copynator inclui um configurador automático:
```bash
copynator --setup-shortcut
```
Para verificar se o atalho está ativo:
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
  ```

---

## ⌨️ Navegação pelo Teclado

- **Digitar**: Filtra a lista instantaneamente.
- **Seta para baixo (`↓`)**: Move o foco do campo de busca para o primeiro item da lista.
- **Setas `↑` / `↓`**: Navega entre os itens do histórico.
- **`Enter`**: Seleciona o item, copia para a área de transferência e fecha a janela.
- **`Esc`**: Fecha o painel.
- **Clique fora**: Fecha o painel automaticamente.
