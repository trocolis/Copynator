// =============================================================================
// HistoryService.hpp
//
// Serviço de aplicação do histórico. É a ÚNICA porta de entrada usada pela
// interface, pelo D-Bus e pelos monitores de área de transferência.
//
// Responsabilidades:
//   - aplicar as regras de negócio (ignorar textos vazios, classificar o
//     conteúdo, limitar o tamanho do histórico, evitar itens gigantes);
//   - delegar a persistência ao IHistoryRepository;
//   - avisar os interessados (observadores) sempre que o histórico mudar.
// =============================================================================
#pragma once

#include "core/ClipboardItem.hpp"
#include "core/IHistoryRepository.hpp"

#include <cstddef>
#include <functional>
#include <memory>
#include <optional>
#include <string_view>
#include <vector>

namespace copynator::core {

// Configurações das regras do histórico.
struct HistorySettings {
    std::size_t maxUnpinnedItems = 200;      // limite de itens não fixados
    std::size_t maxContentBytes = 1024 * 1024; // ignora cópias maiores que 1 MiB
};

class HistoryService {
public:
    // Função chamada sempre que o histórico mudar.
    using ChangeListener = std::function<void()>;

    // Função que fornece o horário atual (injetável para testes).
    using ClockFunction = std::function<Timestamp()>;

    HistoryService(std::unique_ptr<IHistoryRepository> repository, HistorySettings settings = {},
                   ClockFunction clock = [] { return Clock::now(); });

    // Registra um texto copiado. Retorna o item salvo, ou vazio se o texto foi
    // ignorado pelas regras (vazio, só espaços ou grande demais).
    std::optional<ClipboardItem> recordCopy(std::string_view content);

    // Consulta o histórico (ver IHistoryRepository::list).
    [[nodiscard]] std::vector<ClipboardItem> items(std::string_view filter = {}) const;
    [[nodiscard]] std::optional<ClipboardItem> item(ItemId id) const;

    // Alterna o estado "fixado" de um item. Retorna o novo estado.
    std::optional<bool> togglePinned(ItemId id);

    // Remove um item.
    bool removeItem(ItemId id);

    // Limpa os itens não fixados. Retorna quantos foram removidos.
    std::size_t clearUnpinned();

    // Registra um observador de mudanças.
    void addChangeListener(ChangeListener listener);

private:
    void notifyChanged() const;

    std::unique_ptr<IHistoryRepository> repository_;
    HistorySettings settings_;
    ClockFunction clock_;
    std::vector<ChangeListener> listeners_;
};

} // namespace copynator::core
