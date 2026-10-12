// =============================================================================
// ClipboardItem.hpp
//
// Entidade central do domínio: um item do histórico da área de transferência.
// =============================================================================
#pragma once

#include <chrono>
#include <cstdint>
#include <string>

namespace copynator::core {

// Tipo de conteúdo detectado automaticamente, usado para exibir badges.
enum class ContentKind {
    Text, // texto comum
    Link, // URL http/https
    Code, // trecho de código-fonte
};

// Relógio usado em todo o domínio para registrar quando um item foi copiado.
using Clock = std::chrono::system_clock;
using Timestamp = Clock::time_point;

// Identificador persistente de um item (chave primária no banco).
using ItemId = std::int64_t;

// Um item do histórico.
//
// É uma estrutura de dados simples (agregado): as regras de negócio ficam em
// funções livres e no repositório, mantendo a entidade fácil de copiar/testar.
struct ClipboardItem {
    ItemId id = 0;                      // 0 = ainda não persistido
    std::string content;                // texto copiado (UTF-8)
    ContentKind kind = ContentKind::Text;
    bool pinned = false;                // itens fixados não são apagados na limpeza
    Timestamp copiedAt{};               // última vez que o texto foi copiado

    friend bool operator==(const ClipboardItem&, const ClipboardItem&) = default;
};

} // namespace copynator::core
