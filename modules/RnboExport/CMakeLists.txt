add_library(rnbo_gran_delay STATIC
		${CMAKE_CURRENT_SOURCE_DIR}/rnbo/RNBO.cpp
		${CMAKE_CURRENT_SOURCE_DIR}/rnbo_granular.cpp
)

target_include_directories(rnbo_gran_delay PRIVATE
		${CMAKE_CURRENT_SOURCE_DIR}/rnbo
		${CMAKE_CURRENT_SOURCE_DIR}/rnbo/common
)

target_compile_options(rnbo_gran_delay PRIVATE
		$<$<CXX_COMPILER_ID:MSVC>:/w>
		$<$<CXX_COMPILER_ID:GNU>:-w -fPIC>
		$<$<CXX_COMPILER_ID:Clang>:-w -fPIC>
)

target_link_libraries(${TARGET_NAME} PRIVATE rnbo_gran_delay)

target_include_directories(${TARGET_NAME} SYSTEM PRIVATE
		${CMAKE_CURRENT_SOURCE_DIR}/rnbo
		${CMAKE_CURRENT_SOURCE_DIR}/rnbo/common
)