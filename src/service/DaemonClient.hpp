// =============================================================================
// DaemonClient.hpp
//
// Cliente D-Bus síncrono do daemon, usado pela linha de comando (ex.: --status)
// para consultar a instância em execução sem iniciar uma nova.
// =============================================================================
#pragma once

#include "service/DaemonInterface.hpp"

#include <optional>

namespace copynator::service {

class DaemonClient {
public:
    // Verifica se há um daemon dono do nome no barramento de sessão.
    [[nodiscard]] static bool isRunning();

    // Consulta o estado do daemon. Vazio se ele não estiver em execução ou se
    // a chamada falhar.
    [[nodiscard]] static std::optional<DaemonStatus> queryStatus();
};

} // namespace copynator::service
