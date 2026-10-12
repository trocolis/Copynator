// =============================================================================
// GnomeShortcut.hpp
//
// Configura o atalho global do Copynator como "atalho personalizado" do
// GNOME (o mesmo mecanismo de Configurações > Teclado > Atalhos
// personalizados), via GSettings:
//
//   org.gnome.settings-daemon.plugins.media-keys custom-keybindings
//     -> lista de caminhos; o nosso é .../custom-keybindings/copynator/
//   org.gnome.settings-daemon.plugins.media-keys.custom-keybinding (relocável)
//     -> name, command, binding
// =============================================================================
#pragma once

#include <expected>
#include <filesystem>
#include <string>
#include <vector>

namespace copynator::platform {

// Situação atual do atalho.
struct ShortcutStatus {
    bool installed = false;              // nosso atalho está registrado?
    std::string binding;                 // ex.: "<Shift><Super>v"
    std::string command;                 // comando executado
    std::vector<std::string> conflicts;  // outros atalhos com a mesma tecla
};

class GnomeShortcut {
public:
    // Registra (ou atualiza) o atalho para executar `command`.
    // Retorna erro se o ambiente não tiver os esquemas do GNOME.
    static std::expected<void, std::string> install(const std::string& command,
                                                    const std::string& binding);

    // Consulta o estado do atalho e procura conflitos com outros atalhos
    // (do sistema, personalizados e de extensões habilitadas do GNOME Shell).
    static std::expected<ShortcutStatus, std::string> status(const std::string& binding);

    // Procura atalhos iguais a `binding` nos esquemas GSettings das extensões
    // instaladas em `extensionRoots` (diretórios ".../gnome-shell/extensions")
    // cujo UUID esteja em `enabledUuids`. Extensões desabilitadas não contam:
    // seus atalhos não estão ativos.
    // Exposto separadamente para permitir testes com diretórios temporários.
    static std::vector<std::string>
    findExtensionConflicts(const std::string& binding,
                           const std::vector<std::filesystem::path>& extensionRoots,
                           const std::vector<std::string>& enabledUuids);
};

} // namespace copynator::platform
