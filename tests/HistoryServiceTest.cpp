// =============================================================================
// HistoryServiceTest.cpp — testes das regras de negócio do histórico
//
// Usa o repositório SQLite em memória (rápido e realista) e um relógio
// controlado pelo teste.
// =============================================================================
#include "core/HistoryService.hpp"
#include "storage/SqliteHistoryRepository.hpp"

#include <gtest/gtest.h>

using namespace copynator;
using namespace std::chrono_literals;

namespace {

class HistoryServiceTest : public ::testing::Test {
protected:
    // Cria o serviço com configurações específicas para cada teste.
    core::HistoryService makeService(core::HistorySettings settings = {}) {
        return core::HistoryService(
            std::make_unique<storage::SqliteHistoryRepository>(":memory:"), settings,
            [this] { return now; });
    }

    core::Timestamp now = core::Timestamp{} + 1000h;
};

} // namespace

// Textos vazios ou só com espaços são ignorados.
TEST_F(HistoryServiceTest, IgnoresBlankContent) {
    auto service = makeService();
    EXPECT_FALSE(service.recordCopy("").has_value());
    EXPECT_FALSE(service.recordCopy("  \n\t ").has_value());
    EXPECT_TRUE(service.items().empty());
}

// Conteúdos acima do limite de bytes são ignorados.
TEST_F(HistoryServiceTest, IgnoresOversizedContent) {
    auto service = makeService({.maxUnpinnedItems = 10, .maxContentBytes = 4});
    EXPECT_FALSE(service.recordCopy("12345").has_value());
    EXPECT_TRUE(service.recordCopy("1234").has_value());
}

// O tipo do conteúdo é classificado ao registrar.
TEST_F(HistoryServiceTest, ClassifiesContent) {
    auto service = makeService();
    const auto item = service.recordCopy("https://gnome.org");
    ASSERT_TRUE(item.has_value());
    EXPECT_EQ(item->kind, core::ContentKind::Link);
}

// O histórico respeita o limite configurado.
TEST_F(HistoryServiceTest, EnforcesHistoryLimit) {
    auto service = makeService({.maxUnpinnedItems = 3});
    for (int i = 0; i < 10; ++i) {
        now += 1s;
        service.recordCopy("texto " + std::to_string(i));
    }
    EXPECT_EQ(service.items().size(), 3U);
}

// Alternar a fixação inverte o estado e avisa os observadores.
TEST_F(HistoryServiceTest, TogglePinnedNotifiesListeners) {
    auto service = makeService();
    int notifications = 0;
    service.addChangeListener([&] { ++notifications; });

    const auto item = service.recordCopy("abc");
    ASSERT_TRUE(item.has_value());
    EXPECT_EQ(service.togglePinned(item->id), true);
    EXPECT_EQ(service.togglePinned(item->id), false);
    EXPECT_FALSE(service.togglePinned(12345).has_value());
    EXPECT_EQ(notifications, 3); // 1 cópia + 2 alternâncias
}

// A limpeza remove só os não fixados e notifica apenas quando algo mudou.
TEST_F(HistoryServiceTest, ClearUnpinned) {
    auto service = makeService();
    const auto pinned = service.recordCopy("fixo");
    service.recordCopy("solto");
    service.togglePinned(pinned->id);

    int notifications = 0;
    service.addChangeListener([&] { ++notifications; });
    EXPECT_EQ(service.clearUnpinned(), 1U);
    EXPECT_EQ(service.clearUnpinned(), 0U);
    EXPECT_EQ(notifications, 1);
    EXPECT_EQ(service.items().size(), 1U);
}
