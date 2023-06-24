# Include the rnbo directory to the target
target_include_directories(${TARGET_NAME} PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/rnbo
		${CMAKE_CURRENT_SOURCE_DIR}/rnbo/common
)

# Make a variable in the PARENT_SCOPE with the RNBO files to add
set(RNBO_SOURCES    ${CMAKE_CURRENT_SOURCE_DIR}/rnbo/RNBO.h
                    ${CMAKE_CURRENT_SOURCE_DIR}/rnbo/RNBO.cpp
                    ${CMAKE_CURRENT_SOURCE_DIR}/rnbo_granular.cpp PARENT_SCOPE)