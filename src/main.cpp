// =============================================================================
// main.cpp — ponto de entrada do Copynator
//
// Mantido propositalmente enxuto: apenas converte argv, interpreta as opções e
// delega o trabalho real para as camadas da aplicação.
// =============================================================================
#include "core/CommandLine.hpp"
#include "core/Version.hpp"

#include <cstdlib>
#include <print>
#include <span>
#include <string_view>
#include <vector>

int main(int argc, char* argv[]) {
    using copynator::core::CommandAction;

    // Converte argv[1..] em string_view, sem copiar os textos.
    const std::span<char*> rawArguments(argv, static_cast<std::size_t>(argc));
    const std::vector<std::string_view> arguments(rawArguments.begin() + 1, rawArguments.end());

    const auto options = copynator::core::parseCommandLine(arguments);
    if (!options) {
        std::println(stderr, "copynator: {}\n", options.error());
        std::print(stderr, "{}", copynator::core::helpText());
        return EXIT_FAILURE;
    }

    switch (options->action) {
    case CommandAction::PrintHelp:
        std::print("{}", copynator::core::helpText());
        return EXIT_SUCCESS;

    case CommandAction::PrintVersion:
        std::println("{} {}", copynator::version::kAppName, copynator::version::kString);
        return EXIT_SUCCESS;

    default:
        // As demais ações serão implementadas nas próximas versões.
        std::println(stderr, "copynator: ação ainda não implementada nesta versão");
        return EXIT_FAILURE;
    }
}
