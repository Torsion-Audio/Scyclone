include_guard(GLOBAL)

include(${CMAKE_CURRENT_LIST_DIR}/RNBOConan.cmake)

#use mingw_stdthreads for windows cross compiling to work around need for pthreads otherwise
if (CMAKE_SYSTEM_NAME STREQUAL Windows AND CMAKE_CROSSCOMPILING)
	#TODO what if we're using clang?
	set(BUILD_SYSTEM_IS_MINGW On)
	conan_cmake_configure(
		REQUIRES mingw_stdthreads/1.0.1@c74/testing
		GENERATORS cmake_paths
		)
	conan_cmake_install(
		PATH_OR_REFERENCE .
		BUILD missing
		)
	include(${CMAKE_CURRENT_BINARY_DIR}/conan_paths.cmake)
	option(MINGW_STDTHREADS_GENERATE_STDHEADERS "" ON)
	add_subdirectory(${CONAN_MINGW_STDTHREADS_ROOT} ${CMAKE_CURRENT_BINARY_DIR}/mingw_stdthreads)
endif()
