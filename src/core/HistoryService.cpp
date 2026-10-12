// =============================================================================
// HistoryService.cpp — regras de negócio do histórico
// =============================================================================
#include "core/HistoryService.hpp"

#include "core/ContentClassifier.hpp"
#include "core/TextUtils.hpp"

#include <utility>

namespace copynator::core {

HistoryService::HistoryService(std::unique_ptr<IHistoryRepository> repository,
                               HistorySettings settings, ClockFunction clock)
    : repository_(std::move(repository)), settings_(settings), clock_(std::move(clock)) {}

std::optional<ClipboardItem> HistoryService::recordCopy(std::string_view content) {
    // Regra 1: textos vazios ou formados só por espaços não entram no histórico.
    if (trim(content).empty()) {
        return std::nullopt;
    }
    // Regra 2: conteúdos enormes (ex.: um arquivo inteiro) são ignorados para
    // manter o banco e a interface leves.
    if (content.size() > settings_.maxContentBytes) {
        return std::nullopt;
    }

    // O texto é salvo exatamente como foi copiado (sem trim), para que colar
    // devolva o conteúdo original; a classificação usa as próprias heurísticas.
    auto saved = repository_->upsert(content, classifyContent(content), clock_());

    // Regra 3: o histórico não fixado tem tamanho limitado.
    repository_->pruneUnpinned(settings_.maxUnpinnedItems);

    notifyChanged();
    return saved;
}

std::vector<ClipboardItem> HistoryService::items(std::string_view filter) const {
    return repository_->list(trim(filter));
}

std::optional<ClipboardItem> HistoryService::item(ItemId id) const {
    return repository_->findById(id);
}

std::optional<bool> HistoryService::togglePinned(ItemId id) {
    const auto current = repository_->findById(id);
    if (!current) {
        return std::nullopt;
    }
    const bool newState = !current->pinned;
    repository_->setPinned(id, newState);

    // Ao desafixar, o item volta a contar no limite do histórico.
    if (!newState) {
        repository_->pruneUnpinned(settings_.maxUnpinnedItems);
    }
    notifyChanged();
    return newState;
}

bool HistoryService::removeItem(ItemId id) {
    const bool removed = repository_->remove(id);
    if (removed) {
        notifyChanged();
    }
    return removed;
}

std::size_t HistoryService::clearUnpinned() {
    const auto removed = repository_->clearUnpinned();
    if (removed > 0) {
        notifyChanged();
    }
    return removed;
}

void HistoryService::addChangeListener(ChangeListener listener) {
    listeners_.push_back(std::move(listener));
}

void HistoryService::notifyChanged() const {
    for (const auto& listener : listeners_) {
        listener();
    }
}

} // namespace copynator::core
