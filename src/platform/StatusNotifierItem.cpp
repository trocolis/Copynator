// =============================================================================
// StatusNotifierItem.cpp — ícone de bandeja SNI + menu dbusmenu
// =============================================================================
#include "platform/StatusNotifierItem.hpp"

#include "core/Version.hpp"

#include <array>
#include <format>
#include <string_view>
#include <unistd.h>
#include <utility>

namespace copynator::platform {

namespace {

constexpr const char* kItemPath = "/StatusNotifierItem";
constexpr const char* kMenuPath = "/MenuBar";
constexpr const char* kItemInterface = "org.kde.StatusNotifierItem";
constexpr const char* kMenuInterface = "com.canonical.dbusmenu";
constexpr const char* kWatcherName = "org.kde.StatusNotifierWatcher";
constexpr const char* kWatcherPath = "/StatusNotifierWatcher";
constexpr const char* kIconName = "edit-paste-symbolic";

// Especificação do StatusNotifierItem (somente o necessário).
constexpr const char* kItemXml = R"xml(
<node>
  <interface name="org.kde.StatusNotifierItem">
    <property name="Category" type="s" access="read"/>
    <property name="Id" type="s" access="read"/>
    <property name="Title" type="s" access="read"/>
    <property name="Status" type="s" access="read"/>
    <property name="WindowId" type="i" access="read"/>
    <property name="IconName" type="s" access="read"/>
    <property name="IconPixmap" type="a(iiay)" access="read"/>
    <property name="AttentionIconName" type="s" access="read"/>
    <property name="ToolTip" type="(sa(iiay)ss)" access="read"/>
    <property name="ItemIsMenu" type="b" access="read"/>
    <property name="Menu" type="o" access="read"/>
    <method name="Activate"><arg name="x" type="i" direction="in"/><arg name="y" type="i" direction="in"/></method>
    <method name="SecondaryActivate"><arg name="x" type="i" direction="in"/><arg name="y" type="i" direction="in"/></method>
    <method name="ContextMenu"><arg name="x" type="i" direction="in"/><arg name="y" type="i" direction="in"/></method>
    <method name="Scroll"><arg name="delta" type="i" direction="in"/><arg name="orientation" type="s" direction="in"/></method>
    <method name="ProvideXdgActivationToken"><arg name="token" type="s" direction="in"/></method>
    <signal name="NewIcon"/>
    <signal name="NewToolTip"/>
    <signal name="NewStatus"><arg name="status" type="s"/></signal>
  </interface>
</node>
)xml";

// Especificação do com.canonical.dbusmenu (somente o necessário).
constexpr const char* kMenuXml = R"xml(
<node>
  <interface name="com.canonical.dbusmenu">
    <property name="Version" type="u" access="read"/>
    <property name="TextDirection" type="s" access="read"/>
    <property name="Status" type="s" access="read"/>
    <property name="IconThemePath" type="as" access="read"/>
    <method name="GetLayout">
      <arg name="parentId" type="i" direction="in"/>
      <arg name="recursionDepth" type="i" direction="in"/>
      <arg name="propertyNames" type="as" direction="in"/>
      <arg name="revision" type="u" direction="out"/>
      <arg name="layout" type="(ia{sv}av)" direction="out"/>
    </method>
    <method name="GetGroupProperties">
      <arg name="ids" type="ai" direction="in"/>
      <arg name="propertyNames" type="as" direction="in"/>
      <arg name="properties" type="a(ia{sv})" direction="out"/>
    </method>
    <method name="GetProperty">
      <arg name="id" type="i" direction="in"/>
      <arg name="name" type="s" direction="in"/>
      <arg name="value" type="v" direction="out"/>
    </method>
    <method name="Event">
      <arg name="id" type="i" direction="in"/>
      <arg name="eventId" type="s" direction="in"/>
      <arg name="data" type="v" direction="in"/>
      <arg name="timestamp" type="u" direction="in"/>
    </method>
    <method name="EventGroup">
      <arg name="events" type="a(isvu)" direction="in"/>
      <arg name="idErrors" type="ai" direction="out"/>
    </method>
    <method name="AboutToShow">
      <arg name="id" type="i" direction="in"/>
      <arg name="needUpdate" type="b" direction="out"/>
    </method>
    <method name="AboutToShowGroup">
      <arg name="ids" type="ai" direction="in"/>
      <arg name="updatesNeeded" type="ai" direction="out"/>
      <arg name="idErrors" type="ai" direction="out"/>
    </method>
    <signal name="LayoutUpdated"><arg name="revision" type="u"/><arg name="parent" type="i"/></signal>
  </interface>
</node>
)xml";

// -----------------------------------------------------------------------------
// Estrutura do menu (fixa). O id 0 é a raiz exigida pelo protocolo.
// -----------------------------------------------------------------------------
enum MenuItemId : int {
    kRoot = 0,
    kOpenItem = 1,
    kClearItem = 2,
    kSeparator = 3,
    kShortcutItem = 4,
};

struct MenuEntry {
    int id;
    const char* label; // nullptr = separador
};

constexpr std::array kMenuEntries{
    MenuEntry{kOpenItem, "Abrir histórico"},
    MenuEntry{kClearItem, "Limpar histórico (exceto fixados)"},
    MenuEntry{kSeparator, nullptr},
    MenuEntry{kShortcutItem, "Configurar atalho Shift+Super+V"},
};

constexpr guint32 kMenuRevision = 1;

// Propriedades a{sv} de um item do menu.
GVariant* menuItemProperties(int id) {
    GVariantBuilder builder;
    g_variant_builder_init(&builder, G_VARIANT_TYPE("a{sv}"));
    if (id == kRoot) {
        g_variant_builder_add(&builder, "{sv}", "children-display", g_variant_new_string("submenu"));
    }
    for (const auto& entry : kMenuEntries) {
        if (entry.id != id) {
            continue;
        }
        if (entry.label == nullptr) {
            g_variant_builder_add(&builder, "{sv}", "type", g_variant_new_string("separator"));
        } else {
            g_variant_builder_add(&builder, "{sv}", "label", g_variant_new_string(entry.label));
        }
    }
    return g_variant_builder_end(&builder);
}

// Layout completo do menu: (ia{sv}av), em que cada filho é um variant (ia{sv}av).
GVariant* menuLayout() {
    GVariantBuilder children;
    g_variant_builder_init(&children, G_VARIANT_TYPE("av"));
    for (const auto& entry : kMenuEntries) {
        GVariant* child = g_variant_new("(i@a{sv}@av)", entry.id, menuItemProperties(entry.id),
                                        g_variant_new_array(G_VARIANT_TYPE_VARIANT, nullptr, 0));
        g_variant_builder_add(&children, "v", child);
    }
    return g_variant_new("(i@a{sv}@av)", kRoot, menuItemProperties(kRoot),
                         g_variant_builder_end(&children));
}

// Array vazio de ícones em pixels (usamos apenas nomes de ícones do tema).
GVariant* emptyPixmaps() {
    return g_variant_new_array(G_VARIANT_TYPE("(iiay)"), nullptr, 0);
}

} // namespace

// -----------------------------------------------------------------------------
// Ciclo de vida
// -----------------------------------------------------------------------------

StatusNotifierItem::StatusNotifierItem(GDBusConnection* connection, TrayActions actions)
    : connection_(G_DBUS_CONNECTION(g_object_ref(connection))), actions_(std::move(actions)),
      // Nome recomendado pela especificação: org.kde.StatusNotifierItem-PID-ID
      busName_(std::format("org.kde.StatusNotifierItem-{}-1", getpid())) {
    itemInfo_ = g_dbus_node_info_new_for_xml(kItemXml, nullptr);
    menuInfo_ = g_dbus_node_info_new_for_xml(kMenuXml, nullptr);
}

StatusNotifierItem::~StatusNotifierItem() {
    unpublish();
    g_dbus_node_info_unref(itemInfo_);
    g_dbus_node_info_unref(menuInfo_);
    g_object_unref(connection_);
}

void StatusNotifierItem::setEnabled(bool enabled) {
    if (enabled == enabled_) {
        return;
    }
    enabled_ = enabled;
    if (enabled_) {
        publish();
    } else {
        unpublish();
    }
}

void StatusNotifierItem::publish() {
    static const GDBusInterfaceVTable itemVTable{&onItemMethodCall, &onItemGetProperty, nullptr, {}};
    static const GDBusInterfaceVTable menuVTable{&onMenuMethodCall, &onMenuGetProperty, nullptr, {}};

    itemRegistrationId_ = g_dbus_connection_register_object(
        connection_, kItemPath, g_dbus_node_info_lookup_interface(itemInfo_, kItemInterface),
        &itemVTable, this, nullptr, nullptr);
    menuRegistrationId_ = g_dbus_connection_register_object(
        connection_, kMenuPath, g_dbus_node_info_lookup_interface(menuInfo_, kMenuInterface),
        &menuVTable, this, nullptr, nullptr);

    ownNameId_ = g_bus_own_name_on_connection(connection_, busName_.c_str(),
                                              G_BUS_NAME_OWNER_FLAGS_NONE, nullptr, nullptr,
                                              nullptr, nullptr);

    // Registra-se agora e toda vez que o watcher (painel) reaparecer.
    watcherWatchId_ = g_bus_watch_name_on_connection(connection_, kWatcherName,
                                                     G_BUS_NAME_WATCHER_FLAGS_NONE,
                                                     &onWatcherAppeared, nullptr, this, nullptr);
}

void StatusNotifierItem::unpublish() {
    if (watcherWatchId_ != 0) {
        g_bus_unwatch_name(watcherWatchId_);
        watcherWatchId_ = 0;
    }
    if (ownNameId_ != 0) {
        // Liberar o nome faz o watcher remover o ícone do painel.
        g_bus_unown_name(ownNameId_);
        ownNameId_ = 0;
    }
    if (itemRegistrationId_ != 0) {
        g_dbus_connection_unregister_object(connection_, itemRegistrationId_);
        itemRegistrationId_ = 0;
    }
    if (menuRegistrationId_ != 0) {
        g_dbus_connection_unregister_object(connection_, menuRegistrationId_);
        menuRegistrationId_ = 0;
    }
}

void StatusNotifierItem::registerWithWatcher() {
    g_dbus_connection_call(connection_, kWatcherName, kWatcherPath, kWatcherName,
                           "RegisterStatusNotifierItem", g_variant_new("(s)", busName_.c_str()),
                           nullptr, G_DBUS_CALL_FLAGS_NONE, -1, nullptr, nullptr, nullptr);
}

void StatusNotifierItem::onWatcherAppeared(GDBusConnection*, const gchar*, const gchar*,
                                           gpointer self) {
    static_cast<StatusNotifierItem*>(self)->registerWithWatcher();
}

// -----------------------------------------------------------------------------
// org.kde.StatusNotifierItem
// -----------------------------------------------------------------------------

void StatusNotifierItem::onItemMethodCall(GDBusConnection*, const gchar*, const gchar*,
                                          const gchar*, const gchar* method, GVariant* parameters,
                                          GDBusMethodInvocation* invocation, gpointer self) {
    static_cast<StatusNotifierItem*>(self)->handleItemCall(method, parameters, invocation);
}

GVariant* StatusNotifierItem::onItemGetProperty(GDBusConnection*, const gchar*, const gchar*,
                                                const gchar*, const gchar* property, GError**,
                                                gpointer self) {
    return static_cast<StatusNotifierItem*>(self)->itemProperty(property);
}

void StatusNotifierItem::handleItemCall(const gchar* method, GVariant* parameters,
                                        GDBusMethodInvocation* invocation) {
    const std::string_view name(method);

    if (name == "Activate" || name == "SecondaryActivate") {
        // Clique esquerdo/do meio: alterna a janela no centro da tela. O token
        // é consumido para não ser reaproveitado em ativações futuras.
        const auto token = std::exchange(activationToken_, {});
        if (actions_.toggleWindow) {
            actions_.toggleWindow(token);
        }
    } else if (name == "ProvideXdgActivationToken") {
        const gchar* token = nullptr;
        g_variant_get(parameters, "(&s)", &token);
        activationToken_ = token != nullptr ? token : "";
    }
    // ContextMenu e Scroll: o menu é fornecido via dbusmenu; nada a fazer.
    g_dbus_method_invocation_return_value(invocation, nullptr);
}

GVariant* StatusNotifierItem::itemProperty(const gchar* property) {
    const std::string_view name(property);
    const std::string title(version::kAppName);

    if (name == "Category") {
        return g_variant_new_string("ApplicationStatus");
    }
    if (name == "Id") {
        return g_variant_new_string("copynator");
    }
    if (name == "Title") {
        return g_variant_new_string(title.c_str());
    }
    if (name == "Status") {
        return g_variant_new_string("Active");
    }
    if (name == "WindowId") {
        return g_variant_new_int32(0);
    }
    if (name == "IconName") {
        return g_variant_new_string(kIconName);
    }
    if (name == "AttentionIconName") {
        return g_variant_new_string("");
    }
    if (name == "IconPixmap") {
        return emptyPixmaps();
    }
    if (name == "ToolTip") {
        return g_variant_new("(s@a(iiay)ss)", kIconName, emptyPixmaps(), title.c_str(),
                             "Histórico da área de transferência");
    }
    if (name == "ItemIsMenu") {
        return g_variant_new_boolean(FALSE);
    }
    if (name == "Menu") {
        return g_variant_new_object_path(kMenuPath);
    }
    return nullptr;
}

// -----------------------------------------------------------------------------
// com.canonical.dbusmenu
// -----------------------------------------------------------------------------

void StatusNotifierItem::onMenuMethodCall(GDBusConnection*, const gchar*, const gchar*,
                                          const gchar*, const gchar* method, GVariant* parameters,
                                          GDBusMethodInvocation* invocation, gpointer self) {
    static_cast<StatusNotifierItem*>(self)->handleMenuCall(method, parameters, invocation);
}

GVariant* StatusNotifierItem::onMenuGetProperty(GDBusConnection*, const gchar*, const gchar*,
                                                const gchar*, const gchar* property, GError**,
                                                gpointer self) {
    return static_cast<StatusNotifierItem*>(self)->menuProperty(property);
}

GVariant* StatusNotifierItem::menuProperty(const gchar* property) {
    const std::string_view name(property);
    if (name == "Version") {
        return g_variant_new_uint32(3);
    }
    if (name == "TextDirection") {
        return g_variant_new_string("ltr");
    }
    if (name == "Status") {
        return g_variant_new_string("normal");
    }
    if (name == "IconThemePath") {
        return g_variant_new_strv(nullptr, 0);
    }
    return nullptr;
}

void StatusNotifierItem::handleMenuCall(const gchar* method, GVariant* parameters,
                                        GDBusMethodInvocation* invocation) {
    const std::string_view name(method);

    if (name == "GetLayout") {
        // O menu é pequeno e fixo: sempre devolvemos a árvore completa.
        g_dbus_method_invocation_return_value(
            invocation, g_variant_new("(u@(ia{sv}av))", kMenuRevision, menuLayout()));
        return;
    }

    if (name == "GetGroupProperties") {
        GVariantIter* ids = nullptr;
        g_variant_get(parameters, "(ai@as)", &ids, nullptr);
        GVariantBuilder result;
        g_variant_builder_init(&result, G_VARIANT_TYPE("a(ia{sv})"));
        gint32 id = 0;
        if (g_variant_iter_n_children(ids) == 0) {
            // Lista vazia significa "todos os itens" segundo o protocolo.
            g_variant_builder_add(&result, "(i@a{sv})", kRoot, menuItemProperties(kRoot));
            for (const auto& entry : kMenuEntries) {
                g_variant_builder_add(&result, "(i@a{sv})", entry.id, menuItemProperties(entry.id));
            }
        }
        while (g_variant_iter_next(ids, "i", &id)) {
            g_variant_builder_add(&result, "(i@a{sv})", id, menuItemProperties(id));
        }
        g_variant_iter_free(ids);
        g_dbus_method_invocation_return_value(invocation,
                                              g_variant_new("(a(ia{sv}))", &result));
        return;
    }

    if (name == "GetProperty") {
        gint32 id = 0;
        const gchar* property = nullptr;
        g_variant_get(parameters, "(i&s)", &id, &property);
        GVariant* properties = menuItemProperties(id);
        GVariant* value = g_variant_lookup_value(properties, property, nullptr);
        g_variant_unref(g_variant_ref_sink(properties));
        if (value == nullptr) {
            g_dbus_method_invocation_return_dbus_error(invocation, "com.canonical.dbusmenu.Error",
                                                       "Propriedade inexistente");
            return;
        }
        g_dbus_method_invocation_return_value(invocation, g_variant_new("(v)", value));
        g_variant_unref(value);
        return;
    }

    if (name == "Event") {
        gint32 id = 0;
        const gchar* eventId = nullptr;
        g_variant_get(parameters, "(i&svu)", &id, &eventId, nullptr, nullptr);
        if (std::string_view(eventId) == "clicked") {
            onMenuItemClicked(id);
        }
        g_dbus_method_invocation_return_value(invocation, nullptr);
        return;
    }

    if (name == "EventGroup") {
        GVariantIter* events = nullptr;
        g_variant_get(parameters, "(a(isvu))", &events);
        gint32 id = 0;
        const gchar* eventId = nullptr;
        while (g_variant_iter_next(events, "(i&svu)", &id, &eventId, nullptr, nullptr)) {
            if (std::string_view(eventId) == "clicked") {
                onMenuItemClicked(id);
            }
        }
        g_variant_iter_free(events);
        g_dbus_method_invocation_return_value(invocation,
                                              g_variant_new("(@ai)", g_variant_new_array(G_VARIANT_TYPE_INT32, nullptr, 0)));
        return;
    }

    if (name == "AboutToShow") {
        g_dbus_method_invocation_return_value(invocation, g_variant_new("(b)", FALSE));
        return;
    }

    if (name == "AboutToShowGroup") {
        GVariant* empty = g_variant_new_array(G_VARIANT_TYPE_INT32, nullptr, 0);
        GVariant* emptyErrors = g_variant_new_array(G_VARIANT_TYPE_INT32, nullptr, 0);
        g_dbus_method_invocation_return_value(invocation,
                                              g_variant_new("(@ai@ai)", empty, emptyErrors));
        return;
    }

    g_dbus_method_invocation_return_dbus_error(invocation, "org.freedesktop.DBus.Error.UnknownMethod",
                                               "Método desconhecido");
}

void StatusNotifierItem::onMenuItemClicked(int itemId) {
    switch (itemId) {
    case kOpenItem:
        if (actions_.toggleWindow) {
            actions_.toggleWindow(std::exchange(activationToken_, {}));
        }
        break;
    case kClearItem:
        if (actions_.clearHistory) {
            actions_.clearHistory();
        }
        break;
    case kShortcutItem:
        if (actions_.setupShortcut) {
            actions_.setupShortcut();
        }
        break;
    default:
        break;
    }
}

} // namespace copynator::platform
