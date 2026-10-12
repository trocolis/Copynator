// =============================================================================
// ExecutablePath.hpp
//
// Descobre o caminho do executável do Copynator, usado em comandos
// registrados no sistema (ex.: atalho de teclado).
// =============================================================================
#pragma once

#include <string>

namespace copynator::platform {

// Prefere o `copynator` encontrado no PATH (ex.: ~/.local/bin), que continua
// válido entre recompilações; senão, usa o binário em execução.
[[nodiscard]] std::string resolveExecutablePath();

} // namespace copynator::platform
