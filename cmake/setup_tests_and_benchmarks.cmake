# Test translation units only (headers are included, not compiled). Benchmark is separate.
file(GLOB_RECURSE TestFiles CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/test/*.cpp")
list(FILTER TestFiles EXCLUDE REGEX ".*/benchmark/.*")
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR}/test PREFIX "" FILES ${TestFiles})

include(FetchContent)

if(TARGET scyclone_sanitizer_flags AND CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /fsanitize=address")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /fsanitize=address")
endif()

FetchContent_Declare(googletest
        URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip)

if(SCYCLONE_SANITIZERS STREQUAL "NONE")
    FetchContent_Declare(benchmark
            GIT_REPOSITORY https://github.com/google/benchmark.git
            GIT_TAG v1.8.0)
endif()

set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

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

add_executable(Test ${TestFiles})
set_property(TARGET Test PROPERTY CXX_STANDARD 20)

target_link_libraries(Test PRIVATE gtest_main "${PROJECT_NAME}")
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

# INCLUDE_DIRECTORIES copied via genex (evaluated at generate time, after RnboExport). RNBO paths
# lose the SYSTEM flag, so re-apply them here (matches modules/RnboExport/CMakeLists.txt).
set(_scyclone_rnbo_include_root "${CMAKE_CURRENT_SOURCE_DIR}/modules/RnboExport/rnbo")
target_include_directories(Test PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/test/torsion
        ${CMAKE_CURRENT_SOURCE_DIR}/test/torsion/audio
        ${CMAKE_CURRENT_SOURCE_DIR}/test/torsion/processors
        ${CMAKE_CURRENT_SOURCE_DIR}/test/torsion/gtest
        ${CMAKE_CURRENT_SOURCE_DIR}/test/scyclone/resampling
        ${CMAKE_CURRENT_SOURCE_DIR}/test/scyclone/mixer
        "${CMAKE_BINARY_DIR}/assets/juce_binarydata_BinaryData/JuceLibraryCode"
        $<TARGET_PROPERTY:${PROJECT_NAME},INCLUDE_DIRECTORIES>)
target_include_directories(Test SYSTEM PRIVATE
        ${_scyclone_rnbo_include_root}
        ${_scyclone_rnbo_include_root}/common)

message(WARNING
        "Test target: RNBO header warnings are suppressed (SYSTEM include, same as ${TARGET_NAME}). "
        "Unused static inline helpers in RNBO exports can trigger MSVC C4505 at /W4.")

set_target_properties(Test PROPERTIES XCODE_GENERATE_SCHEME ON)
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR}/test PREFIX "" FILES ${TestFiles})

include(GoogleTest)

add_test(NAME ScycloneTests COMMAND Test --gtest_filter=-*ExtendedHostMatrix*)
set_tests_properties(ScycloneTests PROPERTIES LABELS "default")

add_test(NAME ScycloneTestsExtendedMatrix COMMAND Test --gtest_filter=*ExtendedHostMatrix*)
set_tests_properties(ScycloneTestsExtendedMatrix PROPERTIES LABELS "extended-matrix")

if(SCYCLONE_SANITIZERS STREQUAL "NONE")
    add_executable(Benchmark test/benchmark/benchmark.cpp)
    set_property(TARGET Benchmark PROPERTY CXX_STANDARD 20)
    target_link_libraries(Benchmark PRIVATE benchmark::benchmark "${PROJECT_NAME}")

    # Same ODR workaround as Test above: Benchmark includes PluginEditor.h, whose source-tree
    # include dirs and compile defs are PRIVATE on ${PROJECT_NAME} and can't be re-linked.
    target_compile_definitions(Benchmark PRIVATE $<TARGET_PROPERTY:${PROJECT_NAME},COMPILE_DEFINITIONS>)
    target_include_directories(Benchmark PRIVATE
            "${CMAKE_BINARY_DIR}/assets/juce_binarydata_BinaryData/JuceLibraryCode"
            $<TARGET_PROPERTY:${PROJECT_NAME},INCLUDE_DIRECTORIES>)
    target_include_directories(Benchmark SYSTEM PRIVATE
            ${_scyclone_rnbo_include_root}
            ${_scyclone_rnbo_include_root}/common)

    set_target_properties(Benchmark PROPERTIES XCODE_GENERATE_SCHEME ON)
endif()
