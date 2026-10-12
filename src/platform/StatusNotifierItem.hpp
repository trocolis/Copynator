// =============================================================================
// StatusNotifierItem.hpp
//
// Ícone na bandeja do sistema pelo protocolo StatusNotifierItem (SNI), usado
// pelo KDE, XFCE, Cinnamon, Budgie e pelo GNOME com a extensão AppIndicator.
//
// Componentes publicados no D-Bus:
//   - org.kde.StatusNotifierItem em /StatusNotifierItem (ícone, dica, cliques);
//   - com.canonical.dbusmenu em /MenuBar (menu do clique direito).
// O item é anunciado ao org.kde.StatusNotifierWatcher sempre que ele aparecer
// no barramento (inclusive se o painel for reiniciado).
//
// Implementado com a API C do GDBus: as assinaturas aninhadas desses
// protocolos — ex.: (ia{sv}av) — ficam bem mais claras com as strings de
// formato do GVariant do que com os templates do glibmm.
// =============================================================================
#pragma once

#include <functional>
#include <gio/gio.h>
#include <string>

namespace copynator::platform {

// Ações disparadas pelo ícone. `activationToken` é o token xdg-activation
// fornecido pelo painel (pode ser vazio); no Wayland ele permite que a janela
// receba o foco.
struct TrayActions {
    std::function<void(const std::string& activationToken)> toggleWindow;
    std::function<void()> clearHistory;
    std::function<void()> setupShortcut;
};

class StatusNotifierItem {
public:
    StatusNotifierItem(GDBusConnection* connection, TrayActions actions);
    ~StatusNotifierItem();

    StatusNotifierItem(const StatusNotifierItem&) = delete;
    StatusNotifierItem& operator=(const StatusNotifierItem&) = delete;

    // Publica (true) ou remove (false) o ícone da bandeja.
    void setEnabled(bool enabled);
    [[nodiscard]] bool isEnabled() const { return enabled_; }

private:
    void publish();
    void unpublish();
    void registerWithWatcher();

    // Tratadores chamados pelos trampolins estáticos do GDBus.
    void handleItemCall(const gchar* method, GVariant* parameters,
                        GDBusMethodInvocation* invocation);
    GVariant* itemProperty(const gchar* property);
    void handleMenuCall(const gchar* method, GVariant* parameters,
                        GDBusMethodInvocation* invocation);
    GVariant* menuProperty(const gchar* property);
    void onMenuItemClicked(int itemId);

    static void onItemMethodCall(GDBusConnection*, const gchar*, const gchar*, const gchar*,
                                 const gchar* method, GVariant* parameters,
                                 GDBusMethodInvocation* invocation, gpointer self);
    static GVariant* onItemGetProperty(GDBusConnection*, const gchar*, const gchar*,
                                       const gchar*, const gchar* property, GError**,
                                       gpointer self);
    static void onMenuMethodCall(GDBusConnection*, const gchar*, const gchar*, const gchar*,
                                 const gchar* method, GVariant* parameters,
                                 GDBusMethodInvocation* invocation, gpointer self);
    static GVariant* onMenuGetProperty(GDBusConnection*, const gchar*, const gchar*,
                                       const gchar*, const gchar* property, GError**,
                                       gpointer self);
    static void onWatcherAppeared(GDBusConnection*, const gchar*, const gchar*, gpointer self);

    GDBusConnection* connection_;
    TrayActions actions_;
    std::string busName_;
    std::string activationToken_; // último token recebido do painel
    bool enabled_ = false;

    GDBusNodeInfo* itemInfo_ = nullptr;
    GDBusNodeInfo* menuInfo_ = nullptr;
    guint itemRegistrationId_ = 0;
    guint menuRegistrationId_ = 0;
    guint ownNameId_ = 0;
    guint watcherWatchId_ = 0;
};

} // namespace copynator::platform
