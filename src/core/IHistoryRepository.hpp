// =============================================================================
// IHistoryRepository.hpp
//
// "Porta" de persistência do domínio (padrão Repository + inversão de
// dependência). O core define O QUE precisa ser armazenado; a camada `storage`
// decide COMO (SQLite). Isso permite trocar a implementação ou usar dublês em
// testes sem alterar as regras de negócio.
// =============================================================================
#pragma once

#include "core/ClipboardItem.hpp"

#include <cstddef>
#include <optional>
#include <string_view>
#include <vector>

namespace copynator::core {

class IHistoryRepository {
public:
    virtual ~IHistoryRepository() = default;

    // Insere o conteúdo ou, se já existir um item idêntico, apenas atualiza a
    // data (`copiedAt`), movendo-o para o topo. Retorna o item resultante.
    virtual ClipboardItem upsert(std::string_view content, ContentKind kind,
                                 Timestamp copiedAt) = 0;

    // Lista os itens: fixados primeiro e, dentro de cada grupo, os mais
    // recentes primeiro. `filter` vazio retorna tudo; caso contrário retorna
    // apenas itens cujo conteúdo contém o texto (sem diferenciar maiúsculas).
    [[nodiscard]] virtual std::vector<ClipboardItem> list(std::string_view filter) const = 0;

    // Busca um item pelo identificador.
    [[nodiscard]] virtual std::optional<ClipboardItem> findById(ItemId id) const = 0;

    // Fixa/desafixa um item. Retorna false se o item não existir.
    virtual bool setPinned(ItemId id, bool pinned) = 0;

    // Remove um item específico. Retorna false se o item não existir.
    virtual bool remove(ItemId id) = 0;

    // Remove todos os itens não fixados. Retorna quantos foram removidos.
    virtual std::size_t clearUnpinned() = 0;

    // Mantém no máximo `maxUnpinned` itens não fixados, apagando os mais
    // antigos. Retorna quantos foram removidos.
    virtual std::size_t pruneUnpinned(std::size_t maxUnpinned) = 0;
};

} // namespace copynator::core
