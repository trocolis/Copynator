// =============================================================================
// CopynatorApplication.cpp — composição da interface gráfica
// =============================================================================
#include "app/CopynatorApplication.hpp"

#include "core/Version.hpp"
#include "ui/HistoryWindow.hpp"
#include "ui/StyleSheet.hpp"

#include <adwaita.h>
#include <gdkmm/clipboard.h>
#include <string>

namespace copynator::app {

Glib::RefPtr<CopynatorApplication>
CopynatorApplication::create(std::unique_ptr<core::HistoryService> history) {
    return Glib::make_refptr_for_instance(new CopynatorApplication(std::move(history)));
}

CopynatorApplication::CopynatorApplication(std::unique_ptr<core::HistoryService> history)
    : Gtk::Application(std::string(version::kAppId)), history_(std::move(history)) {}

// O destrutor fica no .cpp porque HistoryWindow é apenas declarado no .hpp.
CopynatorApplication::~CopynatorApplication() = default;

void CopynatorApplication::on_startup() {
    Gtk::Application::on_startup();

    // O libadwaita traz a folha de estilos do GNOME e acompanha o modo
    // claro/escuro e a cor de destaque escolhidos pelo usuário.
    adw_init();
    ui::installStyleSheet();

    window_ = std::make_unique<ui::HistoryWindow>(*history_);
    add_window(*window_);
    window_->signalItemChosen().connect(sigc::mem_fun(*this, &CopynatorApplication::copyToClipboard));
}

void CopynatorApplication::on_activate() {
    window_->presentFresh();
}

void CopynatorApplication::copyToClipboard(const core::ClipboardItem& item) {
    // A área de transferência pertence ao processo enquanto ele estiver vivo;
    // como o Copynator fica em segundo plano, o conteúdo continua disponível
    // para colar mesmo após a janela ser escondida.
    window_->get_clipboard()->set_text(item.content);

    // Reutilizar um item o move para o topo do histórico.
    history_->recordCopy(item.content);
}

} // namespace copynator::app
