// =============================================================================
// StoragePaths.hpp
//
// Resolve onde os dados do Copynator ficam no disco, seguindo a especificação
// XDG Base Directory: $XDG_DATA_HOME/copynator ou ~/.local/share/copynator.
// =============================================================================
#pragma once

#include <filesystem>

namespace copynator::storage {

// Diretório de dados da aplicação (não é criado por esta função).
[[nodiscard]] std::filesystem::path dataDirectory();

// Caminho completo do banco do histórico: <dataDirectory>/history.db
[[nodiscard]] std::filesystem::path historyDatabasePath();

} // namespace copynator::storage
