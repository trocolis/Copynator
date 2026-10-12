// =============================================================================
// StyleSheet.hpp
//
// Folha de estilos (CSS) da interface. As cores vêm das variáveis do
// libadwaita (ex.: --accent-bg-color, --card-bg-color), por isso o visual
// acompanha automaticamente o modo claro/escuro e a cor de destaque do GNOME.
// =============================================================================
#pragma once

namespace copynator::ui {

// Instala o CSS do Copynator no display padrão. Deve ser chamada uma única vez,
// depois que o GTK estiver inicializado.
void installStyleSheet();

} // namespace copynator::ui
