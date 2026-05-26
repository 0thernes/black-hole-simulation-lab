cmake_minimum_required(VERSION 3.20)

project(oppengrok_blackhole_lab
    VERSION 0.1.0
    DESCRIPTION "Research-first black hole simulation lab"
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(blackhole_ds
    src/BlackHoleDS.cpp
)

target_include_directories(blackhole_ds
    PRIVATE
        ${PROJECT_SOURCE_DIR}/include
)

if(MSVC)
    target_compile_options(blackhole_ds PRIVATE /W4)
else()
    target_compile_options(blackhole_ds PRIVATE -Wall -Wextra -Wpedantic)
endif()

enable_testing()

add_executable(blackhole_ds_smoke_tests
    tests/smoke_tests.cpp
)

target_include_directories(blackhole_ds_smoke_tests
    PRIVATE
        ${PROJECT_SOURCE_DIR}/include
)

if(MSVC)
    target_compile_options(blackhole_ds_smoke_tests PRIVATE /W4)
else()
    target_compile_options(blackhole_ds_smoke_tests PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(NAME blackhole_ds_smoke_tests COMMAND blackhole_ds_smoke_tests)
add_test(NAME blackhole_ds_runs COMMAND blackhole_ds)
