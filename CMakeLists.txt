cmake_minimum_required(VERSION 3.20)

project(oppengrok_blackhole_lab
    VERSION 0.1.0
    DESCRIPTION "Research-first black hole simulation lab"
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# -----------------------------------------------------------------------------
# Optional external integrations (off by default)
#
# Each integration is gated behind a BHDS_ENABLE_<NAME> option. The kernel
# must build and test cleanly with all of these set to OFF. Adoption of
# any single integration requires an ADR in docs/log/DECISIONS.md.
#
# See external/README.md and docs/integrations/TSOTCHKE_ECOSYSTEM.md for
# the integration plan.
# -----------------------------------------------------------------------------
option(BHDS_ENABLE_LIBIRREP "Enable tsotchke/libirrep adapter" OFF)
option(BHDS_ENABLE_ESHKOL "Enable Eshkol DSL transpile step" OFF)
option(BHDS_ENABLE_QGT "Enable tsotchke/quantum_geometric_tensor backend" OFF)
option(BHDS_ENABLE_MOONLAB "Enable tsotchke/moonlab GPU backend" OFF)

if(BHDS_ENABLE_LIBIRREP OR BHDS_ENABLE_ESHKOL OR BHDS_ENABLE_QGT OR BHDS_ENABLE_MOONLAB)
    include(FetchContent)
endif()

# Example adapter wiring (commented; real adapters land via ADR + PR):
#
# if(BHDS_ENABLE_LIBIRREP)
#     FetchContent_Declare(
#         libirrep
#         GIT_REPOSITORY https://github.com/tsotchke/libirrep.git
#         GIT_TAG <PINNED_COMMIT_SHA>
#     )
#     FetchContent_MakeAvailable(libirrep)
# endif()

add_executable(blackhole_ds
    src/BlackHoleDS.cpp
    src/cli/main.cpp
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
