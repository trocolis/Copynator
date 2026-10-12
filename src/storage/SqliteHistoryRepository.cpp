// =============================================================================
// SqliteHistoryRepository.cpp — persistência do histórico em SQLite
// =============================================================================
#include "storage/SqliteHistoryRepository.hpp"

#include <chrono>
#include <filesystem>

namespace copynator::storage {

namespace {

using core::ClipboardItem;
using core::ContentKind;
using core::Timestamp;

// Versão atual do esquema do banco. Incrementar a cada migração.
constexpr int kSchemaVersion = 1;

// Colunas lidas em todas as consultas, sempre nesta ordem.
constexpr std::string_view kSelectColumns = "SELECT id, content, kind, pinned, copied_at FROM items";

// Ordenação padrão: fixados primeiro, depois os mais recentes.
constexpr std::string_view kOrderBy = " ORDER BY pinned DESC, copied_at DESC, id DESC";

// Conversões entre Timestamp e milissegundos (formato salvo no banco).
std::int64_t toMilliseconds(Timestamp moment) {
    using namespace std::chrono;
    return duration_cast<milliseconds>(moment.time_since_epoch()).count();
}

Timestamp fromMilliseconds(std::int64_t milliseconds) {
    return Timestamp{std::chrono::milliseconds{milliseconds}};
}

// Converte a linha atual de um comando em um ClipboardItem.
ClipboardItem readItem(const SqliteStatement& statement) {
    return ClipboardItem{
        .id = statement.columnInt64(0),
        .content = statement.columnText(1),
        .kind = static_cast<ContentKind>(statement.columnInt64(2)),
        .pinned = statement.columnInt64(3) != 0,
        .copiedAt = fromMilliseconds(statement.columnInt64(4)),
    };
}

// Escapa os curingas do LIKE (% e _) para que a busca seja literal.
std::string escapeLikePattern(std::string_view filter) {
    std::string pattern = "%";
    for (const char character : filter) {
        if (character == '%' || character == '_' || character == '\\') {
            pattern += '\\';
        }
        pattern += character;
    }
    pattern += '%';
    return pattern;
}

// Garante que o diretório do arquivo de banco exista.
void ensureParentDirectory(const std::string& databasePath) {
    if (databasePath == ":memory:" || databasePath.empty()) {
        return;
    }
    const auto parent = std::filesystem::path(databasePath).parent_path();
    if (!parent.empty()) {
        std::filesystem::create_directories(parent);
    }
}

// Pequeno auxiliar para abrir o banco já com o diretório criado.
SqliteDatabase openDatabase(const std::string& databasePath) {
    ensureParentDirectory(databasePath);
    return SqliteDatabase(databasePath);
}

} // namespace

SqliteHistoryRepository::SqliteHistoryRepository(const std::string& databasePath)
    : database_(openDatabase(databasePath)) {
    // WAL permite leituras concorrentes (CLI + daemon) com boa performance.
    database_.execute("PRAGMA journal_mode = WAL;");
    database_.execute("PRAGMA synchronous = NORMAL;");
    migrate();
}

void SqliteHistoryRepository::migrate() {
    auto versionQuery = database_.prepare("PRAGMA user_version;");
    versionQuery.step();
    const auto currentVersion = versionQuery.columnInt64(0);

    if (currentVersion < 1) {
        database_.execute(R"sql(
            BEGIN;
            CREATE TABLE IF NOT EXISTS items (
                id        INTEGER PRIMARY KEY,
                content   TEXT    NOT NULL UNIQUE,
                kind      INTEGER NOT NULL DEFAULT 0,
                pinned    INTEGER NOT NULL DEFAULT 0,
                copied_at INTEGER NOT NULL
            );
            CREATE INDEX IF NOT EXISTS idx_items_order ON items (pinned DESC, copied_at DESC);
            PRAGMA user_version = 1;
            COMMIT;
        )sql");
    }
    // Futuras migrações: `if (currentVersion < 2) { ... }`
    static_assert(kSchemaVersion == 1, "Adicione a migração para a nova versão do esquema");
}

ClipboardItem SqliteHistoryRepository::upsert(std::string_view content, ContentKind kind,
                                              Timestamp copiedAt) {
    // UPSERT: insere ou, se o conteúdo já existir, atualiza apenas a data.
    // RETURNING devolve a linha final em um único acesso ao banco.
    auto statement = database_.prepare(R"sql(
        INSERT INTO items (content, kind, pinned, copied_at) VALUES (?1, ?2, 0, ?3)
        ON CONFLICT (content) DO UPDATE SET copied_at = excluded.copied_at
        RETURNING id, content, kind, pinned, copied_at;
    )sql");
    statement.bind(1, content)
        .bind(2, static_cast<std::int64_t>(kind))
        .bind(3, toMilliseconds(copiedAt));

    if (!statement.step()) {
        throw StorageError("SQLite: UPSERT não retornou linha");
    }
    auto item = readItem(statement);
    statement.run(); // finaliza o comando
    return item;
}

std::vector<ClipboardItem> SqliteHistoryRepository::list(std::string_view filter) const {
    std::vector<ClipboardItem> items;

    if (filter.empty()) {
        auto statement = database_.prepare(std::string(kSelectColumns) + std::string(kOrderBy));
        while (statement.step()) {
            items.push_back(readItem(statement));
        }
        return items;
    }

    // LIKE no SQLite não diferencia maiúsculas/minúsculas para ASCII.
    auto statement = database_.prepare(std::string(kSelectColumns) +
                                       " WHERE content LIKE ?1 ESCAPE '\\'" +
                                       std::string(kOrderBy));
    statement.bind(1, escapeLikePattern(filter));
    while (statement.step()) {
        items.push_back(readItem(statement));
    }
    return items;
}

std::optional<ClipboardItem> SqliteHistoryRepository::findById(core::ItemId id) const {
    auto statement = database_.prepare(std::string(kSelectColumns) + " WHERE id = ?1");
    statement.bind(1, id);
    if (!statement.step()) {
        return std::nullopt;
    }
    return readItem(statement);
}

bool SqliteHistoryRepository::setPinned(core::ItemId id, bool pinned) {
    auto statement = database_.prepare("UPDATE items SET pinned = ?1 WHERE id = ?2");
    statement.bind(1, std::int64_t{pinned ? 1 : 0}).bind(2, id);
    statement.run();
    return database_.changes() > 0;
}

bool SqliteHistoryRepository::remove(core::ItemId id) {
    auto statement = database_.prepare("DELETE FROM items WHERE id = ?1");
    statement.bind(1, id);
    statement.run();
    return database_.changes() > 0;
}

std::size_t SqliteHistoryRepository::clearUnpinned() {
    database_.prepare("DELETE FROM items WHERE pinned = 0").run();
    return database_.changes();
}

std::size_t SqliteHistoryRepository::pruneUnpinned(std::size_t maxUnpinned) {
    // Apaga os não fixados que estão além da posição `maxUnpinned` na ordem
    // do mais recente para o mais antigo.
    auto statement = database_.prepare(R"sql(
        DELETE FROM items WHERE id IN (
            SELECT id FROM items WHERE pinned = 0
            ORDER BY copied_at DESC, id DESC
            LIMIT -1 OFFSET ?1
        );
    )sql");
    statement.bind(1, static_cast<std::int64_t>(maxUnpinned));
    statement.run();
    return database_.changes();
}

} // namespace copynator::storage
