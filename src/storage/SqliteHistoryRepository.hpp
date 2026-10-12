// =============================================================================
// SqliteHistoryRepository.hpp
//
// Implementação do IHistoryRepository usando SQLite.
//
// Esquema (versão 1):
//   items(id        INTEGER PRIMARY KEY,
//         content   TEXT    NOT NULL UNIQUE,   -- garante a deduplicação
//         kind      INTEGER NOT NULL,          -- ContentKind
//         pinned    INTEGER NOT NULL,          -- 0/1
//         copied_at INTEGER NOT NULL)          -- milissegundos desde a época
//
// A versão do esquema é guardada em `PRAGMA user_version`, permitindo
// migrações futuras sem perder o histórico do usuário.
// =============================================================================
#pragma once

#include "core/IHistoryRepository.hpp"
#include "storage/SqliteDatabase.hpp"

#include <string>

namespace copynator::storage {

class SqliteHistoryRepository final : public core::IHistoryRepository {
public:
    // Abre o banco no caminho informado (criando diretórios e tabelas se
    // necessário). Use ":memory:" para um banco temporário.
    explicit SqliteHistoryRepository(const std::string& databasePath);

    core::ClipboardItem upsert(std::string_view content, core::ContentKind kind,
                               core::Timestamp copiedAt) override;
    [[nodiscard]] std::vector<core::ClipboardItem> list(std::string_view filter) const override;
    [[nodiscard]] std::optional<core::ClipboardItem> findById(core::ItemId id) const override;
    bool setPinned(core::ItemId id, bool pinned) override;
    bool remove(core::ItemId id) override;
    std::size_t clearUnpinned() override;
    std::size_t pruneUnpinned(std::size_t maxUnpinned) override;

private:
    // Cria/migra as tabelas para a versão atual do esquema.
    void migrate();

    // `mutable` porque consultas (const) também precisam preparar comandos.
    mutable SqliteDatabase database_;
};

} // namespace copynator::storage
