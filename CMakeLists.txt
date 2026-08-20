cmake_minimum_required(VERSION 3.15)
project(demo_parser LANGUAGES CXX)

add_library(demo_parser
    src/DemoParser.cpp
)

target_include_directories(demo_parser PUBLIC include)
target_compile_features(demo_parser PUBLIC cxx_std_17)

add_subdirectory(app)

include(CTest)
if(BUILD_TESTING)
    add_subdirectory(tests)
endif()
