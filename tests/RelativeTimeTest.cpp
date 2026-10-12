// =============================================================================
// RelativeTimeTest.cpp — testes da formatação de tempo relativo
// =============================================================================
#include "core/RelativeTime.hpp"

#include <gtest/gtest.h>

using namespace copynator::core;
using namespace std::chrono_literals;

namespace {

// Momento fixo usado como "agora" em todos os testes.
const Timestamp kNow = Clock::now();

} // namespace

TEST(RelativeTimeTest, LessThanAMinuteIsNow) {
    EXPECT_EQ(formatRelativeTime(kNow, kNow), "Agora");
    EXPECT_EQ(formatRelativeTime(kNow - 59s, kNow), "Agora");
}

TEST(RelativeTimeTest, FutureIsNow) {
    EXPECT_EQ(formatRelativeTime(kNow + 1h, kNow), "Agora");
}

TEST(RelativeTimeTest, Minutes) {
    EXPECT_EQ(formatRelativeTime(kNow - 5min, kNow), "há 5 min");
    EXPECT_EQ(formatRelativeTime(kNow - 59min, kNow), "há 59 min");
}

TEST(RelativeTimeTest, Hours) {
    EXPECT_EQ(formatRelativeTime(kNow - 2h, kNow), "há 2 h");
}

TEST(RelativeTimeTest, Yesterday) {
    EXPECT_EQ(formatRelativeTime(kNow - 30h, kNow), "ontem");
}

TEST(RelativeTimeTest, Days) {
    EXPECT_EQ(formatRelativeTime(kNow - 72h, kNow), "há 3 dias");
}

// Itens com mais de uma semana mostram a data no formato dd/mm/aaaa.
TEST(RelativeTimeTest, OldItemsShowDate) {
    const auto text = formatRelativeTime(kNow - 24h * 30, kNow);
    ASSERT_EQ(text.size(), 10U);
    EXPECT_EQ(text[2], '/');
    EXPECT_EQ(text[5], '/');
}
