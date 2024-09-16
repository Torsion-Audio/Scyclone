set(LIBSAMPLERATE_TESTS OFF CACHE BOOL "Disable deprecated LIBSAMPLERATE_TESTS option" FORCE)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/modules/libsamplerate)

target_link_libraries(${TARGET_NAME} PRIVATE samplerate)