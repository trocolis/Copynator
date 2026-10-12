// =============================================================================
// GnomeShortcut.cpp — atalho global via GSettings do GNOME
// =============================================================================
#include "platform/GnomeShortcut.hpp"

#include "core/Accelerator.hpp"

#include <algorithm>
#include <array>
#include <format>
#include <gio/gio.h>
#include <giomm/init.h>
#include <giomm/settings.h>
#include <giomm/settingsschemasource.h>

namespace copynator::platform {

namespace {

constexpr const char* kMediaKeysSchema = "org.gnome.settings-daemon.plugins.media-keys";
constexpr const char* kCustomKeybindingSchema =
    "org.gnome.settings-daemon.plugins.media-keys.custom-keybinding";
constexpr const char* kCustomKeybindingsKey = "custom-keybindings";
constexpr const char* kCopynatorPath =
    "/org/gnome/settings-daemon/plugins/media-keys/custom-keybindings/copynator/";
constexpr const char* kShortcutName = "Copynator";

// Esquemas com atalhos do sistema verificados na busca por conflitos.
constexpr std::array kKeybindingSchemas{
    "org.gnome.desktop.wm.keybindings",
    "org.gnome.shell.keybindings",
    "org.gnome.mutter.keybindings",
    "org.gnome.mutter.wayland.keybindings",
    "org.gnome.settings-daemon.plugins.media-keys",
};

// Verifica se um esquema está instalado (Gio::Settings aborta o programa se
// o esquema não existir, então é preciso checar antes).
bool schemaExists(const char* schemaId) {
    const auto source = Gio::SettingsSchemaSource::get_default();
    return source && source->lookup(schemaId, true);
}

std::expected<void, std::string> requireGnomeSchemas() {
    Gio::init();
    if (!schemaExists(kMediaKeysSchema) || !schemaExists(kCustomKeybindingSchema)) {
        return std::unexpected(
            "esquemas do GNOME Settings Daemon não encontrados (este ambiente é o GNOME?)");
    }
    return {};
}

// Lê uma chave como lista de atalhos, aceitando chaves do tipo "s" e "as".
std::vector<std::string> readAccelerators(GSettings* settings, const char* key) {
    std::vector<std::string> accelerators;
    GVariant* value = g_settings_get_value(settings, key);
    if (g_variant_is_of_type(value, G_VARIANT_TYPE_STRING)) {
        accelerators.emplace_back(g_variant_get_string(value, nullptr));
    } else if (g_variant_is_of_type(value, G_VARIANT_TYPE_STRING_ARRAY)) {
        gsize length = 0;
        const gchar** items = g_variant_get_strv(value, &length);
        accelerators.assign(items, items + length);
        g_free(static_cast<gpointer>(items));
    }
    g_variant_unref(value);
    return accelerators;
}

// Procura atalhos do sistema que usem a mesma combinação de teclas.
// Usa a API C para listar chaves de qualquer tipo de forma genérica.
void findSystemConflicts(const std::string& binding, std::vector<std::string>& conflicts) {
    GSettingsSchemaSource* source = g_settings_schema_source_get_default();
    for (const char* schemaId : kKeybindingSchemas) {
        GSettingsSchema* schema = g_settings_schema_source_lookup(source, schemaId, TRUE);
        if (schema == nullptr) {
            continue;
        }
        GSettings* settings = g_settings_new(schemaId);
        gchar** keys = g_settings_schema_list_keys(schema);
        for (gchar** key = keys; *key != nullptr; ++key) {
            for (const auto& accelerator : readAccelerators(settings, *key)) {
                if (core::sameAccelerator(accelerator, binding)) {
                    conflicts.push_back(std::format("{} {}", schemaId, *key));
                }
            }
        }
        g_strfreev(keys);
        g_object_unref(settings);
        g_settings_schema_unref(schema);
    }
}

} // namespace

std::expected<void, std::string> GnomeShortcut::install(const std::string& command,
                                                        const std::string& binding) {
    if (auto ready = requireGnomeSchemas(); !ready) {
        return ready;
    }

    // 1. Preenche as chaves do nosso atalho personalizado.
    auto shortcut = Gio::Settings::create(kCustomKeybindingSchema, kCopynatorPath);
    shortcut->set_string("name", kShortcutName);
    shortcut->set_string("command", command);
    shortcut->set_string("binding", binding);

    // 2. Inclui o caminho na lista de atalhos personalizados (sem duplicar e
    //    preservando os atalhos que o usuário já tinha).
    auto mediaKeys = Gio::Settings::create(kMediaKeysSchema);
    auto paths = mediaKeys->get_string_array(kCustomKeybindingsKey);
    if (std::ranges::find(paths, kCopynatorPath) == paths.end()) {
        paths.emplace_back(kCopynatorPath);
        mediaKeys->set_string_array(kCustomKeybindingsKey, paths);
    }

    // As escritas do dconf são assíncronas: garante a gravação antes de sair.
    g_settings_sync();
    return {};
}

std::expected<ShortcutStatus, std::string> GnomeShortcut::status(const std::string& binding) {
    if (auto ready = requireGnomeSchemas(); !ready) {
        return std::unexpected(ready.error());
    }

    ShortcutStatus status;
    auto mediaKeys = Gio::Settings::create(kMediaKeysSchema);
    const auto paths = mediaKeys->get_string_array(kCustomKeybindingsKey);

    for (const auto& path : paths) {
        auto custom = Gio::Settings::create(kCustomKeybindingSchema, path);
        const auto customBinding = custom->get_string("binding").raw();

        if (path == kCopynatorPath) {
            status.installed = true;
            status.binding = customBinding;
            status.command = custom->get_string("command").raw();
        } else if (core::sameAccelerator(customBinding, binding)) {
            status.conflicts.push_back(
                std::format("atalho personalizado \"{}\"", custom->get_string("name").raw()));
        }
    }

    findSystemConflicts(binding, status.conflicts);
    return status;
}

} // namespace copynator::platform
