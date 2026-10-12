// =============================================================================
// CopynatorApplication.hpp
//
// Raiz de composição do daemon. Responsabilidades:
//   - garantir instância única por sessão (Gtk::Application + D-Bus);
//   - receber a linha de comando de novas invocações (`copynator --center`
//     executado com o daemon já rodando é repassado para esta instância);
//   - criar a janela sob demanda e tratar a escolha de itens;
//   - publicar a interface D-Bus usada pela extensão do GNOME Shell.
//
// Ciclo de vida:
//   - `--background` segura (hold) a aplicação sem mostrar a janela;
//   - depois que a janela é criada, ela mantém o processo vivo (a janela é
//     apenas escondida, nunca destruída), preservando a posse da área de
//     transferência e o monitoramento;
//   - comandos pontuais (ex.: `--clear`) sem daemon rodando encerram ao final.
// =============================================================================
#pragma once

#include "core/HistoryService.hpp"
#include "service/DaemonInterface.hpp"

#include <functional>
#include <gtkmm/application.h>
#include <memory>

namespace copynator::ui {
class HistoryWindow;
}
namespace copynator::service {
class DaemonDBusService;
}

namespace copynator::app {

class CopynatorApplication final : public Gtk::Application, public service::IDaemonController {
public:
    // Fábrica do serviço de histórico. É chamada apenas na instância
    // principal, evitando abrir o banco em invocações que só repassam
    // argumentos para o daemon.
    using HistoryServiceFactory = std::function<std::unique_ptr<core::HistoryService>()>;

    static Glib::RefPtr<CopynatorApplication> create(HistoryServiceFactory factory);

    ~CopynatorApplication() override;

    // --- IDaemonController ---------------------------------------------------
    void show(core::Placement placement) override;
    void toggle(core::Placement placement) override;
    void hide() override;
    void addEntry(std::string_view content) override;
    std::size_t clearHistory() override;
    [[nodiscard]] service::DaemonStatus status() const override;

protected:
    explicit CopynatorApplication(HistoryServiceFactory factory);

    void on_startup() override;
    void on_shutdown() override;
    int on_command_line(const Glib::RefPtr<Gio::ApplicationCommandLine>& commandLine) override;

private:
    // Cria a janela na primeira vez em que ela for necessária.
    ui::HistoryWindow& ensureWindow();

    // Mantém o processo vivo sem janela visível (modo daemon).
    void enterBackground();

    // Coloca o conteúdo do item na área de transferência do sistema.
    void copyToClipboard(const core::ClipboardItem& item);

    HistoryServiceFactory historyFactory_;
    std::unique_ptr<core::HistoryService> history_;
    std::unique_ptr<ui::HistoryWindow> window_;
    std::unique_ptr<service::DaemonDBusService> dbusService_;
    bool backgroundHeld_ = false;
};

} // namespace copynator::app
