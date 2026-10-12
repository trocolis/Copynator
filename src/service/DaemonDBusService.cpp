// =============================================================================
// DaemonDBusService.cpp — servidor D-Bus do daemon
// =============================================================================
#include "service/DaemonDBusService.hpp"

#include <giomm/dbuserror.h>
#include <glibmm/variant.h>
#include <string>

namespace copynator::service {

namespace {

// Resposta de erro padronizada para argumentos inválidos.
void returnInvalidArgs(const Glib::RefPtr<Gio::DBus::MethodInvocation>& invocation,
                       const Glib::ustring& message) {
    invocation->return_error(Gio::DBus::Error(Gio::DBus::Error::INVALID_ARGS, message));
}

} // namespace

DaemonDBusService::DaemonDBusService(IDaemonController& controller)
    : controller_(controller),
      introspection_(Gio::DBus::NodeInfo::create_for_xml(std::string(kIntrospectionXml))),
      vtable_(sigc::mem_fun(*this, &DaemonDBusService::onMethodCall)) {}

DaemonDBusService::~DaemonDBusService() {
    unexport();
}

void DaemonDBusService::exportOn(const Glib::RefPtr<Gio::DBus::Connection>& connection) {
    connection_ = connection;
    registrationId_ = connection_->register_object(
        std::string(kObjectPath), introspection_->lookup_interface(std::string(kInterfaceName)),
        vtable_);
}

void DaemonDBusService::unexport() {
    if (connection_ && registrationId_ != 0) {
        connection_->unregister_object(registrationId_);
    }
    registrationId_ = 0;
    connection_.reset();
}

void DaemonDBusService::emitWindowShown(core::Placement placement) {
    if (!connection_) {
        return;
    }
    // Sinal em broadcast (destino vazio): qualquer interessado pode ouvir.
    const auto parameters = Glib::VariantContainerBase::create_tuple(
        Glib::Variant<Glib::ustring>::create(std::string(core::toString(placement))));
    connection_->emit_signal(std::string(kObjectPath), std::string(kInterfaceName),
                             "WindowShown", {}, parameters);
}

std::optional<core::Placement>
DaemonDBusService::readPlacement(const Glib::VariantContainerBase& parameters,
                                 const Glib::RefPtr<Gio::DBus::MethodInvocation>& invocation) {
    Glib::Variant<Glib::ustring> argument;
    parameters.get_child(argument, 0);
    const auto placement = core::parsePlacement(argument.get().raw());
    if (!placement) {
        returnInvalidArgs(invocation, "placement deve ser \"cursor\" ou \"center\"");
    }
    return placement;
}

void DaemonDBusService::onMethodCall(
    const Glib::RefPtr<Gio::DBus::Connection>& /*connection*/, const Glib::ustring& /*sender*/,
    const Glib::ustring& /*objectPath*/, const Glib::ustring& /*interfaceName*/,
    const Glib::ustring& methodName, const Glib::VariantContainerBase& parameters,
    const Glib::RefPtr<Gio::DBus::MethodInvocation>& invocation) {

    // Os tipos dos argumentos já foram validados pelo GIO com base no XML de
    // introspecção; aqui validamos apenas os VALORES.
    if (methodName == "Show" || methodName == "Toggle") {
        const auto placement = readPlacement(parameters, invocation);
        if (!placement) {
            return; // erro já respondido
        }
        if (methodName == "Show") {
            controller_.show(*placement);
        } else {
            controller_.toggle(*placement);
        }
        invocation->return_value({});
        return;
    }

    if (methodName == "Hide") {
        controller_.hide();
        invocation->return_value({});
        return;
    }

    if (methodName == "AddEntry") {
        Glib::Variant<Glib::ustring> content;
        parameters.get_child(content, 0);
        controller_.addEntry(content.get().raw());
        invocation->return_value({});
        return;
    }

    if (methodName == "Clear") {
        const auto removed = static_cast<guint32>(controller_.clearHistory());
        invocation->return_value(
            Glib::VariantContainerBase::create_tuple(Glib::Variant<guint32>::create(removed)));
        return;
    }

    if (methodName == "GetStatus") {
        const auto status = controller_.status();
        invocation->return_value(Glib::VariantContainerBase::create_tuple({
            Glib::Variant<Glib::ustring>::create(status.version),
            Glib::Variant<guint32>::create(static_cast<guint32>(status.itemCount)),
            Glib::Variant<bool>::create(status.windowVisible),
        }));
        return;
    }

    invocation->return_error(
        Gio::DBus::Error(Gio::DBus::Error::UNKNOWN_METHOD, "Método desconhecido: " + methodName));
}

} // namespace copynator::service
