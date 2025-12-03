cmake_minimum_required(VERSION 3.15)
project(demo_analyser LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(${PROJECT_SOURCE_DIR}/include)

add_library(demo_analyser
    src/DemoParser.cpp
)

target_include_directories(demo_analyser PUBLIC include)

add_subdirectory(app)

# add_subdirectory(tests)
