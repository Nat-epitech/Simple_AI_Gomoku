cmake_minimum_required(VERSION 3.10)

# Project name & version
project(Gomoku VERSION 1)

# C++ Standards
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Executable
add_executable(pbrain-gomoku-ai
src/main.cpp
"src/PiskvorkLinker.cpp"
)

# Link headers
include_directories(./include/)
