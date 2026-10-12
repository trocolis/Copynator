// =============================================================================
// CopynatorApplication.hpp
//
// Raiz de composição da interface gráfica: cria a janela, conecta-a ao
// HistoryService e trata a escolha de itens (copiar para a área de
// transferência). Herda de Gtk::Application, que garante instância única por
// sessão através do D-Bus (o ID da aplicação é o nome no barramento).
// =============================================================================
#pragma once

#include "core/HistoryService.hpp"

#include <gtkmm/application.h>
#include <memory>

namespace copynator::ui {
class HistoryWindow;
}

namespace copynator::app {

class CopynatorApplication final : public Gtk::Application {
public:
    // A aplicação assume a posse do serviço de histórico.
    static Glib::RefPtr<CopynatorApplication> create(std::unique_ptr<core::HistoryService> history);

    ~CopynatorApplication() override;

protected:
    explicit CopynatorApplication(std::unique_ptr<core::HistoryService> history);

    void on_startup() override;
    void on_activate() override;

private:
    // Coloca o conteúdo do item na área de transferência do sistema.
    void copyToClipboard(const core::ClipboardItem& item);

    std::unique_ptr<core::HistoryService> history_;
    std::unique_ptr<ui::HistoryWindow> window_;
};

} // namespace copynator::app
