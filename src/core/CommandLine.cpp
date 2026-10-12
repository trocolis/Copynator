// =============================================================================
// CommandLine.cpp — implementação do interpretador de argumentos
// =============================================================================
#include "core/CommandLine.hpp"

#include "core/Version.hpp"

#include <array>
#include <format>
#include <optional>
#include <utility>

namespace copynator::core {

namespace {

// Tabela de mapeamento "flag -> ação". Uma tabela evita cadeias longas de
// if/else e deixa explícito, em um único lugar, tudo o que o programa aceita.
struct FlagMapping {
    std::string_view longFlag;
    std::string_view shortFlag; // vazio quando não existe forma curta
    CommandAction action;
};

constexpr std::array kFlagMappings{
    FlagMapping{"--center", "-c", CommandAction::ShowCentered},
    FlagMapping{"--background", "-b", CommandAction::RunBackground},
    FlagMapping{"--clear", "", CommandAction::ClearHistory},
    FlagMapping{"--status", "-s", CommandAction::ShowStatus},
    FlagMapping{"--setup-shortcut", "", CommandAction::SetupShortcut},
    FlagMapping{"--help", "-h", CommandAction::PrintHelp},
    FlagMapping{"--version", "-v", CommandAction::PrintVersion},
};

// Procura a ação associada a um argumento; retorna vazio se for desconhecido.
std::optional<CommandAction> findAction(std::string_view argument) {
    for (const auto& mapping : kFlagMappings) {
        const bool matchesLong = argument == mapping.longFlag;
        const bool matchesShort = !mapping.shortFlag.empty() && argument == mapping.shortFlag;
        if (matchesLong || matchesShort) {
            return mapping.action;
        }
    }
    return std::nullopt;
}

} // namespace

std::expected<CommandLineOptions, std::string>
parseCommandLine(std::span<const std::string_view> arguments) {
    CommandLineOptions options;
    bool actionAlreadySet = false;

    for (const auto argument : arguments) {
        const auto action = findAction(argument);
        if (!action) {
            return std::unexpected(std::format("Opção desconhecida: '{}'", argument));
        }

        // Combinar ações (ex.: --clear --center) é ambíguo, então é proibido.
        if (actionAlreadySet) {
            return std::unexpected(
                std::format("Apenas uma opção pode ser usada por vez (recebido '{}')", argument));
        }

        options.action = *action;
        actionAlreadySet = true;
    }

    return options;
}

std::string helpText() {
    return std::format(
        "{0} {1} — gerenciador de área de transferência\n"
        "\n"
        "Uso: copynator [OPÇÃO]\n"
        "\n"
        "Sem opções, abre o histórico na posição do cursor do mouse.\n"
        "\n"
        "Opções:\n"
        "  -c, --center          Abre o histórico no centro da tela\n"
        "  -b, --background      Inicia em segundo plano (modo daemon)\n"
        "      --clear           Limpa o histórico não fixado\n"
        "  -s, --status          Mostra o estado do atalho e do daemon\n"
        "      --setup-shortcut  Registra o atalho Shift+Super+V no GNOME\n"
        "  -h, --help            Mostra esta ajuda\n"
        "  -v, --version         Mostra a versão\n",
        version::kAppName, version::kString);
}

} // namespace copynator::core
