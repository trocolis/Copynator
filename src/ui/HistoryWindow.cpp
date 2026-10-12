// =============================================================================
// HistoryWindow.cpp — janela flutuante do histórico
// =============================================================================
#include "ui/HistoryWindow.hpp"

#include "ui/HistoryRow.hpp"

#include <gdk/gdkkeysyms.h>
#include <glibmm/main.h>
#include <gtkmm/eventcontrollerkey.h>

namespace copynator::ui {

namespace {

// Dimensões semelhantes ao painel Win+V do Windows 11.
constexpr int kWindowWidth = 380;
constexpr int kWindowHeight = 500;

// Nomes das páginas do Gtk::Stack.
constexpr const char* kListPage = "list";
constexpr const char* kEmptyPage = "empty";

} // namespace

HistoryWindow::HistoryWindow(core::HistoryService& history) : history_(history) {
    set_title("Copynator");
    set_default_size(kWindowWidth, kWindowHeight);
    set_resizable(false);
    add_css_class("copynator-window");

    // Truque do GTK4 para uma janela "sem decoração" que ainda mantém sombra e
    // cantos arredondados: uma barra de título vazia e invisível no lugar da
    // barra padrão. Com set_decorated(false) a sombra do CSD seria perdida.
    auto* emptyTitlebar = Gtk::make_managed<Gtk::Box>();
    emptyTitlebar->set_visible(false);
    set_titlebar(*emptyTitlebar);

    buildLayout();
    connectSignals();

    // Sempre que o histórico mudar (nova cópia, fixar, excluir), a lista é
    // recarregada. O recarregamento é adiado para o próximo ciclo ocioso.
    history_.addChangeListener([this] { scheduleReload(); });

    reloadItems();
}

// -----------------------------------------------------------------------------
// Construção da interface
// -----------------------------------------------------------------------------

void HistoryWindow::buildLayout() {
    content_.add_css_class("window-content");

    buildHeader();

    searchEntry_.set_placeholder_text("Pesquisar no histórico");
    searchEntry_.add_css_class("search-entry");
    // Qualquer tecla digitada na janela vai para a busca automaticamente.
    searchEntry_.set_key_capture_widget(*this);

    list_.add_css_class("history-list");
    list_.set_selection_mode(Gtk::SelectionMode::NONE);
    list_.set_activate_on_single_click(true);

    scroller_.set_policy(Gtk::PolicyType::NEVER, Gtk::PolicyType::AUTOMATIC);
    scroller_.set_vexpand(true);
    scroller_.set_child(list_);

    buildEmptyState();

    stack_.add(scroller_, kListPage);
    stack_.add(emptyState_, kEmptyPage);
    stack_.set_vexpand(true);

    content_.append(header_);
    content_.append(searchEntry_);
    content_.append(stack_);
    set_child(content_);
}

void HistoryWindow::buildHeader() {
    header_.add_css_class("window-header");

    title_.set_text("Área de transferência");
    title_.add_css_class("window-title");
    title_.set_xalign(0.0F);
    title_.set_hexpand(true);

    clearButton_.set_icon_name("edit-clear-all-symbolic");
    clearButton_.set_tooltip_text("Limpar tudo (exceto fixados)");
    clearButton_.add_css_class("flat");
    clearButton_.add_css_class("circular");
    clearButton_.set_focusable(false);

    header_.append(title_);
    header_.append(clearButton_);
}

void HistoryWindow::buildEmptyState() {
    emptyIcon_.set_from_icon_name("edit-paste-symbolic");
    emptyLabel_.set_wrap(true);
    emptyLabel_.set_justify(Gtk::Justification::CENTER);

    emptyState_.add_css_class("empty-state");
    emptyState_.set_valign(Gtk::Align::CENTER);
    emptyState_.append(emptyIcon_);
    emptyState_.append(emptyLabel_);
}

void HistoryWindow::connectSignals() {
    // Teclado: fase de CAPTURA para tratar Esc/setas antes do campo de busca.
    auto keyController = Gtk::EventControllerKey::create();
    keyController->set_propagation_phase(Gtk::PropagationPhase::CAPTURE);
    keyController->signal_key_pressed().connect(
        sigc::mem_fun(*this, &HistoryWindow::onKeyPressed), false);
    add_controller(keyController);

    // Busca em tempo real.
    searchEntry_.signal_search_changed().connect([this] { reloadItems(); });

    // Enter no campo de busca escolhe o primeiro resultado.
    searchEntry_.signal_activate().connect([this] { chooseRow(firstRow()); });

    // Clique ou Enter em uma linha.
    list_.signal_row_activated().connect([this](Gtk::ListBoxRow* row) { chooseRow(row); });

    clearButton_.signal_clicked().connect([this] { history_.clearUnpinned(); });

    // Fechar a janela apenas a esconde: o daemon continua rodando.
    signal_close_request().connect(
        [this] {
            set_visible(false);
            return true; // impede a destruição da janela
        },
        false);

    // Perdeu o foco (clique fora) -> esconde.
    property_is_active().signal_changed().connect(
        sigc::mem_fun(*this, &HistoryWindow::onActiveChanged));
}

// -----------------------------------------------------------------------------
// Lista
// -----------------------------------------------------------------------------

void HistoryWindow::presentFresh() {
    searchEntry_.set_text("");
    reloadItems();
    present();
    searchEntry_.grab_focus();
}

void HistoryWindow::scheduleReload() {
    // Evita recarregar várias vezes seguidas e, principalmente, evita destruir
    // uma linha dentro do tratador de clique do próprio botão dela.
    if (reloadScheduled_) {
        return;
    }
    reloadScheduled_ = true;
    Glib::signal_idle().connect_once([this] {
        reloadScheduled_ = false;
        reloadItems();
    });
}

void HistoryWindow::reloadItems() {
    // Guarda a posição do foco para restaurá-la após recarregar a lista.
    const auto* previouslyFocused = focusedRow();
    const int focusedIndex = previouslyFocused != nullptr ? previouslyFocused->get_index() : -1;

    list_.remove_all();

    const auto filter = searchEntry_.get_text();
    const auto items = history_.items(filter.raw());
    const auto now = core::Clock::now();

    for (const auto& item : items) {
        auto* row = Gtk::make_managed<HistoryRow>(item, now);
        const auto id = item.id;
        row->signalPinToggled().connect([this, id] { history_.togglePinned(id); });
        row->signalDeleteRequested().connect([this, id] { history_.removeItem(id); });
        list_.append(*row);
    }

    // Estado vazio com mensagem adequada ao contexto.
    if (items.empty()) {
        emptyLabel_.set_text(filter.empty() ? "Nada copiado ainda.\nCopie um texto e ele aparecerá aqui."
                                            : "Nenhum item encontrado.");
        stack_.set_visible_child(kEmptyPage);
        if (focusedIndex >= 0) {
            searchEntry_.grab_focus();
        }
        return;
    }
    stack_.set_visible_child(kListPage);

    if (focusedIndex >= 0) {
        const int lastIndex = static_cast<int>(items.size()) - 1;
        if (auto* row = list_.get_row_at_index(std::min(focusedIndex, lastIndex))) {
            row->grab_focus();
        }
    }
}

HistoryRow* HistoryWindow::firstRow() {
    return dynamic_cast<HistoryRow*>(list_.get_row_at_index(0));
}

HistoryRow* HistoryWindow::focusedRow() {
    // get_focus() retorna o widget com foco; a linha é ele ou um ancestral.
    auto* widget = get_focus();
    while (widget != nullptr) {
        if (auto* row = dynamic_cast<HistoryRow*>(widget)) {
            return row;
        }
        widget = widget->get_parent();
    }
    return nullptr;
}

void HistoryWindow::chooseRow(Gtk::ListBoxRow* row) {
    auto* historyRow = dynamic_cast<HistoryRow*>(row);
    if (historyRow == nullptr) {
        return;
    }
    // Copia o item antes de esconder a janela (a linha pode ser destruída por
    // um recarregamento posterior).
    const auto item = historyRow->item();
    set_visible(false);
    itemChosen_.emit(item);
}

// -----------------------------------------------------------------------------
// Eventos
// -----------------------------------------------------------------------------

bool HistoryWindow::onKeyPressed(guint keyval, guint /*keycode*/, Gdk::ModifierType state) {
    const bool control = (state & Gdk::ModifierType::CONTROL_MASK) == Gdk::ModifierType::CONTROL_MASK;
    auto* row = focusedRow();

    switch (keyval) {
    case GDK_KEY_Escape:
        set_visible(false);
        return true;

    case GDK_KEY_Down:
        // Da busca para o primeiro item da lista.
        if (row == nullptr) {
            if (auto* first = firstRow()) {
                first->grab_focus();
                return true;
            }
        }
        return false;

    case GDK_KEY_Up:
        // Do primeiro item de volta para a busca.
        if (row != nullptr && row->get_index() == 0) {
            searchEntry_.grab_focus();
            return true;
        }
        return false;

    case GDK_KEY_Delete:
    case GDK_KEY_KP_Delete:
        if (row != nullptr) {
            history_.removeItem(row->item().id);
            return true;
        }
        return false;

    case GDK_KEY_p:
    case GDK_KEY_P:
        if (control && row != nullptr) {
            history_.togglePinned(row->item().id);
            return true;
        }
        return false;

    default:
        return false;
    }
}

void HistoryWindow::onActiveChanged() {
    // Comportamento de painel flutuante: clicar fora fecha a janela.
    if (!is_active() && get_visible()) {
        set_visible(false);
    }
}

} // namespace copynator::ui
