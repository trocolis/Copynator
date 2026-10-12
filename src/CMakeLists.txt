# =============================================================================
# src/CMakeLists.txt
#
# Organização em bibliotecas por camada (dependências apontam sempre "para
# dentro", em direção ao core):
#
#   copynator_core    -> regras de negócio puras, sem GTK (testável)
#   copynator_storage -> persistência (SQLite), implementa as portas do core
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
    core/HistoryService.cpp
    core/ContentClassifier.cpp
    core/RelativeTime.cpp
    core/TextUtils.cpp
)
target_include_directories(copynator_core PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}/generated)
copynator_set_warnings(copynator_core)

# -----------------------------------------------------------------------------
# Camada storage: SQLite
# -----------------------------------------------------------------------------
find_package(PkgConfig REQUIRED)
pkg_check_modules(SQLITE3 REQUIRED IMPORTED_TARGET sqlite3)

add_library(copynator_storage STATIC
    storage/SqliteDatabase.cpp
    storage/SqliteHistoryRepository.cpp
    storage/StoragePaths.cpp
)
target_link_libraries(copynator_storage
    PUBLIC copynator_core
    PRIVATE PkgConfig::SQLITE3)
copynator_set_warnings(copynator_storage)

# -----------------------------------------------------------------------------
# Executável principal
# -----------------------------------------------------------------------------
add_executable(copynator main.cpp)
target_link_libraries(copynator PRIVATE copynator_core copynator_storage)
copynator_set_warnings(copynator)

install(TARGETS copynator RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
