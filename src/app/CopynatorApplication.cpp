// =============================================================================
// CopynatorApplication.cpp — composição e ciclo de vida do daemon
// =============================================================================
#include "app/CopynatorApplication.hpp"

#include "core/CommandLine.hpp"
#include "core/Version.hpp"
#include "platform/GdkClipboardMonitor.hpp"
#include "service/DaemonDBusService.hpp"
#include "ui/HistoryWindow.hpp"
#include "ui/StyleSheet.hpp"

#include <adwaita.h>
#include <format>
#include <gdkmm/clipboard.h>
#include <giomm/applicationcommandline.h>
#include <string>
#include <vector>

namespace copynator::app {

namespace {

// Converte os argumentos recebidos (inclusive de instâncias remotas) para o
// formato aceito pelo parser do core, ignorando o nome do programa.
std::vector<std::string> argumentsOf(const Glib::RefPtr<Gio::ApplicationCommandLine>& commandLine) {
    int argc = 0;
    char** argv = commandLine->get_arguments(argc);
    std::vector<std::string> arguments;
    for (int index = 1; index < argc; ++index) {
        arguments.emplace_back(argv[index]);
    }
    g_strfreev(argv);
    return arguments;
}

} // namespace

Glib::RefPtr<CopynatorApplication> CopynatorApplication::create(HistoryServiceFactory factory) {
    return Glib::make_refptr_for_instance(new CopynatorApplication(std::move(factory)));
}

CopynatorApplication::CopynatorApplication(HistoryServiceFactory factory)
    // HANDLES_COMMAND_LINE: o argv de novas invocações é entregue a esta
    // instância (a principal) via D-Bus, em on_command_line().
    : Gtk::Application(std::string(version::kAppId), Gio::Application::Flags::HANDLES_COMMAND_LINE),
      historyFactory_(std::move(factory)) {}

// O destrutor fica no .cpp porque os tipos membros são apenas declarados no .hpp.
CopynatorApplication::~CopynatorApplication() = default;

// -----------------------------------------------------------------------------
// Ciclo de vida
// -----------------------------------------------------------------------------

void CopynatorApplication::on_startup() {
    Gtk::Application::on_startup();

    // O libadwaita traz a folha de estilos do GNOME e acompanha o modo
    // claro/escuro e a cor de destaque escolhidos pelo usuário.
    adw_init();
    ui::installStyleSheet();

    history_ = historyFactory_();

    // Cada texto copiado no sistema vira um item do histórico. O próprio
    // Copynator também dispara o monitor ao copiar um item, o que apenas o
    // move para o topo (a deduplicação evita repetições).
    clipboardMonitor_ =
        std::make_unique<platform::GdkClipboardMonitor>(Gdk::Display::get_default());
    clipboardMonitor_->start([this](const std::string& text) { history_->recordCopy(text); });

    // A interface D-Bus é publicada na mesma conexão do GApplication, que já
    // é dona do nome io.github.copynator no barramento de sessão.
    dbusService_ = std::make_unique<service::DaemonDBusService>(*this);
    if (auto connection = get_dbus_connection()) {
        dbusService_->exportOn(connection);
    }
}

void CopynatorApplication::on_shutdown() {
    dbusService_.reset();
    clipboardMonitor_.reset();
    Gtk::Application::on_shutdown();
}

int CopynatorApplication::on_command_line(
    const Glib::RefPtr<Gio::ApplicationCommandLine>& commandLine) {
    using core::CommandAction;

    const auto ownedArguments = argumentsOf(commandLine);
    const std::vector<std::string_view> arguments(ownedArguments.begin(), ownedArguments.end());
    const auto options = core::parseCommandLine(arguments);

    if (!options) {
        commandLine->printerr(std::format("copynator: {}\n", options.error()));
        return EXIT_FAILURE;
    }

    switch (options->action) {
    case CommandAction::ShowAtCursor:
        show(core::Placement::AtCursor);
        return EXIT_SUCCESS;

    case CommandAction::ShowCentered:
        show(core::Placement::Centered);
        return EXIT_SUCCESS;

    case CommandAction::RunBackground:
        enterBackground();
        return EXIT_SUCCESS;

    case CommandAction::ClearHistory: {
        // A saída é impressa no terminal de quem executou o comando, mesmo que
        // seja outra instância.
        const auto removed = clearHistory();
        commandLine->print(std::format("{} item(ns) removido(s) do histórico.\n", removed));
        return EXIT_SUCCESS;
    }

    default:
        // Ajuda, versão, status e atalho são tratados localmente pelo main().
        commandLine->printerr("copynator: opção não suportada pelo daemon\n");
        return EXIT_FAILURE;
    }
}

ui::HistoryWindow& CopynatorApplication::ensureWindow() {
    if (!window_) {
        window_ = std::make_unique<ui::HistoryWindow>(*history_);
        // add_window também "segura" a aplicação: a partir daqui o processo
        // permanece vivo enquanto a janela existir (mesmo escondida).
        add_window(*window_);
        window_->signalItemChosen().connect(
            sigc::mem_fun(*this, &CopynatorApplication::copyToClipboard));
    }
    return *window_;
}

void CopynatorApplication::enterBackground() {
    if (!backgroundHeld_) {
        hold();
        backgroundHeld_ = true;
    }
}

// -----------------------------------------------------------------------------
// IDaemonController
// -----------------------------------------------------------------------------

void CopynatorApplication::show(core::Placement placement) {
    ensureWindow().presentFresh();
    // Avisa a extensão do GNOME Shell para posicionar a janela.
    dbusService_->emitWindowShown(placement);
}

void CopynatorApplication::toggle(core::Placement placement) {
    if (window_ && window_->get_visible()) {
        hide();
    } else {
        show(placement);
    }
}

void CopynatorApplication::hide() {
    if (window_) {
        window_->set_visible(false);
    }
}

void CopynatorApplication::addEntry(std::string_view content) {
    history_->recordCopy(content);
}

std::size_t CopynatorApplication::clearHistory() {
    return history_->clearUnpinned();
}

service::DaemonStatus CopynatorApplication::status() const {
    return service::DaemonStatus{
        .version = std::string(version::kString),
        .itemCount = history_->items().size(),
        .windowVisible = window_ && window_->get_visible(),
        .monitorDescription = std::string(clipboardMonitor_->description()),
    };
}

// -----------------------------------------------------------------------------
// Área de transferência
// -----------------------------------------------------------------------------

void CopynatorApplication::copyToClipboard(const core::ClipboardItem& item) {
    // A área de transferência pertence ao processo enquanto ele estiver vivo;
    // como o Copynator fica em segundo plano, o conteúdo continua disponível
    // para colar mesmo após a janela ser escondida.
    window_->get_clipboard()->set_text(item.content);

    // Reutilizar um item o move para o topo do histórico.
    history_->recordCopy(item.content);
}

} // namespace copynator::app
