if(NOT DEFINED SOURCE_DIR)
    message(FATAL_ERROR "SOURCE_DIR is not set")
endif()

if(NOT DEFINED OUTPUT_FILE)
    message(FATAL_ERROR "OUTPUT_FILE is not set")
endif()

set(GIT_COMMIT_HASH "unknown")
set(GIT_COMMIT_DATE_FORMATTED "unknown")

find_program(GIT_EXECUTABLE git)

if(GIT_EXECUTABLE)
    execute_process(
        COMMAND ${GIT_EXECUTABLE} rev-parse --short HEAD
        WORKING_DIRECTORY ${SOURCE_DIR}
        OUTPUT_VARIABLE GIT_COMMIT_HASH
        ERROR_QUIET
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )

    execute_process(
        COMMAND ${GIT_EXECUTABLE} log -1 --format=%ci
        WORKING_DIRECTORY ${SOURCE_DIR}
        OUTPUT_VARIABLE GIT_COMMIT_DATE
        ERROR_QUIET
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )

    string(LENGTH "${GIT_COMMIT_DATE}" GIT_COMMIT_DATE_LENGTH)
    if(GIT_COMMIT_DATE_LENGTH GREATER 9)
        string(SUBSTRING "${GIT_COMMIT_DATE}" 0 10 GIT_COMMIT_DATE_DAY)
        set(GIT_COMMIT_DATE_FORMATTED "${GIT_COMMIT_DATE_DAY}")
    endif()
endif()

configure_file(
    ${SOURCE_DIR}/cmake/BuildInfo.h.in
    ${OUTPUT_FILE}
    @ONLY
)
