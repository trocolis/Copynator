// =============================================================================
// ShellExtensionState.hpp
//
// Informa se a extensão do Copynator está habilitada no GNOME Shell e avisa
// quando isso mudar. Usado para evitar dois ícones no GNOME: com a extensão
// ativa, o ícone de bandeja SNI do daemon é desativado.
// =============================================================================
#pragma once

#include <functional>
#include <giomm/settings.h>

namespace copynator::platform {

class ShellExtensionState {
public:
    using ChangeCallback = std::function<void(bool extensionEnabled)>;

    ShellExtensionState();

    // A extensão está habilitada? (false fora do GNOME)
    [[nodiscard]] bool isExtensionEnabled() const;

    // Chamado sempre que a lista de extensões habilitadas mudar.
    void onChanged(ChangeCallback callback);

private:
    Glib::RefPtr<Gio::Settings> shellSettings_; // nulo fora do GNOME
    ChangeCallback callback_;
};

} // namespace copynator::platform
