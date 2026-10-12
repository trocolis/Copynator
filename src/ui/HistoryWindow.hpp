// =============================================================================
// HistoryWindow.hpp
//
// Janela flutuante do histórico, no estilo do Win+V do Windows 11:
//   - sem barra de título, com cantos arredondados e sombra;
//   - campo de busca que filtra em tempo real;
//   - lista de itens com badges, fixar e excluir;
//   - fecha com Esc, ao perder o foco (clique fora) ou ao escolher um item.
//
// A janela não sabe COMO copiar para a área de transferência: ela apenas
// emite `signalItemChosen`, e a aplicação decide o que fazer.
// =============================================================================
#pragma once

#include "core/HistoryService.hpp"

#include <gtkmm/applicationwindow.h>
#include <gtkmm/box.h>
#include <gtkmm/button.h>
#include <gtkmm/image.h>
#include <gtkmm/label.h>
#include <gtkmm/listbox.h>
#include <gtkmm/scrolledwindow.h>
#include <gtkmm/searchentry.h>
#include <gtkmm/stack.h>
#include <sigc++/signal.h>

namespace copynator::ui {

class HistoryRow;

class HistoryWindow final : public Gtk::ApplicationWindow {
public:
    explicit HistoryWindow(core::HistoryService& history);

    // Limpa a busca, recarrega a lista (atualizando as datas relativas) e
    // mostra a janela com o foco no campo de busca.
    void presentFresh();

    // Emitido quando o usuário escolhe um item (Enter ou clique).
    sigc::signal<void(const core::ClipboardItem&)>& signalItemChosen() { return itemChosen_; }

private:
    // Construção da interface
    void buildLayout();
    void buildHeader();
    void buildEmptyState();
    void connectSignals();

    // Lista
    void reloadItems();
    void scheduleReload();
    [[nodiscard]] HistoryRow* firstRow();
    [[nodiscard]] HistoryRow* focusedRow();
    void chooseRow(Gtk::ListBoxRow* row);

    // Eventos
    bool onKeyPressed(guint keyval, guint keycode, Gdk::ModifierType state);
    void onActiveChanged();

    core::HistoryService& history_;
    bool reloadScheduled_ = false;

    // Widgets
    Gtk::Box content_{Gtk::Orientation::VERTICAL, 0};
    Gtk::Box header_{Gtk::Orientation::HORIZONTAL, 6};
    Gtk::Label title_;
    Gtk::Button clearButton_;
    Gtk::SearchEntry searchEntry_;
    Gtk::Stack stack_;
    Gtk::ScrolledWindow scroller_;
    Gtk::ListBox list_;
    Gtk::Box emptyState_{Gtk::Orientation::VERTICAL, 12};
    Gtk::Image emptyIcon_;
    Gtk::Label emptyLabel_;

    sigc::signal<void(const core::ClipboardItem&)> itemChosen_;
};

} // namespace copynator::ui
