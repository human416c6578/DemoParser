add_executable(demo_reader DemoReader.cpp)


target_link_libraries(demo_reader PRIVATE demo_parser)

target_compile_features(demo_reader PRIVATE cxx_std_17)
