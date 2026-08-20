add_executable(demo_parser_tests
    ParserTests.cpp
)

target_link_libraries(demo_parser_tests PRIVATE demo_parser)
target_compile_features(demo_parser_tests PRIVATE cxx_std_17)

add_test(NAME demo_parser_tests COMMAND demo_parser_tests)
set_tests_properties(demo_parser_tests PROPERTIES TIMEOUT 5)
