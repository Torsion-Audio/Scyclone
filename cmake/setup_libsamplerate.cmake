set(LIBSAMPLERATE_TESTS OFF CACHE BOOL "Disable deprecated LIBSAMPLERATE_TESTS option" FORCE)

if(SCYCLONE_SANITIZERS STREQUAL "ASAN" OR SCYCLONE_SANITIZERS STREQUAL "ASAN_UBSAN")
    set(LIBSAMPLERATE_ENABLE_SANITIZERS ON CACHE BOOL "" FORCE)
endif()

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/modules/libsamplerate)

if(TARGET scyclone_sanitizer_flags)
    target_link_libraries(samplerate PRIVATE scyclone_sanitizer_flags)
endif()

target_link_libraries(${TARGET_NAME} PRIVATE samplerate)
