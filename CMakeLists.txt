cmake_minimum_required(VERSION 3.14)
project(easylibstorage C)
set(CMAKE_C_STANDARD 11)

if (NOT LOGOS_STORAGE_NIM_ROOT)
    message(FATAL_ERROR "libstorage repo not found. Set LOGOS_STORAGE_NIM_ROOT to the folder containing the libstorage repo before running cmake.")
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
    message(FATAL_ERROR "could not find ${LIBSTORAGE_NAMES} under ${LIBSTORAGE_DIR}/build. Make sure to build it before running cmake.")
endif ()

# --- Vendored: inih ---
add_library(inih STATIC vendor/inih/ini.c)
target_include_directories(inih PUBLIC vendor/inih)

# --- Shared library: easystorage ---
add_library(easystorage STATIC
        easystorage.c
        easystorage.h
)

target_include_directories(easystorage PUBLIC
        "${CMAKE_SOURCE_DIR}"
        "${LOGOS_STORAGE_NIM_ROOT}/library"
)

target_link_libraries(easystorage PRIVATE ${LIBSTORAGE_PATH} inih)

# --- Example: storageconsole ---
add_executable(storageconsole
        examples/storageconsole.c
)

target_link_libraries(storageconsole PRIVATE easystorage)
target_link_libraries(storageconsole PRIVATE ${LIBSTORAGE_PATH})

# --- Example: uploader/downloader ---
add_executable(uploader
        examples/uploader.c)

add_executable(downloader
        examples/downloader.c)

target_link_libraries(uploader PRIVATE easystorage)
target_link_libraries(downloader PRIVATE easystorage)
target_link_libraries(uploader PRIVATE ${LIBSTORAGE_PATH})
target_link_libraries(downloader PRIVATE ${LIBSTORAGE_PATH})

# --- Tests ---
enable_testing()

add_executable(test_runner
        tests/test_runner.c
        easystorage.c
        tests/mock_libstorage.c
)

target_link_libraries(test_runner PRIVATE inih)

target_include_directories(test_runner PRIVATE
        "${CMAKE_SOURCE_DIR}"
        "${LOGOS_STORAGE_NIM_ROOT}/library"
)

target_link_libraries(test_runner PRIVATE inih)

add_test(NAME easystorage_tests COMMAND test_runner)
