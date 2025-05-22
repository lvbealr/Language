cmake_minimum_required(VERSION 3.10)
project(back-end LANGUAGES CXX)

add_library(back-end STATIC
    # add src
    src/core.cpp
    src/treeReader.cpp
    src/asmTranslator.cpp
    ${CMAKE_SOURCE_DIR}/front-end/AST/src/nameTable.cpp
    IR/src/IRBasics.cpp
    IR/src/IRGenerator.cpp
    IR/src/ASMGenerator.cpp
    IR/src/debugUtils.cpp
)

target_link_libraries(back-end PUBLIC customWarning binaryTree Buffer)

target_include_directories(back-end PUBLIC
    # add include
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/front-end/AST/include
    ${CMAKE_SOURCE_DIR}/front-end/
    ${CMAKE_CURRENT_SOURCE_DIR}/IR/include
    ${CMAKE_CURRENT_SOURCE_DIR}/IR/linkedList/
)

add_executable(backend main.cpp)

target_link_libraries(backend PRIVATE back-end)