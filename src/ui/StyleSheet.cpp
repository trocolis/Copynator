// =============================================================================
// StyleSheet.cpp — CSS embutido da interface
// =============================================================================
#include "ui/StyleSheet.hpp"

#include <gtkmm/cssprovider.h>
#include <gtkmm/styleprovider.h>

namespace copynator::ui {

namespace {

// O CSS fica embutido no binário: nenhum arquivo extra precisa ser instalado.
constexpr const char* kStyleSheet = R"css(
/* ---------------------------------------------------------------------------
 * Janela flutuante: cantos arredondados (a sombra vem do modo CSD do GTK)
 * ------------------------------------------------------------------------- */
window.copynator-window {
    border-radius: 12px;
}

.copynator-window .window-content {
    padding: 8px 0 8px 0;
}

/* Cabeçalho com o título e o botão "Limpar tudo" */
.copynator-window .window-header {
    padding: 4px 12px 6px 16px;
}

.copynator-window .window-title {
    font-weight: bold;
    font-size: 1.05em;
}

.copynator-window .search-entry {
    margin: 0 12px 8px 12px;
}

/* ---------------------------------------------------------------------------
 * Lista de itens
 * ------------------------------------------------------------------------- */
.copynator-window .history-list {
    background: transparent;
}

.history-row {
    margin: 3px 10px;
    padding: 10px 10px 8px 12px;
    border-radius: 8px;
    background-color: var(--card-bg-color);
    box-shadow: 0 0 0 1px var(--card-shade-color);
}

.history-row:hover {
    background-color: color-mix(in srgb, var(--card-bg-color) 92%, currentColor);
}

/* Foco/seleção destacados com a cor de destaque do sistema */
.history-row:focus-visible,
.history-row:selected {
    background-color: color-mix(in srgb, var(--accent-bg-color) 18%, var(--card-bg-color));
    box-shadow: 0 0 0 2px var(--accent-bg-color);
}

.history-row.pinned {
    box-shadow: inset 3px 0 0 var(--accent-bg-color), 0 0 0 1px var(--card-shade-color);
}

.history-row .preview {
    font-size: 0.95em;
}

.history-row .preview.code {
    font-family: monospace;
    font-size: 0.88em;
}

/* ---------------------------------------------------------------------------
 * Etiquetas (badges): tipo, quantidade de caracteres e data
 * ------------------------------------------------------------------------- */
.history-row .badge {
    font-size: 0.75em;
    padding: 1px 8px;
    border-radius: 999px;
    background-color: alpha(currentColor, 0.08);
}

.history-row .badge.link {
    color: var(--accent-color);
    background-color: color-mix(in srgb, var(--accent-bg-color) 20%, transparent);
}

.history-row .badge.code {
    /* Mistura com a cor do texto: legível tanto no tema claro quanto no escuro */
    color: color-mix(in srgb, #9141ac 65%, currentColor);
    background-color: alpha(#9141ac, 0.18);
}

.history-row .meta {
    font-size: 0.75em;
    opacity: 0.6;
}

/* Faixa inferior do cartão com caracteres e tempo */
.history-row .row-footer {
    margin-top: 2px;
}

/* ---------------------------------------------------------------------------
 * Ações do item (fixar/excluir): discretas até passar o mouse
 * ------------------------------------------------------------------------- */
.history-row .row-action {
    min-width: 26px;
    min-height: 26px;
    padding: 0;
    opacity: 0.35;
}

.history-row:hover .row-action,
.history-row:focus-within .row-action {
    opacity: 0.85;
}

.history-row .row-action.active {
    opacity: 1;
    color: var(--accent-color);
}

/* ---------------------------------------------------------------------------
 * Estado vazio
 * ------------------------------------------------------------------------- */
.copynator-window .empty-state {
    opacity: 0.55;
}

.copynator-window .empty-state image {
    -gtk-icon-size: 48px;
}
)css";

} // namespace

void installStyleSheet() {
    auto provider = Gtk::CssProvider::create();
    provider->load_from_string(kStyleSheet);
    Gtk::StyleProvider::add_provider_for_display(Gdk::Display::get_default(), provider,
                                                GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
}

} // namespace copynator::ui
