# Externally provided libraries
# Using zip files instead is faster

# Test translation units only (headers are included, not compiled). Benchmark is separate.
file(GLOB_RECURSE TestFiles CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/test/*.cpp")
list(FILTER TestFiles EXCLUDE REGEX ".*/benchmark/.*")
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR}/test PREFIX "" FILES ${TestFiles})

# This module enables populating content at configure time via any method supported by the ExternalProject module. Whereas ExternalProject_Add() downloads at build time, the FetchContent module makes content available immediately, allowing the configure step to use the content in commands like add_subdirectory(), include() or file() operations.
include(FetchContent)

if(TARGET scyclone_sanitizer_flags AND CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /fsanitize=address")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /fsanitize=address")
endif()

FetchContent_Declare(googletest
        URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip)

# Sanitizer CI runs gtest only; no Test source uses benchmark. Skip fetch/link when sanitizers are on.
if(SCYCLONE_SANITIZERS STREQUAL "NONE")
    FetchContent_Declare(benchmark
            GIT_REPOSITORY https://github.com/google/benchmark.git
            GIT_TAG v1.8.0)
endif()

# Embedded googletest — never installed; avoids GTestTargets export errors when gtest uses project-only flags.
set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)

# This command ensures that each of the named dependencies are made available to the project by the time it returns. If the dependency has already been populated the command does nothing. Otherwise, the command populates the dependency and then calls add_subdirectory() on the result.
FetchContent_MakeAvailable(googletest)

# MSan / LEAK: every object in the link must be instrumented (including FetchContent gtest).
# Propagate flags directly — do not link scyclone_sanitizer_flags (breaks GTest install export validation).
if(SCYCLONE_SANITIZERS STREQUAL "MEMORY" OR SCYCLONE_SANITIZERS STREQUAL "LEAK")
    if(TARGET scyclone_sanitizer_flags)
        get_target_property(_scyclone_gt_san_compile_opts scyclone_sanitizer_flags INTERFACE_COMPILE_OPTIONS)
        get_target_property(_scyclone_gt_san_link_opts scyclone_sanitizer_flags INTERFACE_LINK_OPTIONS)
        foreach(_scyclone_gt IN ITEMS gtest gtest_main)
            if(TARGET ${_scyclone_gt})
                if(_scyclone_gt_san_compile_opts)
                    target_compile_options(${_scyclone_gt} PRIVATE ${_scyclone_gt_san_compile_opts})
                endif()
                if(_scyclone_gt_san_link_opts)
                    target_link_options(${_scyclone_gt} PRIVATE ${_scyclone_gt_san_link_opts})
                endif()
            endif()
        endforeach()
    endif()
endif()

if(SCYCLONE_SANITIZERS STREQUAL "NONE")
    set(BENCHMARK_ENABLE_TESTING OFF CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(benchmark)
endif()

# Setup the test executable
add_executable(Test ${TestFiles})
set_property(TARGET Test PROPERTY CXX_STANDARD 20)

target_link_libraries(Test PRIVATE gtest_main "${PROJECT_NAME}")
if(SCYCLONE_SANITIZERS STREQUAL "NONE")
    target_link_libraries(Test PRIVATE benchmark::benchmark)
endif()
if(TARGET scyclone_sanitizer_flags)
    target_link_libraries(Test PRIVATE scyclone_sanitizer_flags)
endif()
if(SCYCLONE_SKIP_PLUGIN_INTEGRATION_TEST)
    target_compile_definitions(Test PRIVATE SCYCLONE_SKIP_PLUGIN_INTEGRATION_TEST=1)
endif()

# We can't link again to the shared juce target without ODL violations (https://github.com/sudara/pamplejuce/issues/31, https://forum.juce.com/t/windows-linker-issue-on-develop/55524/2)
# Therefore we steal the compile definitions and include directories from the main target and pass them to our test target
# Since we linked the shared juce targets in PRIVATE mode, they are not linked to the test target again
target_compile_definitions(Test PRIVATE $<TARGET_PROPERTY:${PROJECT_NAME},COMPILE_DEFINITIONS>)
target_include_directories(Test PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/test/torsion
        ${CMAKE_CURRENT_SOURCE_DIR}/test/torsion/audio
        ${CMAKE_CURRENT_SOURCE_DIR}/test/torsion/processors
        ${CMAKE_CURRENT_SOURCE_DIR}/test/torsion/gtest
        ${CMAKE_CURRENT_SOURCE_DIR}/test/scyclone/resampling
        ${CMAKE_CURRENT_SOURCE_DIR}/test/scyclone/mixer
        $<TARGET_PROPERTY:${PROJECT_NAME},INCLUDE_DIRECTORIES>)

# Make an Xcode Scheme for the test executable so we can run Test in the IDE
set_target_properties(Test PROPERTIES XCODE_GENERATE_SCHEME ON)

# Organize the test source in the test/ folder in the IDE
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR}/test PREFIX "" FILES ${TestFiles})

# include Loads and runs CMake code from the file given. Loads and runs CMake code from the file given.
include(GoogleTest)

# Default CI / sanitizer jobs: exclude ExtendedHostMatrix suite.
add_test(NAME ScycloneTests COMMAND Test --gtest_filter=-*ExtendedHostMatrix*)
set_tests_properties(ScycloneTests PROPERTIES LABELS "default")

# Extended host matrix — release tags and manual: ctest -L extended-matrix
add_test(NAME ScycloneTestsExtendedMatrix COMMAND Test --gtest_filter=*ExtendedHostMatrix*)
set_tests_properties(ScycloneTestsExtendedMatrix PROPERTIES LABELS "extended-matrix")
