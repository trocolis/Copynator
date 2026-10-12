// =============================================================================
// main.cpp — ponto de entrada do Copynator
//
// Mantido propositalmente enxuto: apenas converte argv, interpreta as opções e
// delega o trabalho real para as camadas da aplicação.
// =============================================================================
#include "app/CopynatorApplication.hpp"
#include "core/CommandLine.hpp"
#include "core/HistoryService.hpp"
#include "core/Version.hpp"
#include "storage/SqliteHistoryRepository.hpp"
#include "storage/StoragePaths.hpp"

#include <array>
#include <cstdlib>
#include <exception>
#include <memory>
#include <print>
#include <span>
#include <string_view>
#include <vector>

namespace {

using namespace copynator;

// Monta o serviço de histórico com o repositório SQLite padrão do usuário.
std::unique_ptr<core::HistoryService> makeHistoryService() {
    auto repository =
        std::make_unique<storage::SqliteHistoryRepository>(storage::historyDatabasePath().string());
    return std::make_unique<core::HistoryService>(std::move(repository));
}

// Executa a interface gráfica. O argv repassado ao GTK contém apenas o nome do
// programa, pois as opções já foram interpretadas pelo nosso parser.
int runGui(char* programName) {
    auto application = app::CopynatorApplication::create(makeHistoryService());
    std::array<char*, 1> guiArguments{programName};
    return application->run(static_cast<int>(guiArguments.size()), guiArguments.data());
}

int dispatch(const core::CommandLineOptions& options, char* programName) {
    using core::CommandAction;

    switch (options.action) {
    case CommandAction::PrintHelp:
        std::print("{}", core::helpText());
        return EXIT_SUCCESS;

    case CommandAction::PrintVersion:
        std::println("{} {}", version::kAppName, version::kString);
        return EXIT_SUCCESS;

    case CommandAction::ClearHistory: {
        // Acesso direto ao banco: funciona mesmo sem o daemon em execução.
        storage::SqliteHistoryRepository repository(storage::historyDatabasePath().string());
        const auto removed = repository.clearUnpinned();
        std::println("{} item(ns) removido(s) do histórico.", removed);
        return EXIT_SUCCESS;
    }

    case CommandAction::ShowAtCursor:
    case CommandAction::ShowCentered:
        return runGui(programName);

    default:
        // As demais ações serão implementadas nas próximas versões.
        std::println(stderr, "copynator: ação ainda não implementada nesta versão");
        return EXIT_FAILURE;
    }
}

} // namespace

int main(int argc, char* argv[]) {
    // Converte argv[1..] em string_view, sem copiar os textos.
    const std::span<char*> rawArguments(argv, static_cast<std::size_t>(argc));
    const std::vector<std::string_view> arguments(rawArguments.begin() + 1, rawArguments.end());

    const auto options = core::parseCommandLine(arguments);
    if (!options) {
        std::println(stderr, "copynator: {}\n", options.error());
        std::print(stderr, "{}", core::helpText());
        return EXIT_FAILURE;
    }

    // Última barreira contra exceções não tratadas (ex.: banco inacessível),
    // garantindo uma mensagem clara em vez de um "terminate called...".
    try {
        return dispatch(*options, rawArguments.front());
    } catch (const std::exception& error) {
        std::println(stderr, "copynator: erro: {}", error.what());
        return EXIT_FAILURE;
    }
}
