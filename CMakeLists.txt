cmake_minimum_required(VERSION 3.14)
project(storageconsole C)
set(CMAKE_C_STANDARD 11)

add_executable(storageconsole
        main.c
        easylibstorage.c
        easylibstorage.h
)

if(NOT DEFINED LOGOS_STORAGE_NIM_ROOT)
    message(FATAL_ERROR "Need to set LOGOS_STORAGE_NIM_ROOT")
endif()

target_include_directories(storageconsole PRIVATE "${LOGOS_STORAGE_NIM_ROOT}/library")

if(APPLE)
    set(LIBSTORAGE_NAMES libstorage.dylib)
elseif(WIN32)
    set(LIBSTORAGE_NAMES libstorage.dll)
else()
    set(LIBSTORAGE_NAMES libstorage.so)
endif()

find_library(LIBSTORAGE_PATH NAMES ${LIBSTORAGE_NAMES} PATHS ${LOGOS_STORAGE_NIM_ROOT}/build NO_DEFAULT_PATH)

if(LIBSTORAGE_PATH)
    target_link_libraries(storageconsole PRIVATE ${LIBSTORAGE_PATH})
else()
    message(WARNING "libstorage not found. Build or provide it before linking.")
endif()

# --- Tests ---
enable_testing()

add_executable(test_runner
        tests/test_runner.c
        easylibstorage.c
        tests/mock_libstorage.c
)

target_include_directories(test_runner PRIVATE
        "${CMAKE_SOURCE_DIR}"
        "${LOGOS_STORAGE_NIM_ROOT}/library"
)

add_test(NAME easylibstorage_tests COMMAND test_runner)
