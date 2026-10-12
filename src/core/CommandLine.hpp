// =============================================================================
// CommandLine.hpp
//
// Interpretação dos argumentos de linha de comando do Copynator.
//
// Esta camada é pura (sem GTK) para que possa ser testada isoladamente.
// O resultado é uma estrutura `CommandLineOptions` que descreve a intenção do
// usuário; quem decide COMO executar essa intenção é a camada `app`.
// =============================================================================
#pragma once

#include <expected>
#include <span>
#include <string>
#include <string_view>

namespace copynator::core {

// Ação principal solicitada pelo usuário via linha de comando.
enum class CommandAction {
    ShowAtCursor,   // (padrão) abre a janela na posição do cursor
    ShowCentered,   // --center: abre a janela no centro da tela
    RunBackground,  // --background: inicia o daemon sem mostrar a janela
    ClearHistory,   // --clear: apaga o histórico não fixado
    ShowStatus,     // --status: mostra o estado do atalho/daemon
    SetupShortcut,  // --setup-shortcut: registra Shift+Super+V no GNOME
    PrintHelp,      // --help
    PrintVersion,   // --version
};

// Resultado da interpretação da linha de comando.
struct CommandLineOptions {
    CommandAction action = CommandAction::ShowAtCursor;
};

// Interpreta os argumentos (sem o nome do programa, ou seja, argv[1..]).
//
// Regras:
//   - Nenhum argumento  -> ShowAtCursor
//   - Apenas uma ação é permitida por chamada
//   - Argumento desconhecido -> erro com mensagem amigável
[[nodiscard]] std::expected<CommandLineOptions, std::string>
parseCommandLine(std::span<const std::string_view> arguments);

// Texto de ajuda exibido em --help (e após erros de uso).
[[nodiscard]] std::string helpText();

} // namespace copynator::core
