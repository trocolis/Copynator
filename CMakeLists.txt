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
    VERSION 0.9.0
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
