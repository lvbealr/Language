cmake_minimum_required(VERSION 3.10)
project(binaryTree LANGUAGES CXX)

add_library(binaryTree INTERFACE)

target_link_libraries(binaryTree INTERFACE customWarning colorPrint)

target_include_directories(binaryTree INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)