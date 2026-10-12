// =============================================================================
// DaemonDBusService.hpp
//
// Publica a interface io.github.copynator.Daemon no barramento de sessão e
// traduz as chamadas D-Bus em chamadas ao IDaemonController.
//
// A classe não conhece GTK nem a janela: apenas o contrato do controlador.
// =============================================================================
#pragma once

#include "core/Placement.hpp"
#include "service/DaemonInterface.hpp"

#include <giomm/dbusconnection.h>
#include <giomm/dbusintrospection.h>

namespace copynator::service {

class DaemonDBusService {
public:
    explicit DaemonDBusService(IDaemonController& controller);
    ~DaemonDBusService();

    DaemonDBusService(const DaemonDBusService&) = delete;
    DaemonDBusService& operator=(const DaemonDBusService&) = delete;

    // Registra o objeto na conexão informada (a do GApplication).
    void exportOn(const Glib::RefPtr<Gio::DBus::Connection>& connection);

    // Remove o registro (chamado automaticamente no destrutor).
    void unexport();

    // Emite o sinal WindowShown(placement).
    void emitWindowShown(core::Placement placement);

private:
    void onMethodCall(const Glib::RefPtr<Gio::DBus::Connection>& connection,
                      const Glib::ustring& sender, const Glib::ustring& objectPath,
                      const Glib::ustring& interfaceName, const Glib::ustring& methodName,
                      const Glib::VariantContainerBase& parameters,
                      const Glib::RefPtr<Gio::DBus::MethodInvocation>& invocation);

    // Lê o argumento "placement" e responde com erro se for inválido.
    std::optional<core::Placement>
    readPlacement(const Glib::VariantContainerBase& parameters,
                  const Glib::RefPtr<Gio::DBus::MethodInvocation>& invocation);

    IDaemonController& controller_;
    Glib::RefPtr<Gio::DBus::NodeInfo> introspection_;
    Glib::RefPtr<Gio::DBus::Connection> connection_;
    Gio::DBus::InterfaceVTable vtable_;
    guint registrationId_ = 0;
};

} // namespace copynator::service
