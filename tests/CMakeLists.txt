find_package(Catch2 REQUIRED)

add_executable(runTests
    test_main.cpp
    unit/TrieTests.cpp
    unit/SpellCheckerTests.cpp
    ../src/core/Dictionary.cpp
    ../src/core/SpellChecker.cpp
    ../src/core/Trie.cpp
    ../src/data/Database.cpp
    ../src/app/Config.cpp
)

target_include_directories(runTests PRIVATE
    ../include
    ${Catch2_INCLUDE_DIRS}
    $<TARGET_PROPERTY:unofficial::sqlite3::sqlite3,INTERFACE_INCLUDE_DIRECTORIES>
)

target_link_libraries(runTests PRIVATE
    Catch2::Catch2WithMain
    nlohmann_json::nlohmann_json
    unofficial::sqlite3::sqlite3
)