# =============================================================================
# tests/CMakeLists.txt — testes unitários com GoogleTest
# =============================================================================
find_package(GTest REQUIRED)
include(GoogleTest)

add_executable(copynator_tests
    CommandLineTest.cpp
    ContentClassifierTest.cpp
    HistoryServiceTest.cpp
    PlacementTest.cpp
    RelativeTimeTest.cpp
    SqliteHistoryRepositoryTest.cpp
    TextUtilsTest.cpp
)
target_link_libraries(copynator_tests PRIVATE
    copynator_core
    copynator_storage
    GTest::gtest_main)
copynator_set_warnings(copynator_tests)

# Registra cada TEST() individualmente no CTest
gtest_discover_tests(copynator_tests)
