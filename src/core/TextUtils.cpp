// =============================================================================
// TextUtils.cpp — utilitários de texto UTF-8
// =============================================================================
#include "core/TextUtils.hpp"

#include <algorithm>

namespace copynator::core {

namespace {

// Caracteres considerados "espaço em branco" para fins de trim/prévia.
constexpr std::string_view kWhitespace = " \t\r\n\v\f";

bool isWhitespace(char character) {
    return kWhitespace.find(character) != std::string_view::npos;
}

// Em UTF-8, bytes de continuação têm o formato 10xxxxxx. Todo byte que NÃO é
// de continuação marca o início de um novo caractere (code point).
bool isUtf8LeadByte(char byte) {
    return (static_cast<unsigned char>(byte) & 0xC0U) != 0x80U;
}

} // namespace

std::string_view trim(std::string_view text) {
    const auto first = text.find_first_not_of(kWhitespace);
    if (first == std::string_view::npos) {
        return {};
    }
    const auto last = text.find_last_not_of(kWhitespace);
    return text.substr(first, last - first + 1);
}

std::size_t countCharacters(std::string_view text) {
    std::size_t count = 0;
    for (const char byte : text) {
        if (isUtf8LeadByte(byte)) {
            ++count;
        }
    }
    return count;
}

std::string makePreview(std::string_view text, std::size_t maxCharacters) {
    std::string preview;
    preview.reserve(std::min(text.size(), maxCharacters * 4));

    std::size_t characters = 0;
    bool pendingSpace = false; // há espaço acumulado a ser emitido?

    for (const char byte : trim(text)) {
        // Colapsa qualquer sequência de espaços em um único espaço.
        if (isWhitespace(byte)) {
            pendingSpace = true;
            continue;
        }

        if (isUtf8LeadByte(byte)) {
            // Contabiliza o espaço pendente como um caractere.
            const std::size_t needed = pendingSpace ? 2 : 1;
            if (characters + needed > maxCharacters) {
                preview += "…";
                return preview;
            }
            if (pendingSpace) {
                preview += ' ';
                ++characters;
                pendingSpace = false;
            }
            ++characters;
        }
        preview += byte;
    }
    return preview;
}

} // namespace copynator::core
