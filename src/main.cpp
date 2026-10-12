// =============================================================================
// main.cpp — ponto de entrada do Copynator
//
// Mantido propositalmente enxuto: apenas converte argv, interpreta as opções e
// delega o trabalho real para as camadas da aplicação.
// =============================================================================
#include "app/CopynatorApplication.hpp"
#include "core/Accelerator.hpp"
#include "core/CommandLine.hpp"
#include "core/HistoryService.hpp"
#include "core/Version.hpp"
#include "platform/GnomeShortcut.hpp"
#include "service/DaemonClient.hpp"
#include "storage/SqliteHistoryRepository.hpp"
#include "storage/StoragePaths.hpp"

#include <cstdlib>
#include <exception>
#include <filesystem>
#include <glibmm/miscutils.h>
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

// Caminho do executável usado no atalho. Prefere o `copynator` do PATH
// (ex.: ~/.local/bin), estável entre recompilações; senão, o binário atual.
std::string resolveExecutablePath() {
    if (auto inPath = Glib::find_program_in_path("copynator"); !inPath.empty()) {
        return inPath;
    }
    std::error_code error;
    const auto self = std::filesystem::read_symlink("/proc/self/exe", error);
    return error ? std::string("copynator") : self.string();
}

// Mostra os conflitos encontrados para o atalho.
void printConflicts(const std::vector<std::string>& conflicts) {
    for (const auto& conflict : conflicts) {
        std::println("  ⚠ conflito com: {}", conflict);
    }
}

// --setup-shortcut: registra Shift+Super+V no GNOME.
int setupShortcut() {
    const auto command = resolveExecutablePath();
    const std::string binding(core::kDefaultAccelerator);

    if (const auto installed = platform::GnomeShortcut::install(command, binding); !installed) {
        std::println(stderr, "copynator: não foi possível configurar o atalho: {}",
                     installed.error());
        return EXIT_FAILURE;
    }
    std::println("Atalho Shift+Super+V configurado para executar: {}", command);
    if (const auto status = platform::GnomeShortcut::status(binding); status) {
        printConflicts(status->conflicts);
    }
    return EXIT_SUCCESS;
}

// --status: mostra o estado do daemon e do atalho.
int printStatus() {
    if (const auto status = service::DaemonClient::queryStatus()) {
        std::println("Daemon:    em execução (versão {})", status->version);
        std::println("Histórico: {} item(ns)", status->itemCount);
        std::println("Janela:    {}", status->windowVisible ? "visível" : "escondida");
        std::println("Monitor:   {}", status->monitorDescription);
    } else {
        std::println("Daemon:    parado (inicie com `copynator --background`)");
    }

    const auto shortcut = platform::GnomeShortcut::status(std::string(core::kDefaultAccelerator));
    if (!shortcut) {
        std::println("Atalho:    indisponível ({})", shortcut.error());
    } else if (!shortcut->installed) {
        std::println("Atalho:    não configurado (execute `copynator --setup-shortcut`)");
    } else {
        std::println("Atalho:    {} -> {}", shortcut->binding, shortcut->command);
    }
    if (shortcut) {
        printConflicts(shortcut->conflicts);
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
        return setupShortcut();

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
