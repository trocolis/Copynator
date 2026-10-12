// =============================================================================
// HistoryRow.hpp
//
// Linha da lista que representa um item do histórico:
//
//   ┌──────────────────────────────────────────────┐
//   │ Prévia do texto copiado (até 3 linhas)…  📌 🗑 │
//   │ [Link]  42 caracteres · há 5 min              │
//   └──────────────────────────────────────────────┘
//
// A linha apenas EXIBE o item e emite sinais; quem decide o que fazer (fixar,
// excluir, copiar) é a janela, que conhece o HistoryService.
// =============================================================================
#pragma once

#include "core/ClipboardItem.hpp"

#include <gtkmm/box.h>
#include <gtkmm/button.h>
#include <gtkmm/label.h>
#include <gtkmm/listboxrow.h>
#include <sigc++/signal.h>

namespace copynator::ui {

class HistoryRow final : public Gtk::ListBoxRow {
public:
    // `now` é recebido para que todas as linhas usem o mesmo "agora" ao
    // calcular a data relativa.
    HistoryRow(const core::ClipboardItem& item, core::Timestamp now);

    [[nodiscard]] const core::ClipboardItem& item() const { return item_; }

    // Sinais emitidos pelos botões de ação da linha.
    sigc::signal<void()>& signalPinToggled() { return pinToggled_; }
    sigc::signal<void()>& signalDeleteRequested() { return deleteRequested_; }

private:
    void buildPreview();
    void buildMetadata(core::Timestamp now);
    void buildActions();

    core::ClipboardItem item_;

    // Widgets (membros diretos: o ciclo de vida acompanha o da linha).
    Gtk::Box layout_{Gtk::Orientation::HORIZONTAL, 6};
    Gtk::Box textColumn_{Gtk::Orientation::VERTICAL, 6};
    Gtk::Box previewBox_{Gtk::Orientation::VERTICAL, 0}; // rótulos da prévia
    Gtk::Box metadata_{Gtk::Orientation::HORIZONTAL, 6};
    Gtk::Label kindBadge_;
    Gtk::Label details_;
    Gtk::Box actions_{Gtk::Orientation::VERTICAL, 2};
    Gtk::Button pinButton_;
    Gtk::Button deleteButton_;

    sigc::signal<void()> pinToggled_;
    sigc::signal<void()> deleteRequested_;
};

} // namespace copynator::ui
