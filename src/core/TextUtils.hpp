// =============================================================================
// TextUtils.hpp
//
// Funções utilitárias de texto com suporte a UTF-8, usadas pelo domínio e pela
// interface (contagem de caracteres, prévia de uma linha, remoção de espaços).
// =============================================================================
#pragma once

#include <cstddef>
#include <string>
#include <string_view>

namespace copynator::core {

// Remove espaços em branco (espaço, tab, quebras de linha) do início e do fim.
[[nodiscard]] std::string_view trim(std::string_view text);

// Conta caracteres (code points UTF-8), e não bytes. "ação" tem 4 caracteres.
[[nodiscard]] std::size_t countCharacters(std::string_view text);

// Gera uma prévia de uma única linha:
//   - sequências de espaços/quebras de linha viram um único espaço
//   - o resultado é cortado em `maxCharacters` caracteres, com "…" no final
//   - nunca corta um caractere UTF-8 ao meio
[[nodiscard]] std::string makePreview(std::string_view text, std::size_t maxCharacters);

} // namespace copynator::core
