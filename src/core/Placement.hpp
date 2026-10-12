// =============================================================================
// Placement.hpp
//
// Onde a janela deve aparecer ao ser aberta:
//   - AtCursor: na posição do cursor do mouse (atalho Shift+Super+V);
//   - Centered: no centro da tela (clique no ícone da barra).
//
// O valor também trafega como texto pelo D-Bus ("cursor" / "center").
// =============================================================================
#pragma once

#include <optional>
#include <string_view>

namespace copynator::core {

enum class Placement {
    AtCursor,
    Centered,
};

// Converte para o texto usado no D-Bus.
[[nodiscard]] constexpr std::string_view toString(Placement placement) {
    return placement == Placement::Centered ? "center" : "cursor";
}

// Interpreta o texto vindo do D-Bus. Retorna vazio para valores inválidos.
[[nodiscard]] constexpr std::optional<Placement> parsePlacement(std::string_view text) {
    if (text == "cursor") {
        return Placement::AtCursor;
    }
    if (text == "center") {
        return Placement::Centered;
    }
    return std::nullopt;
}

} // namespace copynator::core
