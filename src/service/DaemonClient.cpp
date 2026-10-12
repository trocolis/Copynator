// =============================================================================
// DaemonClient.cpp — consultas ao daemon via D-Bus
// =============================================================================
#include "service/DaemonClient.hpp"

#include <giomm/dbusconnection.h>
#include <giomm/init.h>
#include <glibmm/error.h>
#include <glibmm/variant.h>
#include <string>

namespace copynator::service {

namespace {

// Tempo máximo de espera por respostas do D-Bus (ms).
constexpr int kCallTimeoutMs = 2000;

// Obtém a conexão com o barramento de sessão (ou nulo se indisponível).
Glib::RefPtr<Gio::DBus::Connection> sessionBus() {
    Gio::init(); // seguro chamar mais de uma vez
    try {
        return Gio::DBus::Connection::get_sync(Gio::DBus::BusType::SESSION);
    } catch (const Glib::Error&) {
        return {};
    }
}

} // namespace

bool DaemonClient::isRunning() {
    const auto bus = sessionBus();
    if (!bus) {
        return false;
    }
    try {
        // Pergunta ao próprio barramento se o nome tem dono.
        const auto reply = bus->call_sync(
            "/org/freedesktop/DBus", "org.freedesktop.DBus", "NameHasOwner",
            Glib::VariantContainerBase::create_tuple(
                Glib::Variant<Glib::ustring>::create(std::string(kBusName))),
            "org.freedesktop.DBus", kCallTimeoutMs);
        Glib::Variant<bool> hasOwner;
        reply.get_child(hasOwner, 0);
        return hasOwner.get();
    } catch (const Glib::Error&) {
        return false;
    }
}

std::optional<DaemonStatus> DaemonClient::queryStatus() {
    if (!isRunning()) {
        return std::nullopt;
    }
    const auto bus = sessionBus();
    try {
        const auto reply = bus->call_sync(std::string(kObjectPath), std::string(kInterfaceName),
                                          "GetStatus", {}, std::string(kBusName), kCallTimeoutMs);
        Glib::Variant<Glib::ustring> version;
        Glib::Variant<guint32> itemCount;
        Glib::Variant<bool> windowVisible;
        reply.get_child(version, 0);
        reply.get_child(itemCount, 1);
        reply.get_child(windowVisible, 2);
        return DaemonStatus{
            .version = version.get().raw(),
            .itemCount = itemCount.get(),
            .windowVisible = windowVisible.get(),
        };
    } catch (const Glib::Error&) {
        return std::nullopt;
    }
}

} // namespace copynator::service
