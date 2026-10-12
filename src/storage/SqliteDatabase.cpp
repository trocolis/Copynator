// =============================================================================
// SqliteDatabase.cpp — invólucro RAII do SQLite
// =============================================================================
#include "storage/SqliteDatabase.hpp"

#include <format>
#include <sqlite3.h>

namespace copynator::storage {

namespace {

// Monta a mensagem de erro com o contexto e o texto retornado pelo SQLite.
[[noreturn]] void throwError(sqlite3* connection, std::string_view context) {
    const char* detail = connection != nullptr ? sqlite3_errmsg(connection) : "erro desconhecido";
    throw StorageError(std::format("SQLite: {} ({})", context, detail));
}

} // namespace

// -----------------------------------------------------------------------------
// SqliteDatabase
// -----------------------------------------------------------------------------

void SqliteDatabase::ConnectionCloser::operator()(sqlite3* connection) const noexcept {
    sqlite3_close_v2(connection);
}

SqliteDatabase::SqliteDatabase(const std::string& path) {
    sqlite3* rawConnection = nullptr;
    const int flags = SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | SQLITE_OPEN_FULLMUTEX;
    const int result = sqlite3_open_v2(path.c_str(), &rawConnection, flags, nullptr);

    // Mesmo em caso de erro o SQLite pode alocar a conexão; o unique_ptr
    // garante que ela será fechada.
    connection_.reset(rawConnection);
    if (result != SQLITE_OK) {
        throwError(rawConnection, std::format("não foi possível abrir '{}'", path));
    }

    // Aguarda até 2s se outro processo estiver escrevendo (ex.: `--clear`
    // executado enquanto o daemon está rodando).
    sqlite3_busy_timeout(rawConnection, 2000);
}

void SqliteDatabase::execute(std::string_view sql) {
    char* errorMessage = nullptr;
    const std::string sqlText(sql);
    if (sqlite3_exec(connection_.get(), sqlText.c_str(), nullptr, nullptr, &errorMessage) !=
        SQLITE_OK) {
        const std::string detail = errorMessage != nullptr ? errorMessage : "";
        sqlite3_free(errorMessage);
        throw StorageError(std::format("SQLite: falha ao executar comando ({})", detail));
    }
}

SqliteStatement SqliteDatabase::prepare(std::string_view sql) {
    return SqliteStatement(connection_.get(), sql);
}

std::size_t SqliteDatabase::changes() const {
    return static_cast<std::size_t>(sqlite3_changes(connection_.get()));
}

// -----------------------------------------------------------------------------
// SqliteStatement
// -----------------------------------------------------------------------------

void SqliteStatement::StatementFinalizer::operator()(sqlite3_stmt* statement) const noexcept {
    sqlite3_finalize(statement);
}

SqliteStatement::SqliteStatement(sqlite3* connection, std::string_view sql)
    : connection_(connection) {
    sqlite3_stmt* rawStatement = nullptr;
    if (sqlite3_prepare_v2(connection, sql.data(), static_cast<int>(sql.size()), &rawStatement,
                           nullptr) != SQLITE_OK) {
        throwError(connection, "falha ao preparar comando");
    }
    statement_.reset(rawStatement);
}

SqliteStatement& SqliteStatement::bind(int index, std::int64_t value) {
    if (sqlite3_bind_int64(statement_.get(), index, value) != SQLITE_OK) {
        throwError(connection_, "falha no bind de inteiro");
    }
    return *this;
}

SqliteStatement& SqliteStatement::bind(int index, std::string_view value) {
    // SQLITE_TRANSIENT faz o SQLite copiar o texto, pois o string_view pode
    // deixar de existir antes da execução do comando.
    if (sqlite3_bind_text(statement_.get(), index, value.data(), static_cast<int>(value.size()),
                          SQLITE_TRANSIENT) != SQLITE_OK) {
        throwError(connection_, "falha no bind de texto");
    }
    return *this;
}

bool SqliteStatement::step() {
    const int result = sqlite3_step(statement_.get());
    if (result == SQLITE_ROW) {
        return true;
    }
    if (result == SQLITE_DONE) {
        return false;
    }
    throwError(connection_, "falha ao executar comando");
}

void SqliteStatement::run() {
    while (step()) {
        // Descarta eventuais linhas retornadas.
    }
}

std::int64_t SqliteStatement::columnInt64(int index) const {
    return sqlite3_column_int64(statement_.get(), index);
}

std::string SqliteStatement::columnText(int index) const {
    // Lê os bytes com o tamanho exato (o texto pode conter '\0' no meio).
    const auto* text = sqlite3_column_text(statement_.get(), index);
    const auto size = sqlite3_column_bytes(statement_.get(), index);
    if (text == nullptr) {
        return {};
    }
    return {reinterpret_cast<const char*>(text), static_cast<std::size_t>(size)};
}

} // namespace copynator::storage
