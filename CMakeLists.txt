cmake_minimum_required(VERSION 3.14)
project(easylibstorage C)
set(CMAKE_C_STANDARD 11)

if (NOT DEFINED LOGOS_STORAGE_NIM_ROOT)
    message(FATAL_ERROR "Need to set LOGOS_STORAGE_NIM_ROOT")
endif ()

# --- Find libstorage ---
if (APPLE)
    set(LIBSTORAGE_NAMES libstorage.dylib)
elseif (WIN32)
    set(LIBSTORAGE_NAMES libstorage.dll)
else ()
    set(LIBSTORAGE_NAMES libstorage.so)
endif ()

find_library(LIBSTORAGE_PATH NAMES ${LIBSTORAGE_NAMES} PATHS ${LOGOS_STORAGE_NIM_ROOT}/build NO_DEFAULT_PATH)

if (NOT LIBSTORAGE_PATH)
    message(WARNING "libstorage not found. Build or provide it before linking.")
endif ()

# --- Shared library: easystorage ---
add_library(easystorage SHARED
        easystorage.c
        easystorage.h
)

target_include_directories(easystorage PUBLIC
        "${CMAKE_SOURCE_DIR}"
        "${LOGOS_STORAGE_NIM_ROOT}/library"
)

if (LIBSTORAGE_PATH)
    target_link_libraries(easystorage PRIVATE ${LIBSTORAGE_PATH})
endif ()

# --- Example: storageconsole ---
add_executable(storageconsole
        examples/storageconsole.c
)

target_link_libraries(storageconsole PRIVATE easystorage)

if (LIBSTORAGE_PATH)
    target_link_libraries(storageconsole PRIVATE ${LIBSTORAGE_PATH})
endif ()

# --- Example: uploader/downloader ---
add_executable(uploader
        examples/uploader.c)

add_executable(downloader
        examples/downloader.c)

target_link_libraries(uploader PRIVATE easystorage)
target_link_libraries(downloader PRIVATE easystorage)

if (LIBSTORAGE_PATH)
    target_link_libraries(uploader PRIVATE ${LIBSTORAGE_PATH})
    target_link_libraries(downloader PRIVATE ${LIBSTORAGE_PATH})
endif ()

# --- Tests ---
enable_testing()

add_executable(test_runner
        tests/test_runner.c
        easystorage.c
        tests/mock_libstorage.c
)

target_include_directories(test_runner PRIVATE
        "${CMAKE_SOURCE_DIR}"
        "${LOGOS_STORAGE_NIM_ROOT}/library"
)

add_test(NAME easystorage_tests COMMAND test_runner)
