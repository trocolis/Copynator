// =============================================================================
// PlacementTest.cpp — testes da conversão de posicionamento (texto do D-Bus)
// =============================================================================
#include "core/Placement.hpp"

#include <gtest/gtest.h>

using namespace copynator::core;

// A conversão ida e volta preserva o valor.
TEST(PlacementTest, RoundTrip) {
    EXPECT_EQ(parsePlacement(toString(Placement::AtCursor)), Placement::AtCursor);
    EXPECT_EQ(parsePlacement(toString(Placement::Centered)), Placement::Centered);
}

// Textos desconhecidos são rejeitados.
TEST(PlacementTest, RejectsUnknownValues) {
    EXPECT_FALSE(parsePlacement("").has_value());
    EXPECT_FALSE(parsePlacement("Center").has_value());
    EXPECT_FALSE(parsePlacement("top").has_value());
}
