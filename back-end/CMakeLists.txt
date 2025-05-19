cmake_minimum_required(VERSION 3.10)
project(back-end LANGUAGES CXX)

add_library(back-end STATIC
    # add src
    src/core.cpp
    src/treeReader.cpp
)

target_link_libraries(back-end PUBLIC customWarning binaryTree Buffer)

target_include_directories(back-end PUBLIC
    # add include
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/front-end/AST/include
    ${CMAKE_SOURCE_DIR}/front-end/
)

add_executable(backend main.cpp)

target_link_libraries(backend PRIVATE back-end)