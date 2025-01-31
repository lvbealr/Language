cmake_minimum_required(VERSION 3.30)
project(PrisonLanguage)

set(CMAKE_CXX_STANDARD 17)

include_directories(colorPrint)
include_directories(customWarning)
include_directories(binaryTree)
include_directories(simpleConsoleParser)

add_executable(PrisonLanguage colorPrint/colorPrint.cpp main.cpp)
