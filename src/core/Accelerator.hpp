// =============================================================================
// Accelerator.hpp
//
// Normalização de atalhos de teclado no formato do GTK/GNOME
// (ex.: "<Shift><Super>v"). Duas grafias diferentes podem representar o mesmo
// atalho — "<Super><Shift>V", "<shift><super>v", "<Mod4><Shift>v" — e
// precisamos compará-las para detectar conflitos.
// =============================================================================
#pragma once

#include <string>
#include <string_view>

namespace copynator::core {

// Atalho padrão do Copynator.
inline constexpr std::string_view kDefaultAccelerator = "<Shift><Super>v";

// Retorna uma forma canônica: modificadores em ordem fixa, minúsculos e com
// sinônimos unificados (Mod4 -> super, Ctrl/Primary -> control), seguidos da
// tecla em minúsculas. Retorna string vazia para atalhos vazios/"disabled".
[[nodiscard]] std::string normalizeAccelerator(std::string_view accelerator);

// Compara dois atalhos pela forma canônica.
[[nodiscard]] bool sameAccelerator(std::string_view first, std::string_view second);

} // namespace copynator::core
