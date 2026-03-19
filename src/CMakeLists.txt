# Add all core and data source files that are common to both applications
set(CORE_SOURCES
    core/Dictionary.cpp
    core/SpellChecker.cpp
    core/Trie.cpp
    data/Database.cpp
)

# --- Console Application ---
add_executable(dict
    app/main.cpp
    ${CORE_SOURCES}
)

# Set common include directories for the dict target
target_include_directories(dict PRIVATE
    ../include
    $<TARGET_PROPERTY:unofficial::sqlite3::sqlite3,INTERFACE_INCLUDE_DIRECTORIES>
)

# Link executable against all required libraries
target_link_libraries(dict PRIVATE
    nlohmann_json::nlohmann_json
    unofficial::sqlite3::sqlite3
)


# --- Web Application ---
add_executable(dict_crow
    app/main_crow.cpp
    http/Server.cpp
    http/routes/WordRoutes.cpp
    http/handlers/WordHandler.cpp
    http/dto/WordResponse.cpp
    ${CORE_SOURCES}
)

# Set common include directories for the dict_crow target
target_include_directories(dict_crow PRIVATE
    ../include
    $<TARGET_PROPERTY:unofficial::sqlite3::sqlite3,INTERFACE_INCLUDE_DIRECTORIES>
)

# Link executable against all required libraries
target_link_libraries(dict_crow PRIVATE
    nlohmann_json::nlohmann_json
    Crow::Crow
    Threads::Threads
    unofficial::sqlite3::sqlite3
)

# Define the static file directory for Crow at compile time
target_compile_definitions(dict_crow PRIVATE
    CROW_STATIC_DIR="${CMAKE_SOURCE_DIR}/web"
)