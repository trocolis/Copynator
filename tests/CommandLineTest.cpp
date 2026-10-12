// =============================================================================
// CommandLineTest.cpp — testes do interpretador de linha de comando
// =============================================================================
#include "core/CommandLine.hpp"

#include <gtest/gtest.h>
#include <string_view>
#include <vector>

using copynator::core::CommandAction;
using copynator::core::parseCommandLine;

namespace {

// Atalho para chamar o parser com uma lista literal de argumentos.
auto parse(std::vector<std::string_view> arguments) {
    return parseCommandLine(arguments);
}

} // namespace

// Sem argumentos o comportamento padrão é abrir na posição do cursor.
TEST(CommandLineTest, NoArgumentsShowsAtCursor) {
    const auto result = parse({});
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->action, CommandAction::ShowAtCursor);
}

// Cada flag longa deve mapear para a ação correspondente.
TEST(CommandLineTest, LongFlagsMapToActions) {
    EXPECT_EQ(parse({"--center"})->action, CommandAction::ShowCentered);
    EXPECT_EQ(parse({"--background"})->action, CommandAction::RunBackground);
    EXPECT_EQ(parse({"--clear"})->action, CommandAction::ClearHistory);
    EXPECT_EQ(parse({"--status"})->action, CommandAction::ShowStatus);
    EXPECT_EQ(parse({"--setup-shortcut"})->action, CommandAction::SetupShortcut);
    EXPECT_EQ(parse({"--help"})->action, CommandAction::PrintHelp);
    EXPECT_EQ(parse({"--version"})->action, CommandAction::PrintVersion);
}

// As formas curtas são equivalentes às longas.
TEST(CommandLineTest, ShortFlagsMapToActions) {
    EXPECT_EQ(parse({"-c"})->action, CommandAction::ShowCentered);
    EXPECT_EQ(parse({"-b"})->action, CommandAction::RunBackground);
    EXPECT_EQ(parse({"-s"})->action, CommandAction::ShowStatus);
    EXPECT_EQ(parse({"-h"})->action, CommandAction::PrintHelp);
    EXPECT_EQ(parse({"-v"})->action, CommandAction::PrintVersion);
}

// Opções desconhecidas geram erro com o nome da opção na mensagem.
TEST(CommandLineTest, UnknownFlagIsRejected) {
    const auto result = parse({"--banana"});
    ASSERT_FALSE(result.has_value());
    EXPECT_NE(result.error().find("--banana"), std::string::npos);
}

// Duas ações ao mesmo tempo são ambíguas e devem ser rejeitadas.
TEST(CommandLineTest, MultipleActionsAreRejected) {
    EXPECT_FALSE(parse({"--center", "--clear"}).has_value());
}
