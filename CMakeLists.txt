cmake_minimum_required(VERSION 3.15)

project(lol_graphdb LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

find_package(SQLite3 REQUIRED)
find_package(CURL REQUIRED)
find_package(Gumbo QUIET)

option(USE_GUMBO "Use gumbo-parser for HTML parsing" ${Gumbo_FOUND})

add_library(lolgraph
    src/storage/database.cpp
    src/storage/schema.cpp
    src/scraper/fetcher.cpp
    src/scraper/parser.cpp
    src/pipeline/ingest.cpp
    src/analysis/synergy.cpp
)

target_include_directories(lolgraph
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(lolgraph
    PUBLIC
        SQLite::SQLite3
        CURL::libcurl
)

if(USE_GUMBO)
    target_compile_definitions(lolgraph PUBLIC LOL_USE_GUMBO)
    target_link_libraries(lolgraph PUBLIC Gumbo::Gumbo)
endif()

add_executable(lolgraph_cli src/main.cpp)
target_link_libraries(lolgraph_cli PRIVATE lolgraph)

add_executable(lolgraph_tests src/tests/sanity.cpp)
target_link_libraries(lolgraph_tests PRIVATE lolgraph)

enable_testing()
add_test(NAME lolgraph_sanity COMMAND lolgraph_tests)

