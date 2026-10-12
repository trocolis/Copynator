// =============================================================================
// StoragePaths.cpp — caminhos de dados segundo o padrão XDG
// =============================================================================
#include "storage/StoragePaths.hpp"

#include <cstdlib>

namespace copynator::storage {

std::filesystem::path dataDirectory() {
    // 1º: respeita $XDG_DATA_HOME, se definido e absoluto (exigência do XDG).
    if (const char* xdgDataHome = std::getenv("XDG_DATA_HOME");
        xdgDataHome != nullptr && std::filesystem::path(xdgDataHome).is_absolute()) {
        return std::filesystem::path(xdgDataHome) / "copynator";
    }
    // 2º: padrão ~/.local/share
    if (const char* home = std::getenv("HOME"); home != nullptr) {
        return std::filesystem::path(home) / ".local" / "share" / "copynator";
    }
    // Último recurso: diretório atual (ambiente sem HOME é muito incomum).
    return std::filesystem::current_path() / ".copynator";
}

std::filesystem::path historyDatabasePath() {
    return dataDirectory() / "history.db";
}

} // namespace copynator::storage
