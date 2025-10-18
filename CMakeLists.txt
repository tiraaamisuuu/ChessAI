cmake_minimum_required(VERSION 3.22)
project(ChessAI)

set(CMAKE_CXX_STANDARD 20)

include_directories(include)
file(GLOB SOURCES "src/*.cpp")

add_executable(ChessAI ${SOURCES} main.cpp)
