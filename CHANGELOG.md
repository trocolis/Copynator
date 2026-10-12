# Changelog

Versionamento: `RELEASE.FEATURE.HOTFIX`.

## [1.3.0] — 2026-10-11
### Adicionado
- Escolher um item (clique ou Enter) também o cola no aplicativo que estava em foco, como no Win+V. A extensão envia `Ctrl+V` (ou `Ctrl+Shift+V` em terminais) por um teclado virtual do GNOME Shell, após o sinal D-Bus `PasteRequested`.

### Alterado
- Botões de fixar e excluir lado a lado, deixando os cartões mais compactos.

### Corrigido
- Compatibilidade com o GNOME 51: o teclado virtual é obtido por `global.stage.context.get_backend()`.

## [1.2.0] — 2026-10-11
### Alterado
- Itens de texto comum não exibem mais a etiqueta "Texto" (apenas "Link" e "Código").
- Caracteres e tempo ficam numa faixa própria na margem inferior do cartão.

## [1.1.1] — 2026-10-11
### Corrigido
- URL do projeto no `metadata.json` da extensão aponta para o repositório real.

## [1.1.0] — 2026-10-11
### Adicionado
- `--status` e `--setup-shortcut` também detectam conflitos com atalhos de extensões habilitadas do GNOME Shell (ex.: outros gerenciadores de área de transferência).

## [1.0.0] — 2026-10-11

Primeira versão estável da reescrita em C++.

## [0.10.1] — 2026-10-11
### Corrigido
- O item escolhido é copiado antes de a janela ser escondida. No Wayland, o compositor só aceita a seleção de um cliente com foco.

## [0.10.0] — 2026-10-11
### Adicionado
- `cmake --install`: binário, `.desktop` (`NoDisplay`), autostart do daemon e extensão do GNOME Shell.
- Importação automática do histórico da versão anterior (tabela `clipboard_history`).

## [0.9.0] — 2026-10-11
### Adicionado
- Ícone de bandeja StatusNotifierItem com menu `com.canonical.dbusmenu`, desativado automaticamente quando a extensão do GNOME está ativa.

## [0.8.0] — 2026-10-11
### Adicionado
- `--setup-shortcut` (Shift+Super+V via GSettings) e detecção de conflitos de atalho.

## [0.7.0] — 2026-10-11
### Adicionado
- Extensão do GNOME Shell: ícone no painel, monitoramento da área de transferência no Wayland e posicionamento no cursor/centro.

## [0.6.0] — 2026-10-11
### Adicionado
- Monitor da área de transferência via GDK (XFixes no X11).

## [0.5.0] — 2026-10-11
### Adicionado
- Daemon de instância única e interface D-Bus `io.github.copynator.Daemon`; `--status`.

## [0.4.0] — 2026-10-11
### Adicionado
- Janela flutuante com busca, badges, fixar/excluir e navegação por teclado.

## [0.3.0] — 2026-10-11
### Adicionado
- Persistência em SQLite com deduplicação, limite de itens e `--clear`.

## [0.2.0] — 2026-10-11
### Adicionado
- Domínio: classificação de conteúdo, tempo relativo e utilitários UTF-8.

## [0.1.0] — 2026-10-11
### Adicionado
- Estrutura do projeto em C++23 com CMake e testes (GoogleTest).
