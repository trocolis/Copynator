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
#include "service/DaemonClient.hpp"
#include "storage/SqliteHistoryRepository.hpp"
#include "storage/StoragePaths.hpp"

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

// Executa a aplicação GTK com o argv original. Se já houver um daemon rodando,
// o GApplication apenas repassa os argumentos para ele e encerra.
int runApplication(int argc, char* argv[]) {
    auto application = app::CopynatorApplication::create(makeHistoryService);
    return application->run(argc, argv);
}

// --status: mostra o estado do daemon.
int printStatus() {
    if (const auto status = service::DaemonClient::queryStatus()) {
        std::println("Daemon:    em execução (versão {})", status->version);
        std::println("Histórico: {} item(ns)", status->itemCount);
        std::println("Janela:    {}", status->windowVisible ? "visível" : "escondida");
    } else {
        std::println("Daemon:    parado (inicie com `copynator --background`)");
    }
    return EXIT_SUCCESS;
}

int dispatch(const core::CommandLineOptions& options, int argc, char* argv[]) {
    using core::CommandAction;

    switch (options.action) {
    case CommandAction::PrintHelp:
        std::print("{}", core::helpText());
        return EXIT_SUCCESS;

    case CommandAction::PrintVersion:
        std::println("{} {}", version::kAppName, version::kString);
        return EXIT_SUCCESS;

    case CommandAction::ShowStatus:
        return printStatus();

    case CommandAction::SetupShortcut:
        // Implementado em uma versão futura.
        std::println(stderr, "copynator: ação ainda não implementada nesta versão");
        return EXIT_FAILURE;

    case CommandAction::ShowAtCursor:
    case CommandAction::ShowCentered:
    case CommandAction::RunBackground:
    case CommandAction::ClearHistory:
        // Ações do daemon: tratadas em CopynatorApplication::on_command_line.
        return runApplication(argc, argv);
    }
    return EXIT_FAILURE;
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
        return dispatch(*options, argc, argv);
    } catch (const std::exception& error) {
        std::println(stderr, "copynator: erro: {}", error.what());
        return EXIT_FAILURE;
    }
}
