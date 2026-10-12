# =============================================================================
# src/CMakeLists.txt
#
# Organização em bibliotecas por camada (dependências apontam sempre "para
# dentro", em direção ao core):
#
#   copynator_core    -> regras de negócio puras, sem GTK (testável)
#   copynator_storage -> persistência (SQLite), implementa as portas do core
#   copynator_service -> interface D-Bus do daemon (servidor e cliente)
#   copynator_ui      -> interface gráfica (gtkmm + libadwaita)
#   copynator_app     -> composição da aplicação (liga todas as camadas)
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
# Camada ui: gtkmm 4 + libadwaita (API C, usada para tema/cores do sistema)
# -----------------------------------------------------------------------------
pkg_check_modules(GTKMM REQUIRED IMPORTED_TARGET gtkmm-4.0)
pkg_check_modules(LIBADWAITA REQUIRED IMPORTED_TARGET libadwaita-1)

add_library(copynator_ui STATIC
    ui/HistoryRow.cpp
    ui/HistoryWindow.cpp
    ui/StyleSheet.cpp
)
target_link_libraries(copynator_ui
    PUBLIC copynator_core PkgConfig::GTKMM)
copynator_set_warnings(copynator_ui)

# -----------------------------------------------------------------------------
# Camada service: D-Bus (giomm)
# -----------------------------------------------------------------------------
pkg_check_modules(GIOMM REQUIRED IMPORTED_TARGET giomm-2.68)

add_library(copynator_service STATIC
    service/DaemonClient.cpp
    service/DaemonDBusService.cpp
)
target_link_libraries(copynator_service
    PUBLIC copynator_core PkgConfig::GIOMM)
copynator_set_warnings(copynator_service)

# -----------------------------------------------------------------------------
# Camada app: composição
# -----------------------------------------------------------------------------
add_library(copynator_app STATIC
    app/CopynatorApplication.cpp
)
target_link_libraries(copynator_app
    PUBLIC copynator_ui copynator_storage copynator_service
    PRIVATE PkgConfig::LIBADWAITA)
copynator_set_warnings(copynator_app)

# -----------------------------------------------------------------------------
# Executável principal
# -----------------------------------------------------------------------------
add_executable(copynator main.cpp)
target_link_libraries(copynator PRIVATE copynator_app)
copynator_set_warnings(copynator)

install(TARGETS copynator RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
