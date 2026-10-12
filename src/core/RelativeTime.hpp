// =============================================================================
// RelativeTime.hpp
//
// Formata datas de forma amigável em português: "Agora", "há 5 min",
// "há 2 h", "ontem", "há 3 dias" ou a data completa para itens antigos.
// =============================================================================
#pragma once

#include "core/ClipboardItem.hpp"

#include <string>

namespace copynator::core {

// Formata `moment` relativo a `now`. Recebe `now` como parâmetro (em vez de
// consultar o relógio) para que a função seja determinística e testável.
//
// Regras (baseadas no tempo decorrido):
//   < 1 min    -> "Agora"
//   < 60 min   -> "há N min"
//   < 24 h     -> "há N h"
//   < 48 h     -> "ontem"
//   < 7 dias   -> "há N dias"
//   senão      -> "dd/mm/aaaa" (no fuso local)
//
// Datas no futuro (relógio ajustado) são tratadas como "Agora".
[[nodiscard]] std::string formatRelativeTime(Timestamp moment, Timestamp now);

} // namespace copynator::core
