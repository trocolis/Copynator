// =============================================================================
// ContentClassifier.hpp
//
// Detecta automaticamente o tipo de um texto copiado (link, código ou texto)
// para que a interface possa exibir a etiqueta (badge) adequada.
// =============================================================================
#pragma once

#include "core/ClipboardItem.hpp"

#include <string_view>

namespace copynator::core {

// Classifica o conteúdo. A ordem de verificação é: Link -> Code -> Text.
[[nodiscard]] ContentKind classifyContent(std::string_view content);

// Verifica se o texto (já sem espaços nas pontas) é uma única URL http/https.
[[nodiscard]] bool looksLikeLink(std::string_view content);

// Heurística simples para identificar trechos de código-fonte.
[[nodiscard]] bool looksLikeCode(std::string_view content);

} // namespace copynator::core
