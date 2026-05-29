cmake_minimum_required(VERSION 3.16)
project(electromagnetic-field LANGUAGES CXX)

set(ENGINE_WITH_GTEST ON CACHE BOOL "" FORCE)
add_subdirectory(external/engine)
enable_testing()

engine_add_game(electromagnetic-field
    src/main.cpp
    src/game.cpp
    src/electrostatics.cpp
    src/magnetism.cpp
    src/simulation.cpp
    src/scene.cpp
    src/camera_control.cpp
    src/field_viz.cpp
    src/field_view.cpp
    src/trails.cpp
    src/panel_view_model.cpp
    src/help.cpp
    src/help_view_model.cpp
    src/welcome.cpp)

add_executable(electromagnetic_field_tests
    tests/electrostatics_test.cpp
    tests/simulation_test.cpp
    tests/scene_test.cpp
    tests/camera_control_test.cpp
    tests/field_viz_test.cpp
    tests/dynamics_test.cpp
    tests/slider_test.cpp
    tests/locale_test.cpp
    tests/magnetism_test.cpp
    src/electrostatics.cpp
    src/magnetism.cpp
    src/simulation.cpp
    src/scene.cpp
    src/camera_control.cpp
    src/field_viz.cpp
    src/trails.cpp)
target_compile_features(electromagnetic_field_tests PRIVATE cxx_std_23)
target_include_directories(electromagnetic_field_tests PRIVATE include)
file(TO_CMAKE_PATH "${CMAKE_SOURCE_DIR}" GAME_SOURCE_DIR_POSIX)
target_compile_definitions(electromagnetic_field_tests PRIVATE GAME_SOURCE_DIR="${GAME_SOURCE_DIR_POSIX}")
target_link_libraries(electromagnetic_field_tests PRIVATE GTest::gtest_main glm::glm)
gtest_discover_tests(electromagnetic_field_tests DISCOVERY_MODE PRE_TEST)
