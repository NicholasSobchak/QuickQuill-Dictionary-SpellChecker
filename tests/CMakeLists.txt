find_package(Catch2 REQUIRED)
include(Catch)

add_executable(runTests
    test_main.cpp
    integration/WordApiIntegration.cpp
    unit/TrieTests.cpp
    ../src/core/Dictionary.cpp
    ../src/core/SpellChecker.cpp
    ../src/core/Trie.cpp
    ../src/data/Database.cpp
    ../src/app/Config.cpp
    ../src/http/handlers/WordHandler.cpp
    ../src/http/dto/WordResponse.cpp
)

target_include_directories(runTests PRIVATE
    ../include
    ${Catch2_INCLUDE_DIRS}
    ${CMAKE_CURRENT_LIST_DIR}/support
    $<TARGET_PROPERTY:unofficial::sqlite3::sqlite3,INTERFACE_INCLUDE_DIRECTORIES>
)

target_link_libraries(runTests PRIVATE
    Catch2::Catch2WithMain
    nlohmann_json::nlohmann_json
    unofficial::sqlite3::sqlite3
)

# Register tests with CTest via Catch2
catch_discover_tests(runTests)
