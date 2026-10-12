// =============================================================================
// ContentClassifierTest.cpp — testes da detecção de tipo de conteúdo
// =============================================================================
#include "core/ContentClassifier.hpp"

#include <gtest/gtest.h>

using namespace copynator::core;

// URLs simples são reconhecidas como link, mesmo com espaços nas pontas.
TEST(ContentClassifierTest, DetectsLinks) {
    EXPECT_EQ(classifyContent("https://www.gnome.org"), ContentKind::Link);
    EXPECT_EQ(classifyContent("  http://exemplo.com/a?b=c \n"), ContentKind::Link);
}

// Uma URL no meio de uma frase não é um link "puro".
TEST(ContentClassifierTest, SentenceWithUrlIsNotLink) {
    EXPECT_NE(classifyContent("veja https://gnome.org agora"), ContentKind::Link);
}

// Trechos típicos de código em linguagens diferentes.
TEST(ContentClassifierTest, DetectsCode) {
    EXPECT_EQ(classifyContent("#include <iostream>\nint main() {\n    return 0;\n}"),
              ContentKind::Code);
    EXPECT_EQ(classifyContent("def soma(a, b):\n    return a + b"), ContentKind::Code);
    EXPECT_EQ(classifyContent("const x = () => { return 1; };"), ContentKind::Code);
}

// Frases comuns em português continuam como texto.
TEST(ContentClassifierTest, PlainTextStaysText) {
    EXPECT_EQ(classifyContent("Olá, tudo bem? Vamos almoçar amanhã."), ContentKind::Text);
    EXPECT_EQ(classifyContent("Lista de compras: arroz, feijão e café"), ContentKind::Text);
    EXPECT_EQ(classifyContent(""), ContentKind::Text);
}
