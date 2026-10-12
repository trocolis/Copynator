// =============================================================================
// GnomeShortcutTest.cpp — testes da busca de conflitos em extensões
//
// Cria uma extensão falsa em um diretório temporário, com um esquema
// GSettings compilado que define um atalho, e verifica a detecção.
// =============================================================================
#include "platform/GnomeShortcut.hpp"

#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <gtest/gtest.h>
#include <unistd.h>

using copynator::platform::GnomeShortcut;

namespace {

class GnomeShortcutTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Backend em memória: os testes nunca tocam nas configurações reais.
        setenv("GSETTINGS_BACKEND", "memory", 1);

        root = std::filesystem::temp_directory_path() /
               ("copynator-ext-" + std::to_string(::getpid()));
        const auto schemaDir = root / "fake@test" / "schemas";
        std::filesystem::create_directories(schemaDir);

        std::ofstream(schemaDir / "org.test.fake.gschema.xml") << R"xml(
<schemalist>
  <schema id="org.test.fake" path="/org/test/fake/">
    <key name="open-shortcut" type="as"><default>['&lt;Super&gt;&lt;Shift&gt;V']</default></key>
    <key name="other-shortcut" type="as"><default>['&lt;Control&gt;e']</default></key>
    <key name="behavior" type="s"><default>'toggle'</default></key>
  </schema>
</schemalist>)xml";
        const auto command = "glib-compile-schemas " + schemaDir.string();
        ASSERT_EQ(std::system(command.c_str()), 0);
    }

    void TearDown() override { std::filesystem::remove_all(root); }

    std::filesystem::path root;
};

} // namespace

// Extensão habilitada com o mesmo atalho (grafia diferente) é um conflito.
TEST_F(GnomeShortcutTest, DetectsConflictInEnabledExtension) {
    const auto conflicts =
        GnomeShortcut::findExtensionConflicts("<Shift><Super>v", {root}, {"fake@test"});
    ASSERT_EQ(conflicts.size(), 1U);
    EXPECT_NE(conflicts[0].find("fake@test"), std::string::npos);
    EXPECT_NE(conflicts[0].find("open-shortcut"), std::string::npos);
}

// Extensões desabilitadas não contam.
TEST_F(GnomeShortcutTest, IgnoresDisabledExtensions) {
    EXPECT_TRUE(GnomeShortcut::findExtensionConflicts("<Shift><Super>v", {root}, {}).empty());
}

// Atalhos diferentes não geram conflito.
TEST_F(GnomeShortcutTest, NoConflictForDifferentBinding) {
    EXPECT_TRUE(
        GnomeShortcut::findExtensionConflicts("<Super>c", {root}, {"fake@test"}).empty());
}
