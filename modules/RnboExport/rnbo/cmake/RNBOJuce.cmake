include_guard(GLOBAL)

set(RNBO_JUCE_VERSION 7.0.3)

if (RNBO_JUCE_USE_CONAN)
	include(${CMAKE_CURRENT_LIST_DIR}/RNBOConan.cmake)

	conan_cmake_configure(
		REQUIRES "JUCE/${RNBO_JUCE_VERSION}@c74/testing"
		GENERATORS cmake_paths
		)
	conan_cmake_autodetect(settings)
	conan_cmake_install(
		PATH_OR_REFERENCE .
		BUILD missing
		SETTINGS ${settings}
		REMOTE cycling-public
		)
	include(${CMAKE_CURRENT_BINARY_DIR}/conan_paths.cmake)
	find_package(JUCE ${RNBO_JUCE_VERSION} CONFIG REQUIRED)
else()
	add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/../../../thirdparty/juce ${CMAKE_BINARY_DIR}/juce)
endif()
