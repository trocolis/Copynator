// =============================================================================
// HistoryRow.cpp — linha visual de um item do histórico
// =============================================================================
#include "ui/HistoryRow.hpp"

#include "core/RelativeTime.hpp"
#include "core/TextUtils.hpp"

#include <format>
#include <optional>
#include <ranges>
#include <string>
#include <vector>

namespace copynator::ui {

namespace {

// Limites da prévia: suficientes para reconhecer o item sem poluir a lista.
constexpr std::size_t kPreviewCharacters = 280;
constexpr std::size_t kCodePreviewLines = 3;
constexpr std::size_t kCodeLineCharacters = 120;
constexpr int kPreviewMaxLines = 3;

// Para código, preserva a estrutura (uma entrada por linha, com indentação),
// mas limita a quantidade de linhas e o tamanho de cada uma. Uma última
// entrada "…" indica que o trecho continua.
std::vector<std::string> makeCodePreviewLines(std::string_view content) {
    std::vector<std::string> lines;
    for (const auto line : core::trim(content) | std::views::split('\n')) {
        const std::string_view lineView(line.begin(), line.end());
        // Linhas em branco ocupam espaço sem ajudar a reconhecer o trecho.
        if (core::trim(lineView).empty()) {
            continue;
        }
        if (lines.size() == kCodePreviewLines) {
            lines.emplace_back("…");
            break;
        }
        // Preserva a indentação (tabs viram 4 espaços), pois makePreview
        // removeria os espaços iniciais.
        std::string previewLine;
        const auto indentEnd = lineView.find_first_not_of(" \t");
        for (const char character : lineView.substr(0, indentEnd)) {
            previewLine += character == '\t' ? "    " : " ";
        }
        previewLine += core::makePreview(lineView.substr(indentEnd, kCodeLineCharacters * 4),
                                         kCodeLineCharacters);
        lines.push_back(std::move(previewLine));
    }
    return lines;
}

// Texto e classe CSS da etiqueta de tipo.
struct KindBadge {
    const char* text;
    const char* cssClass;
};

// Apenas links e código recebem etiqueta: são os casos em que ela ajuda a
// reconhecer o item. Texto comum (a maioria) fica sem etiqueta, deixando a
// lista mais limpa.
std::optional<KindBadge> badgeFor(core::ContentKind kind) {
    switch (kind) {
    case core::ContentKind::Link:
        return KindBadge{"Link", "link"};
    case core::ContentKind::Code:
        return KindBadge{"Código", "code"};
    case core::ContentKind::Text:
        break;
    }
    return std::nullopt;
}

// Configura um botão de ação pequeno, redondo e sem fundo.
void setupActionButton(Gtk::Button& button, const char* iconName, const char* tooltip) {
    button.set_icon_name(iconName);
    button.set_tooltip_text(tooltip);
    button.add_css_class("flat");
    button.add_css_class("circular");
    button.add_css_class("row-action");
    // Os botões não recebem foco pelo teclado: as setas navegam entre linhas e
    // as ações têm atalhos próprios (Ctrl+P e Delete).
    button.set_focusable(false);
    button.set_valign(Gtk::Align::START);
}

} // namespace

HistoryRow::HistoryRow(const core::ClipboardItem& item, core::Timestamp now) : item_(item) {
    add_css_class("history-row");
    if (item_.pinned) {
        add_css_class("pinned");
    }

    buildPreview();
    buildMetadata(now);
    buildActions();

    // Parte de cima: prévia + botões. Parte de baixo: etiqueta, caracteres e
    // tempo em uma faixa que ocupa toda a largura do cartão.
    topRow_.append(previewBox_);
    topRow_.append(actions_);

    layout_.append(topRow_);
    layout_.append(metadata_);
    set_child(layout_);

    // Leitores de tela anunciam a prévia do conteúdo.
    // (API C usada diretamente: mais simples que montar um Glib::Value.)
    const auto accessibleLabel = core::makePreview(item_.content, 120);
    gtk_accessible_update_property(GTK_ACCESSIBLE(gobj()), GTK_ACCESSIBLE_PROPERTY_LABEL,
                                   accessibleLabel.c_str(), -1);
}

void HistoryRow::buildPreview() {
    previewBox_.set_hexpand(true);

    if (item_.kind == core::ContentKind::Code) {
        // Código: um rótulo de uma linha por linha de código, cada um cortado
        // com reticências. Um único rótulo sem quebra calcularia a altura
        // errado no GTK.
        for (const auto& line : makeCodePreviewLines(item_.content)) {
            auto* label = Gtk::make_managed<Gtk::Label>(line);
            label->add_css_class("preview");
            label->add_css_class("code");
            label->set_xalign(0.0F);
            label->set_ellipsize(Pango::EllipsizeMode::END);
            label->set_max_width_chars(1); // deixa o rótulo encolher com a janela
            previewBox_.append(*label);
        }
        return;
    }

    // Texto/link: um único rótulo com quebra em até 3 linhas e reticências.
    auto* label = Gtk::make_managed<Gtk::Label>(core::makePreview(item_.content, kPreviewCharacters));
    label->add_css_class("preview");
    label->set_xalign(0.0F);
    label->set_wrap(true);
    label->set_wrap_mode(Pango::WrapMode::WORD_CHAR);
    label->set_lines(kPreviewMaxLines);
    label->set_ellipsize(Pango::EllipsizeMode::END);
    label->set_max_width_chars(1);
    previewBox_.append(*label);
}

void HistoryRow::buildMetadata(core::Timestamp now) {
    if (const auto badge = badgeFor(item_.kind)) {
        kindBadge_.set_text(badge->text);
        kindBadge_.add_css_class("badge");
        kindBadge_.add_css_class(badge->cssClass);
        metadata_.append(kindBadge_);
    }

    const auto characters = core::countCharacters(item_.content);
    details_.set_text(std::format("{} {} · {}", characters,
                                  characters == 1 ? "caractere" : "caracteres",
                                  core::formatRelativeTime(item_.copiedAt, now)));
    details_.add_css_class("meta");
    details_.set_xalign(0.0F);

    metadata_.add_css_class("row-footer");
    metadata_.append(details_);
}

void HistoryRow::buildActions() {
    setupActionButton(pinButton_, "view-pin-symbolic",
                      item_.pinned ? "Desafixar (Ctrl+P)" : "Fixar (Ctrl+P)");
    if (item_.pinned) {
        pinButton_.add_css_class("active");
    }
    setupActionButton(deleteButton_, "user-trash-symbolic", "Excluir (Delete)");

    pinButton_.signal_clicked().connect([this] { pinToggled_.emit(); });
    deleteButton_.signal_clicked().connect([this] { deleteRequested_.emit(); });

    actions_.append(pinButton_);
    actions_.append(deleteButton_);
}

} // namespace copynator::ui
