// =============================================================================
// SqliteDatabase.hpp
//
// Pequeno invólucro RAII sobre a API C do SQLite:
//   - `SqliteDatabase` abre/fecha a conexão automaticamente;
//   - `SqliteStatement` prepara/finaliza comandos e faz o bind de parâmetros.
//
// Erros do SQLite são convertidos em exceções `StorageError`, para que falhas
// não passem despercebidas (ex.: disco cheio, banco corrompido).
// =============================================================================
#pragma once

#include <cstdint>
#include <memory>
#include <stdexcept>
#include <string>
#include <string_view>

struct sqlite3;
struct sqlite3_stmt;

namespace copynator::storage {

// Exceção lançada em qualquer falha do banco de dados.
class StorageError : public std::runtime_error {
public:
    using std::runtime_error::runtime_error;
};

class SqliteStatement;

// Conexão com um banco SQLite (não copiável, apenas movível).
class SqliteDatabase {
public:
    // Abre (ou cria) o banco no caminho informado. ":memory:" cria um banco em
    // memória, útil para testes.
    explicit SqliteDatabase(const std::string& path);

    // Executa um ou mais comandos SQL sem resultado (DDL, PRAGMA, etc.).
    void execute(std::string_view sql);

    // Prepara um comando para execução com parâmetros.
    [[nodiscard]] SqliteStatement prepare(std::string_view sql);

    // Quantidade de linhas afetadas pelo último INSERT/UPDATE/DELETE.
    [[nodiscard]] std::size_t changes() const;

    // Acesso ao ponteiro nativo (para uso interno do SqliteStatement).
    [[nodiscard]] sqlite3* handle() const { return connection_.get(); }

private:
    // Deleter customizado: fecha a conexão ao destruir o unique_ptr.
    struct ConnectionCloser {
        void operator()(sqlite3* connection) const noexcept;
    };
    std::unique_ptr<sqlite3, ConnectionCloser> connection_;
};

// Comando preparado (prepared statement).
class SqliteStatement {
public:
    SqliteStatement(sqlite3* connection, std::string_view sql);

    // Bind de parâmetros. Os índices começam em 1, como na API do SQLite.
    SqliteStatement& bind(int index, std::int64_t value);
    SqliteStatement& bind(int index, std::string_view value);

    // Avança para a próxima linha. Retorna false quando não há mais linhas.
    bool step();

    // Executa um comando que não retorna linhas.
    void run();

    // Leitura de colunas da linha atual (índices começam em 0).
    [[nodiscard]] std::int64_t columnInt64(int index) const;
    [[nodiscard]] std::string columnText(int index) const;

private:
    struct StatementFinalizer {
        void operator()(sqlite3_stmt* statement) const noexcept;
    };
    sqlite3* connection_;
    std::unique_ptr<sqlite3_stmt, StatementFinalizer> statement_;
};

} // namespace copynator::storage
