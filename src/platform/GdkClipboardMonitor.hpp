// =============================================================================
// GdkClipboardMonitor.hpp
//
// Monitor da área de transferência baseado no GDK.
//
// Comportamento por servidor gráfico:
//   - X11: o GDK usa a extensão XFixes (XFixesSelectSelectionInput) para ser
//     notificado de QUALQUER mudança, mesmo com a janela escondida. Consumo de
//     CPU praticamente nulo: não há polling, apenas eventos.
//   - Wayland: por segurança, o compositor só envia a seleção ao cliente com
//     foco de teclado. Aqui o monitor funciona apenas enquanto a janela está
//     ativa; o monitoramento contínuo é feito pela extensão do GNOME Shell.
// =============================================================================
#pragma once

#include "core/IClipboardMonitor.hpp"

#include <gdkmm/clipboard.h>
#include <gdkmm/display.h>
#include <sigc++/trackable.h>

namespace copynator::platform {

class GdkClipboardMonitor final : public core::IClipboardMonitor, public sigc::trackable {
public:
    explicit GdkClipboardMonitor(const Glib::RefPtr<Gdk::Display>& display);
    ~GdkClipboardMonitor() override;

    void start(TextCallback onTextCopied) override;
    void stop() override;
    [[nodiscard]] std::string_view description() const override;

private:
    void onClipboardChanged();
    void onTextRead(const Glib::RefPtr<Gio::AsyncResult>& result);

    Glib::RefPtr<Gdk::Clipboard> clipboard_;
    sigc::connection changedConnection_;
    TextCallback onTextCopied_;
    bool isWayland_ = false;
};

} // namespace copynator::platform
