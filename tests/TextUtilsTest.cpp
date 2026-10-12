// =============================================================================
// TextUtilsTest.cpp — testes dos utilitários de texto UTF-8
// =============================================================================
#include "core/TextUtils.hpp"

#include <gtest/gtest.h>

using namespace copynator::core;

// trim remove espaços e quebras de linha das duas pontas.
TEST(TextUtilsTest, TrimRemovesSurroundingWhitespace) {
    EXPECT_EQ(trim("  \n olá \t\n"), "olá");
    EXPECT_EQ(trim("   "), "");
    EXPECT_EQ(trim(""), "");
}

// A contagem é feita em caracteres, não em bytes.
TEST(TextUtilsTest, CountCharactersHandlesUtf8) {
    EXPECT_EQ(countCharacters("abc"), 3U);
    EXPECT_EQ(countCharacters("ação"), 4U);
    EXPECT_EQ(countCharacters("📋"), 1U);
    EXPECT_EQ(countCharacters(""), 0U);
}

// A prévia junta linhas e colapsa espaços repetidos.
TEST(TextUtilsTest, PreviewCollapsesWhitespace) {
    EXPECT_EQ(makePreview("linha 1\n\n   linha 2", 100), "linha 1 linha 2");
}

// A prévia é truncada com reticências sem quebrar caracteres UTF-8.
TEST(TextUtilsTest, PreviewTruncatesOnCharacterBoundary) {
    EXPECT_EQ(makePreview("ãããããã", 3), "ããã…");
    EXPECT_EQ(makePreview("abc", 3), "abc");
}
