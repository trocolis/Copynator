// =============================================================================
// ShellExtensionState.cpp — estado da extensão do GNOME Shell via GSettings
// =============================================================================
#include "platform/ShellExtensionState.hpp"

#include <algorithm>
#include <giomm/settingsschemasource.h>

namespace copynator::platform {

namespace {

constexpr const char* kShellSchema = "org.gnome.shell";
constexpr const char* kExtensionUuid = "copynator@bruno.local";

} // namespace

ShellExtensionState::ShellExtensionState() {
    // Fora do GNOME o esquema não existe; Gio::Settings abortaria o programa.
    const auto source = Gio::SettingsSchemaSource::get_default();
    if (source && source->lookup(kShellSchema, true)) {
        shellSettings_ = Gio::Settings::create(kShellSchema);
        shellSettings_->signal_changed().connect([this](const Glib::ustring& key) {
            if (callback_ && (key == "enabled-extensions" || key == "disable-user-extensions")) {
                callback_(isExtensionEnabled());
            }
        });
    }
}

bool ShellExtensionState::isExtensionEnabled() const {
    if (!shellSettings_) {
        return false;
    }
    // O usuário pode desativar todas as extensões de uma vez.
    if (shellSettings_->get_boolean("disable-user-extensions")) {
        return false;
    }
    const auto enabled = shellSettings_->get_string_array("enabled-extensions");
    return std::ranges::find(enabled, kExtensionUuid) != enabled.end();
}

void ShellExtensionState::onChanged(ChangeCallback callback) {
    callback_ = std::move(callback);
}

} // namespace copynator::platform
