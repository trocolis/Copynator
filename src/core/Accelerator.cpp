// =============================================================================
// Accelerator.cpp — normalização de atalhos de teclado
// =============================================================================
#include "core/Accelerator.hpp"

#include <algorithm>
#include <array>
#include <cctype>
#include <set>

namespace copynator::core {

namespace {

// Converte para minúsculas (atalhos usam apenas ASCII).
std::string toLower(std::string_view text) {
    std::string lower(text);
    std::ranges::transform(lower, lower.begin(),
                           [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return lower;
}

// Unifica sinônimos de modificadores usados pelo GTK.
std::string canonicalModifier(std::string_view modifier) {
    const auto lower = toLower(modifier);
    if (lower == "ctrl" || lower == "primary" || lower == "control") {
        return "control";
    }
    if (lower == "mod4" || lower == "super") {
        return "super";
    }
    if (lower == "mod1" || lower == "alt") {
        return "alt";
    }
    return lower; // shift, meta, hyper...
}

// Ordem fixa dos modificadores na forma canônica.
constexpr std::array<std::string_view, 6> kModifierOrder{"control", "shift", "alt",
                                                         "super",   "meta",  "hyper"};

} // namespace

std::string normalizeAccelerator(std::string_view accelerator) {
    std::set<std::string> modifiers;
    std::string_view rest = accelerator;

    // Extrai todos os "<Modificador>" do início.
    while (rest.starts_with('<')) {
        const auto close = rest.find('>');
        if (close == std::string_view::npos) {
            break; // formato inválido: trata o restante como tecla
        }
        modifiers.insert(canonicalModifier(rest.substr(1, close - 1)));
        rest.remove_prefix(close + 1);
    }

    const auto key = toLower(rest);
    if (key.empty() || key == "disabled") {
        return {};
    }

    std::string canonical;
    for (const auto modifier : kModifierOrder) {
        if (modifiers.erase(std::string(modifier)) > 0) {
            canonical += "<" + std::string(modifier) + ">";
        }
    }
    // Modificadores desconhecidos vão no fim, em ordem alfabética (std::set).
    for (const auto& modifier : modifiers) {
        canonical += "<" + modifier + ">";
    }
    return canonical + key;
}

bool sameAccelerator(std::string_view first, std::string_view second) {
    const auto normalizedFirst = normalizeAccelerator(first);
    return !normalizedFirst.empty() && normalizedFirst == normalizeAccelerator(second);
}

} // namespace copynator::core
