cmake_minimum_required(VERSION 3.16)
project(electromagnetic-field LANGUAGES CXX)

set(ENGINE_WITH_GTEST ON CACHE BOOL "" FORCE)
add_subdirectory(external/engine)
enable_testing()

engine_add_game(electromagnetic-field
    src/main.cpp
    src/game.cpp)

add_executable(electromagnetic_field_tests
    tests/smoke_test.cpp)
target_compile_features(electromagnetic_field_tests PRIVATE cxx_std_23)
target_include_directories(electromagnetic_field_tests PRIVATE include)
target_link_libraries(electromagnetic_field_tests PRIVATE GTest::gtest_main)
gtest_discover_tests(electromagnetic_field_tests DISCOVERY_MODE PRE_TEST)
