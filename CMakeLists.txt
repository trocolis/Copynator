# =============================================================================
# Copynator — gerenciador de área de transferência para Linux
#
# Versionamento: RELEASE.FEATURE.HOTFIX
#   - RELEASE: versões estáveis e marcos maiores
#   - FEATURE: cada nova funcionalidade/escopo entregue
#   - HOTFIX : correções pontuais sem novas funcionalidades
# =============================================================================
cmake_minimum_required(VERSION 3.25)

project(copynator
    VERSION 1.1.0
    DESCRIPTION "Gerenciador de área de transferência estilo Windows 11 para Linux"
    LANGUAGES CXX)

# -----------------------------------------------------------------------------
# Padrão da linguagem: C++23 sem extensões específicas do compilador
# -----------------------------------------------------------------------------
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Gera compile_commands.json para ferramentas como clangd e clang-tidy
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Build "Release" por padrão quando nenhum tipo foi informado
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Tipo de build" FORCE)
endif()

# -----------------------------------------------------------------------------
# Opções do projeto
# -----------------------------------------------------------------------------
option(COPYNATOR_BUILD_TESTS "Compila os testes unitários (GoogleTest)" ON)

# Diretórios de instalação padrão (bin, share, etc.)
include(GNUInstallDirs)

# Módulo interno com os avisos de compilação usados em todos os alvos
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")
include(CompilerWarnings)

# -----------------------------------------------------------------------------
# Subprojetos
# -----------------------------------------------------------------------------
add_subdirectory(src)

if(COPYNATOR_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

# -----------------------------------------------------------------------------
# Instalação de dados: .desktop, autostart e extensão do GNOME Shell
#
# Instalação do usuário (recomendada):
#   cmake -B build -DCMAKE_INSTALL_PREFIX=$HOME/.local
#   cmake --build build && cmake --install build
# -----------------------------------------------------------------------------

# O autostart fica em ~/.config/autostart para instalações dentro do HOME e em
# /etc/xdg/autostart para instalações do sistema.
set(copynator_home "$ENV{HOME}")
set(copynator_user_install FALSE)
if(copynator_home)
    cmake_path(IS_PREFIX copynator_home "${CMAKE_INSTALL_PREFIX}" NORMALIZE copynator_user_install)
endif()
if(copynator_user_install)
    set(copynator_default_autostart "$ENV{HOME}/.config/autostart")
else()
    set(copynator_default_autostart "${CMAKE_INSTALL_FULL_SYSCONFDIR}/xdg/autostart")
endif()
set(COPYNATOR_AUTOSTART_DIR "${copynator_default_autostart}"
    CACHE PATH "Diretório de autostart do daemon")

# Os .desktop usam o caminho absoluto do executável: o PATH da sessão gráfica
# nem sempre inclui ~/.local/bin.
configure_file(data/io.github.copynator.desktop.in
    ${CMAKE_BINARY_DIR}/data/io.github.copynator.desktop @ONLY)
configure_file(data/io.github.copynator-autostart.desktop.in
    ${CMAKE_BINARY_DIR}/data/io.github.copynator-autostart.desktop @ONLY)

install(FILES ${CMAKE_BINARY_DIR}/data/io.github.copynator.desktop
    DESTINATION ${CMAKE_INSTALL_DATADIR}/applications)
install(FILES ${CMAKE_BINARY_DIR}/data/io.github.copynator-autostart.desktop
    DESTINATION ${COPYNATOR_AUTOSTART_DIR}
    RENAME io.github.copynator.desktop)

# Extensão: a versão em metadata.json vem do project(VERSION ...).
set(copynator_extension_uuid "copynator@bruno.local")
configure_file(extension/${copynator_extension_uuid}/metadata.json.in
    ${CMAKE_BINARY_DIR}/extension/metadata.json @ONLY)
install(DIRECTORY extension/${copynator_extension_uuid}
    DESTINATION ${CMAKE_INSTALL_DATADIR}/gnome-shell/extensions
    PATTERN "metadata.json.in" EXCLUDE)
install(FILES ${CMAKE_BINARY_DIR}/extension/metadata.json
    DESTINATION ${CMAKE_INSTALL_DATADIR}/gnome-shell/extensions/${copynator_extension_uuid})

# Orientações exibidas ao final da instalação.
install(CODE [[
    message(STATUS "")
    message(STATUS "Copynator instalado. Próximos passos:")
    message(STATUS "  1. gnome-extensions enable copynator@bruno.local")
    message(STATUS "     (no Wayland, encerre e inicie a sessão para o GNOME Shell carregar a extensão)")
    message(STATUS "  2. copynator --setup-shortcut")
    message(STATUS "  3. copynator --status")
]])
