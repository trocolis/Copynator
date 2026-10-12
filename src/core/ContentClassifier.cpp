// =============================================================================
// ContentClassifier.cpp — heurísticas de classificação de conteúdo
// =============================================================================
#include "core/ContentClassifier.hpp"

#include "core/TextUtils.hpp"

#include <algorithm>
#include <array>
#include <ranges>

namespace copynator::core {

namespace {

// Trechos que, quando presentes, indicam fortemente que o texto é código.
constexpr std::array<std::string_view, 24> kCodeMarkers{
    "#include", "#!/",      "#define",   "import ",  "from ",     "def ",
    "class ",   "function", "return ",   "const ",   "let ",      "var ",
    "public ",  "private ", "fn ",       "func ",    "=>",        "->",
    "::",       "&&",       "||",        "</",       "SELECT ",   "std::",
};

// Caracteres típicos de sintaxe de linguagens de programação.
constexpr std::string_view kSyntaxCharacters = "{}()[];=<>";

bool startsWith(std::string_view text, std::string_view prefix) {
    return text.substr(0, prefix.size()) == prefix;
}

// Proporção de caracteres de sintaxe em relação ao total de caracteres
// visíveis. Textos comuns raramente passam de poucos por cento.
double syntaxDensity(std::string_view text) {
    std::size_t syntax = 0;
    std::size_t visible = 0;
    for (const char character : text) {
        if (character == ' ' || character == '\n' || character == '\t' || character == '\r') {
            continue;
        }
        ++visible;
        if (kSyntaxCharacters.find(character) != std::string_view::npos) {
            ++syntax;
        }
    }
    return visible == 0 ? 0.0 : static_cast<double>(syntax) / static_cast<double>(visible);
}

// Conta linhas que começam com indentação (espaços ou tab).
std::size_t countIndentedLines(std::string_view text) {
    std::size_t indented = 0;
    for (const auto line : text | std::views::split('\n')) {
        const std::string_view lineView(line.begin(), line.end());
        if (lineView.starts_with("  ") || lineView.starts_with('\t')) {
            ++indented;
        }
    }
    return indented;
}

} // namespace

bool looksLikeLink(std::string_view content) {
    const auto text = trim(content);
    if (!startsWith(text, "http://") && !startsWith(text, "https://")) {
        return false;
    }
    // Uma URL não contém espaços nem quebras de linha.
    return std::ranges::none_of(text, [](char c) { return c == ' ' || c == '\n' || c == '\t'; });
}

bool looksLikeCode(std::string_view content) {
    const auto text = trim(content);
    if (text.empty()) {
        return false;
    }

    // Pontuação: cada evidência soma; acima de um limite é código.
    int score = 0;

    const auto markers = std::ranges::count_if(
        kCodeMarkers, [&](std::string_view marker) { return text.find(marker) != text.npos; });
    score += static_cast<int>(markers);

    if (syntaxDensity(text) > 0.08) {
        score += 2;
    }
    if (countIndentedLines(text) >= 1 && text.find('\n') != text.npos) {
        score += 1;
    }
    // Linhas terminadas em ';' ou '{' são muito características de código.
    if (text.find(";\n") != text.npos || text.find("{\n") != text.npos || text.ends_with(';')) {
        score += 2;
    }

    return score >= 3;
}

ContentKind classifyContent(std::string_view content) {
    if (looksLikeLink(content)) {
        return ContentKind::Link;
    }
    if (looksLikeCode(content)) {
        return ContentKind::Code;
    }
    return ContentKind::Text;
}

} // namespace copynator::core
