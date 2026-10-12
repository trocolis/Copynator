// =============================================================================
// SqliteHistoryRepositoryTest.cpp — testes da persistência em SQLite
//
// Todos os testes usam um banco em memória (":memory:"), rápido e isolado.
// =============================================================================
#include "storage/SqliteHistoryRepository.hpp"

#include <gtest/gtest.h>

using namespace copynator;
using namespace std::chrono_literals;
using core::ContentKind;

namespace {

class SqliteHistoryRepositoryTest : public ::testing::Test {
protected:
    storage::SqliteHistoryRepository repository{":memory:"};
    const core::Timestamp start = core::Timestamp{} + 1000h; // instante fixo
};

} // namespace

// Um item inserido pode ser recuperado com todos os campos.
TEST_F(SqliteHistoryRepositoryTest, UpsertInsertsNewItem) {
    const auto item = repository.upsert("olá", ContentKind::Text, start);
    EXPECT_GT(item.id, 0);
    EXPECT_EQ(item.content, "olá");
    EXPECT_FALSE(item.pinned);
    EXPECT_EQ(item.copiedAt, start);

    const auto found = repository.findById(item.id);
    ASSERT_TRUE(found.has_value());
    EXPECT_EQ(*found, item);
}

// Copiar o mesmo texto novamente não duplica: só atualiza a data.
TEST_F(SqliteHistoryRepositoryTest, UpsertDeduplicatesAndMovesToTop) {
    const auto first = repository.upsert("a", ContentKind::Text, start);
    repository.upsert("b", ContentKind::Text, start + 1s);
    const auto again = repository.upsert("a", ContentKind::Text, start + 2s);

    EXPECT_EQ(again.id, first.id);
    const auto items = repository.list("");
    ASSERT_EQ(items.size(), 2U);
    EXPECT_EQ(items[0].content, "a"); // voltou ao topo
    EXPECT_EQ(items[1].content, "b");
}

// Itens fixados aparecem antes dos demais, independentemente da data.
TEST_F(SqliteHistoryRepositoryTest, PinnedItemsComeFirst) {
    const auto old = repository.upsert("antigo", ContentKind::Text, start);
    repository.upsert("novo", ContentKind::Text, start + 1h);
    ASSERT_TRUE(repository.setPinned(old.id, true));

    const auto items = repository.list("");
    ASSERT_EQ(items.size(), 2U);
    EXPECT_EQ(items[0].content, "antigo");
    EXPECT_TRUE(items[0].pinned);
}

// A busca filtra por trecho, sem diferenciar maiúsculas e tratando % e _
// como caracteres literais.
TEST_F(SqliteHistoryRepositoryTest, ListFiltersByContent) {
    repository.upsert("Relatório Mensal", ContentKind::Text, start);
    repository.upsert("100% pronto", ContentKind::Text, start);
    repository.upsert("outro", ContentKind::Text, start);

    EXPECT_EQ(repository.list("mensal").size(), 1U);
    EXPECT_EQ(repository.list("%").size(), 1U);
    EXPECT_EQ(repository.list("_").size(), 0U);
    EXPECT_EQ(repository.list("xyz").size(), 0U);
}

// A limpeza remove apenas itens não fixados.
TEST_F(SqliteHistoryRepositoryTest, ClearUnpinnedKeepsPinned) {
    const auto keep = repository.upsert("fixo", ContentKind::Text, start);
    repository.upsert("x", ContentKind::Text, start);
    repository.upsert("y", ContentKind::Text, start);
    repository.setPinned(keep.id, true);

    EXPECT_EQ(repository.clearUnpinned(), 2U);
    const auto items = repository.list("");
    ASSERT_EQ(items.size(), 1U);
    EXPECT_EQ(items[0].id, keep.id);
}

// O limite remove os não fixados mais antigos e preserva os fixados.
TEST_F(SqliteHistoryRepositoryTest, PruneKeepsNewestUnpinned) {
    const auto pinned = repository.upsert("fixo", ContentKind::Text, start);
    repository.setPinned(pinned.id, true);
    for (int i = 0; i < 5; ++i) {
        repository.upsert("item " + std::to_string(i), ContentKind::Text, start + std::chrono::seconds{i});
    }

    EXPECT_EQ(repository.pruneUnpinned(2), 3U);
    const auto items = repository.list("");
    ASSERT_EQ(items.size(), 3U);
    EXPECT_EQ(items[0].content, "fixo");
    EXPECT_EQ(items[1].content, "item 4");
    EXPECT_EQ(items[2].content, "item 3");
}

// Operações em ids inexistentes retornam false.
TEST_F(SqliteHistoryRepositoryTest, MissingIdsReturnFalse) {
    EXPECT_FALSE(repository.setPinned(999, true));
    EXPECT_FALSE(repository.remove(999));
    EXPECT_FALSE(repository.findById(999).has_value());
}
