// =============================================================================
// AcceleratorTest.cpp — testes da normalização de atalhos
// =============================================================================
#include "core/Accelerator.hpp"

#include <gtest/gtest.h>

using namespace copynator::core;

// A ordem e a caixa dos modificadores não importam.
TEST(AcceleratorTest, OrderAndCaseAreIgnored) {
    EXPECT_EQ(normalizeAccelerator("<Super><Shift>V"), "<shift><super>v");
    EXPECT_EQ(normalizeAccelerator("<shift><super>v"), "<shift><super>v");
}

// Sinônimos do GTK são unificados.
TEST(AcceleratorTest, SynonymsAreUnified) {
    EXPECT_TRUE(sameAccelerator("<Mod4><Shift>v", "<Shift><Super>v"));
    EXPECT_TRUE(sameAccelerator("<Primary>c", "<Control>c"));
    EXPECT_TRUE(sameAccelerator("<Ctrl><Alt>t", "<Mod1><Control>t"));
}

// Atalhos diferentes continuam diferentes.
TEST(AcceleratorTest, DifferentAcceleratorsDiffer) {
    EXPECT_FALSE(sameAccelerator("<Super>v", "<Shift><Super>v"));
    EXPECT_FALSE(sameAccelerator("<Shift><Super>c", "<Shift><Super>v"));
}

// Atalhos vazios ou desativados nunca são iguais a nada.
TEST(AcceleratorTest, EmptyOrDisabledNeverMatch) {
    EXPECT_EQ(normalizeAccelerator(""), "");
    EXPECT_EQ(normalizeAccelerator("disabled"), "");
    EXPECT_FALSE(sameAccelerator("", ""));
}
