#setup conan, include the module paths, setup the remote
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_BINARY_DIR})
list(APPEND CMAKE_PREFIX_PATH ${CMAKE_CURRENT_BINARY_DIR})

include_guard(GLOBAL)

#cloud compiler might have already included conan
if (NOT COMMAND conan_check)
	include(${CMAKE_CURRENT_LIST_DIR}/conan.cmake)
endif()

conan_check(VERSION 1.29.0 REQUIRED)
conan_add_remote(
	NAME cycling-public
	INDEX 1
	URL https://conan-public.cycling74.com
	VERIFY_SSL True
)
