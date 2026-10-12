// =============================================================================
// GdkClipboardMonitor.cpp — monitoramento via GDK (XFixes no X11)
// =============================================================================
#include "platform/GdkClipboardMonitor.hpp"

#include <gdk/wayland/gdkwayland.h>
#include <gdkmm/contentformats.h>
#include <glibmm/error.h>

namespace copynator::platform {

GdkClipboardMonitor::GdkClipboardMonitor(const Glib::RefPtr<Gdk::Display>& display)
    : clipboard_(display->get_clipboard()),
      isWayland_(GDK_IS_WAYLAND_DISPLAY(display->gobj()) != 0) {}

GdkClipboardMonitor::~GdkClipboardMonitor() {
    stop();
}

void GdkClipboardMonitor::start(TextCallback onTextCopied) {
    onTextCopied_ = std::move(onTextCopied);
    if (!changedConnection_.connected()) {
        changedConnection_ = clipboard_->signal_changed().connect(
            sigc::mem_fun(*this, &GdkClipboardMonitor::onClipboardChanged));
    }
}

void GdkClipboardMonitor::stop() {
    changedConnection_.disconnect();
}

std::string_view GdkClipboardMonitor::description() const {
    return isWayland_ ? "GDK (Wayland: apenas com a janela em foco; use a extensão do GNOME Shell)"
                      : "GDK/XFixes (X11, contínuo)";
}

void GdkClipboardMonitor::onClipboardChanged() {
    // Ignora conteúdos que não são texto (imagens, arquivos, etc.).
    const auto formats = clipboard_->get_formats();
    if (!formats || !formats->contain_gtype(G_TYPE_STRING)) {
        return;
    }
    // Leitura assíncrona: o dono da seleção pode ser outro processo, e a
    // transferência não deve travar a interface. Por herdar de sigc::trackable,
    // o callback é descartado automaticamente se o monitor for destruído antes.
    clipboard_->read_text_async(sigc::mem_fun(*this, &GdkClipboardMonitor::onTextRead));
}

void GdkClipboardMonitor::onTextRead(const Glib::RefPtr<Gio::AsyncResult>& result) {
    try {
        const auto text = clipboard_->read_text_finish(result);
        if (onTextCopied_ && !text.empty()) {
            onTextCopied_(text.raw());
        }
    } catch (const Glib::Error&) {
        // O dono da seleção pode ter mudado ou encerrado durante a leitura;
        // simplesmente ignoramos esta cópia.
    }
}

} // namespace copynator::platform
