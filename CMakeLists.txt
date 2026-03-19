cmake_minimum_required(VERSION 3.10)
project(QuickQuill)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# find system libraries
# vcpkg will ensure these are found
find_package(nlohmann_json 3.2.0 CONFIG REQUIRED)
find_package(Crow CONFIG REQUIRED)
find_package(asio CONFIG REQUIRED)
find_package(unofficial-sqlite3 CONFIG REQUIRED)
find_package(Threads REQUIRED)

add_subdirectory(src)
