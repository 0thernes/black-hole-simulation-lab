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

# No adapter is wired yet (each lands via its own ADR + PR per ADR-0005).
# Until then, enabling a flag must not silently build as if the integration
# were present, so we warn loudly. This keeps the option honest rather than
# a no-op (inspection finding S02.11).
foreach(_bhds_flag IN ITEMS
        BHDS_ENABLE_LIBIRREP BHDS_ENABLE_ESHKOL
        BHDS_ENABLE_QGT BHDS_ENABLE_MOONLAB)
    if(${_bhds_flag})
        message(WARNING
            "${_bhds_flag}=ON, but its adapter is not wired yet (ADR pending; "
            "see docs/integrations/). Building WITHOUT the integration.")
    endif()
endforeach()

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

add_executable(blackhole_ds_integrator_tests
    tests/integrator_tests.cpp
)

target_include_directories(blackhole_ds_integrator_tests
    PRIVATE
        ${PROJECT_SOURCE_DIR}/include
)

if(MSVC)
    target_compile_options(blackhole_ds_integrator_tests PRIVATE /W4)
else()
    target_compile_options(blackhole_ds_integrator_tests PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_executable(blackhole_ds_cli_tests
    tests/cli_tests.cpp
)

target_include_directories(blackhole_ds_cli_tests
    PRIVATE
        ${PROJECT_SOURCE_DIR}/include
)

if(MSVC)
    target_compile_options(blackhole_ds_cli_tests PRIVATE /W4)
else()
    target_compile_options(blackhole_ds_cli_tests PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_executable(blackhole_ds_geodesic_tests
    tests/geodesic_tests.cpp
)

target_include_directories(blackhole_ds_geodesic_tests
    PRIVATE
        ${PROJECT_SOURCE_DIR}/include
)

if(MSVC)
    target_compile_options(blackhole_ds_geodesic_tests PRIVATE /W4)
else()
    target_compile_options(blackhole_ds_geodesic_tests PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_executable(blackhole_ds_shadow_tests
    tests/shadow_tests.cpp
)

target_include_directories(blackhole_ds_shadow_tests
    PRIVATE
        ${PROJECT_SOURCE_DIR}/include
)

if(MSVC)
    target_compile_options(blackhole_ds_shadow_tests PRIVATE /W4)
else()
    target_compile_options(blackhole_ds_shadow_tests PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_executable(blackhole_ds_render_tests
    tests/render_tests.cpp
)

target_include_directories(blackhole_ds_render_tests
    PRIVATE
        ${PROJECT_SOURCE_DIR}/include
)

if(MSVC)
    target_compile_options(blackhole_ds_render_tests PRIVATE /W4)
else()
    target_compile_options(blackhole_ds_render_tests PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_executable(blackhole_ds_disk_tests
    tests/disk_tests.cpp
)

target_include_directories(blackhole_ds_disk_tests
    PRIVATE
        ${PROJECT_SOURCE_DIR}/include
)

if(MSVC)
    target_compile_options(blackhole_ds_disk_tests PRIVATE /W4)
else()
    target_compile_options(blackhole_ds_disk_tests PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_executable(blackhole_ds_kerr_shadow_tests
    tests/kerr_shadow_tests.cpp
)

target_include_directories(blackhole_ds_kerr_shadow_tests
    PRIVATE
        ${PROJECT_SOURCE_DIR}/include
)

if(MSVC)
    target_compile_options(blackhole_ds_kerr_shadow_tests PRIVATE /W4)
else()
    target_compile_options(blackhole_ds_kerr_shadow_tests PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_executable(blackhole_ds_kerr_geodesic_tests
    tests/kerr_geodesic_tests.cpp
)

target_include_directories(blackhole_ds_kerr_geodesic_tests
    PRIVATE
        ${PROJECT_SOURCE_DIR}/include
)

if(MSVC)
    target_compile_options(blackhole_ds_kerr_geodesic_tests PRIVATE /W4)
else()
    target_compile_options(blackhole_ds_kerr_geodesic_tests PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_executable(blackhole_ds_kerr_disk_tests
    tests/kerr_disk_tests.cpp
)

target_include_directories(blackhole_ds_kerr_disk_tests
    PRIVATE
        ${PROJECT_SOURCE_DIR}/include
)

if(MSVC)
    target_compile_options(blackhole_ds_kerr_disk_tests PRIVATE /W4)
else()
    target_compile_options(blackhole_ds_kerr_disk_tests PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_executable(blackhole_ds_kerr_metric_cross_check_tests
    tests/kerr_metric_cross_check_tests.cpp
)

target_include_directories(blackhole_ds_kerr_metric_cross_check_tests
    PRIVATE
        ${PROJECT_SOURCE_DIR}/include
)

if(MSVC)
    target_compile_options(blackhole_ds_kerr_metric_cross_check_tests PRIVATE /W4)
else()
    target_compile_options(blackhole_ds_kerr_metric_cross_check_tests PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_executable(blackhole_ds_thermo_tests
    tests/thermo_tests.cpp
)

target_include_directories(blackhole_ds_thermo_tests
    PRIVATE
        ${PROJECT_SOURCE_DIR}/include
)

if(MSVC)
    target_compile_options(blackhole_ds_thermo_tests PRIVATE /W4)
else()
    target_compile_options(blackhole_ds_thermo_tests PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_executable(blackhole_ds_qnm_tests
    tests/qnm_tests.cpp
)

target_include_directories(blackhole_ds_qnm_tests
    PRIVATE
        ${PROJECT_SOURCE_DIR}/include
)

if(MSVC)
    target_compile_options(blackhole_ds_qnm_tests PRIVATE /W4)
else()
    target_compile_options(blackhole_ds_qnm_tests PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_executable(blackhole_ds_starfield_tests
    tests/starfield_tests.cpp
)

target_include_directories(blackhole_ds_starfield_tests
    PRIVATE
        ${PROJECT_SOURCE_DIR}/include
)

if(MSVC)
    target_compile_options(blackhole_ds_starfield_tests PRIVATE /W4)
else()
    target_compile_options(blackhole_ds_starfield_tests PRIVATE -Wall -Wextra -Wpedantic)
endif()

# Stage 0 of the CUDA port (ADR-0013). Off by default. When no CUDA
# compiler is present the CPU renderer is the build, which is what CI uses.
option(BHDS_ENABLE_CUDA
    "Compile the CUDA device-count probe. Pixels stay on the CPU." OFF)
if(BHDS_ENABLE_CUDA)
    include(CheckLanguage)
    check_language(CUDA)
    if(CMAKE_CUDA_COMPILER)
        enable_language(CUDA)
        add_library(bhds_cuda_probe STATIC src/viz/cuda_probe.cu)
        set_target_properties(bhds_cuda_probe PROPERTIES CUDA_STANDARD 17)
        target_link_libraries(blackhole_ds PRIVATE bhds_cuda_probe)
        target_compile_definitions(blackhole_ds PRIVATE BHDS_HAS_CUDA=1)
    else()
        message(STATUS
            "BHDS_ENABLE_CUDA=ON but no CUDA compiler was found. "
            "Building the CPU renderer only.")
    endif()
endif()

add_test(NAME blackhole_ds_smoke_tests COMMAND blackhole_ds_smoke_tests)
add_test(NAME blackhole_ds_integrator_tests COMMAND blackhole_ds_integrator_tests)
add_test(NAME blackhole_ds_cli_tests COMMAND blackhole_ds_cli_tests)
add_test(NAME blackhole_ds_geodesic_tests COMMAND blackhole_ds_geodesic_tests)
add_test(NAME blackhole_ds_shadow_tests COMMAND blackhole_ds_shadow_tests)
add_test(NAME blackhole_ds_render_tests COMMAND blackhole_ds_render_tests)
add_test(NAME blackhole_ds_disk_tests COMMAND blackhole_ds_disk_tests)
add_test(NAME blackhole_ds_kerr_shadow_tests COMMAND blackhole_ds_kerr_shadow_tests)
add_test(NAME blackhole_ds_kerr_geodesic_tests COMMAND blackhole_ds_kerr_geodesic_tests)
add_test(NAME blackhole_ds_kerr_disk_tests COMMAND blackhole_ds_kerr_disk_tests)
add_test(NAME blackhole_ds_kerr_metric_cross_check_tests COMMAND blackhole_ds_kerr_metric_cross_check_tests)
add_test(NAME blackhole_ds_thermo_tests COMMAND blackhole_ds_thermo_tests)
add_executable(blackhole_ds_classical_physics_tests
    tests/classical_physics_tests.cpp
)

target_include_directories(blackhole_ds_classical_physics_tests
    PRIVATE
        ${PROJECT_SOURCE_DIR}/include
)

if(MSVC)
    target_compile_options(blackhole_ds_classical_physics_tests PRIVATE /W4)
else()
    target_compile_options(blackhole_ds_classical_physics_tests PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_executable(blackhole_ds_qrng_sample_tests
    tests/qrng_sample_tests.cpp
)

target_include_directories(blackhole_ds_qrng_sample_tests
    PRIVATE
        ${PROJECT_SOURCE_DIR}/include
)

if(MSVC)
    target_compile_options(blackhole_ds_qrng_sample_tests PRIVATE /W4)
else()
    target_compile_options(blackhole_ds_qrng_sample_tests PRIVATE -Wall -Wextra -Wpedantic)
endif()

# Pinned 0thernes/quantum_rng (commit 1a77e77). The upstream headers need
# POSIX unistd.h / sys/time.h, which the MSVC CI image does not provide.
enable_language(C)
include(CheckIncludeFile)
check_include_file(unistd.h BHDS_HAVE_UNISTD_H)
if(BHDS_HAVE_UNISTD_H)
    add_library(bhds_qrng STATIC
        external/quantum_rng/quantum_rng.c
    )
    target_include_directories(bhds_qrng PUBLIC
        ${PROJECT_SOURCE_DIR}/external/quantum_rng
    )
    target_link_libraries(blackhole_ds_qrng_sample_tests PRIVATE bhds_qrng)
    target_compile_definitions(blackhole_ds_qrng_sample_tests PRIVATE BHDS_HAS_QRNG=1)
endif()

add_test(NAME blackhole_ds_qnm_tests COMMAND blackhole_ds_qnm_tests)
add_test(NAME blackhole_ds_starfield_tests COMMAND blackhole_ds_starfield_tests)
add_test(NAME blackhole_ds_classical_physics_tests COMMAND blackhole_ds_classical_physics_tests)
add_test(NAME blackhole_ds_qrng_sample_tests COMMAND blackhole_ds_qrng_sample_tests)

# The bare-run test asserts on output content, not just the exit code, so a
# silently-broken CLI cannot pass (inspection finding S07.08).
add_test(NAME blackhole_ds_runs COMMAND blackhole_ds)
set_tests_properties(blackhole_ds_runs PROPERTIES
    PASS_REGULAR_EXPRESSION "Truth tier: analytic_classical")

# CSV mode emits the truth-tier-labeled header.
add_test(NAME blackhole_ds_csv COMMAND blackhole_ds --format csv --steps 2)
set_tests_properties(blackhole_ds_csv PROPERTIES
    PASS_REGULAR_EXPRESSION "model_status,mass_solar,spin_a_over_M")

# Help text is printed on request.
add_test(NAME blackhole_ds_help COMMAND blackhole_ds --help)
set_tests_properties(blackhole_ds_help PROPERTIES
    PASS_REGULAR_EXPRESSION "analytic Schwarzschild and Kerr observables")

# A bad flag exits non-zero.
add_test(NAME blackhole_ds_badflag COMMAND blackhole_ds --bogus)
set_tests_properties(blackhole_ds_badflag PROPERTIES WILL_FAIL TRUE)

# Image render reports a measured shadow radius near the analytic sqrt(27)
# ~ 5.2 M (the exact digit is resolution-dependent; render_tests checks the
# within-one-pixel agreement rigorously).
add_test(NAME blackhole_ds_image
    COMMAND blackhole_ds --image ${CMAKE_CURRENT_BINARY_DIR}/shadow_test.ppm)
set_tests_properties(blackhole_ds_image PROPERTIES
    PASS_REGULAR_EXPRESSION "Measured shadow radius: 5\\.[12]")

# Lensed accretion-disk render runs and reports its inclination.
add_test(NAME blackhole_ds_disk
    COMMAND blackhole_ds --disk ${CMAKE_CURRENT_BINARY_DIR}/disk_test.ppm
            --inclination 80)
set_tests_properties(blackhole_ds_disk PROPERTIES
    PASS_REGULAR_EXPRESSION "lensed accretion-disk image")

# Kerr shadow render at high spin reports a non-zero alpha-asymmetry (the spin
# signature). The boundary itself is exercised rigorously by
# blackhole_ds_kerr_shadow_tests; here we just confirm the CLI path works.
add_test(NAME blackhole_ds_kerr_shadow
    COMMAND blackhole_ds --kerr-shadow
            ${CMAKE_CURRENT_BINARY_DIR}/kerr_shadow_test.ppm
            --spin 0.99 --inclination 80)
set_tests_properties(blackhole_ds_kerr_shadow PROPERTIES
    PASS_REGULAR_EXPRESSION "Kerr shadow image")
