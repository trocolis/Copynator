# =============================================================================
# src/CMakeLists.txt
#
# Organização em bibliotecas por camada (dependências apontam sempre "para
# dentro", em direção ao core):
#
#   copynator_core  -> regras de negócio puras, sem GTK (testável)
#   copynator (exe) -> ponto de entrada que compõe todas as camadas
# =============================================================================

# Gera o cabeçalho de versão a partir do project(VERSION ...)
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/core/Version.hpp.in
    ${CMAKE_CURRENT_BINARY_DIR}/generated/core/Version.hpp
    @ONLY)

# -----------------------------------------------------------------------------
# Camada core: lógica pura
# -----------------------------------------------------------------------------
add_library(copynator_core STATIC
    core/CommandLine.cpp
)
target_include_directories(copynator_core PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}/generated)
copynator_set_warnings(copynator_core)

# -----------------------------------------------------------------------------
# Executável principal
# -----------------------------------------------------------------------------
add_executable(copynator main.cpp)
target_link_libraries(copynator PRIVATE copynator_core)
copynator_set_warnings(copynator)

install(TARGETS copynator RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
