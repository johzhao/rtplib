file(READ ${CMAKE_SOURCE_DIR}/version.txt VERSION_IN_FILE)

set(APP_VERSION "${VERSION_IN_FILE}")

find_package(Git QUIET)

if (GIT_FOUND)
    execute_process(
            COMMAND ${GIT_EXECUTABLE} describe --tags --dirty --abbrev=8 --always
            WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
            OUTPUT_VARIABLE GIT_VERSION
            OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    set(COMMIT ${GIT_VERSION})
else()
    set(COMMIT "unknown")
endif()

string(TIMESTAMP BUILD_DATE "%Y%m%d")

message(STATUS "App version: ${APP_VERSION}")
message(STATUS "Commit: ${COMMIT}")
message(STATUS "Build date: ${BUILD_DATE}")

add_compile_definitions(
        APP_VERSION=\"${APP_VERSION}\"
        COMMIT=\"${COMMIT}\"
        BUILD_DATE=\"${BUILD_DATE}\"
)
