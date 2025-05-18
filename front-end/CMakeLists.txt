cmake_minimum_required(VERSION 3.10)
project(front-end LANGUAGES CXX)

add_library(front-end STATIC
    src/astDump.cpp
    src/core.cpp
    src/lexer.cpp
    src/parser.cpp
    AST/src/nameTable.cpp
    src/treeSaver.cpp
)

target_link_libraries(front-end PUBLIC customWarning binaryTree Buffer)

target_include_directories(front-end PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/AST/include
)