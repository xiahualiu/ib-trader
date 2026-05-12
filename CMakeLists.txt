cmake_minimum_required(VERSION 3.21)
project(ibalgotrade VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# ── Dependencies ────────────────────────────────────────────────
include(FetchContent)

# pybind11
FetchContent_Declare(
    pybind11
    GIT_REPOSITORY https://github.com/pybind/pybind11.git
    GIT_TAG v2.13.6
    GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(pybind11)

# ── Core C++ library ────────────────────────────────────────────
add_library(ibalgotrade_core STATIC
    src/core/order.cpp
)
target_include_directories(ibalgotrade_core
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)
target_compile_features(ibalgotrade_core PUBLIC cxx_std_20)

# ── Python bindings ─────────────────────────────────────────────
pybind11_add_module(_core
    src/pybind/module.cpp
)
target_link_libraries(_core PRIVATE ibalgotrade_core)
target_compile_definitions(_core PRIVATE VERSION_INFO="${PROJECT_VERSION}")

# Place the built .so in the python package dir for editable install
add_custom_command(TARGET _core POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "$<TARGET_FILE:_core>"
        "${CMAKE_SOURCE_DIR}/python/ibalgotrade/_core$<TARGET_FILE_SUFFIX:_core>"
)

# ── Tests ───────────────────────────────────────────────────────
option(BUILD_TESTS "Build tests" ON)

if(BUILD_TESTS)
    enable_testing()

    FetchContent_Declare(
        Catch2
        GIT_REPOSITORY https://github.com/catchorg/Catch2.git
        GIT_TAG v3.8.0
        GIT_SHALLOW TRUE
    )
    FetchContent_MakeAvailable(Catch2)

    add_executable(ibalgotrade_tests
        tests/cpp/test_order_book.cpp
    )
    target_link_libraries(ibalgotrade_tests PRIVATE ibalgotrade_core Catch2::Catch2WithMain)
    target_compile_features(ibalgotrade_tests PRIVATE cxx_std_20)

    add_test(NAME ibalgotrade_tests COMMAND ibalgotrade_tests)
endif()
