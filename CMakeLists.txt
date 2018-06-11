cmake_minimum_required(VERSION 3.0)

project(zxing)

option(BUILD_TESTING "Enable generation of test targets" OFF)

set(CMAKE_LIBRARY_PATH /opt/local/lib ${CMAKE_LIBRARY_PATH})

# Check for polluted source tree.
if(EXISTS ${CMAKE_SOURCE_DIR}/CMakeCache.txt OR
    EXISTS ${CMAKE_SOURCE_DIR}/CMakeFiles)
    message(FATAL_ERROR
        "Source directory is polluted:"
        "\n  * remove CMakeCache.txt"
        "\n  * remove CMakeFiles directory")
endif()

# Suppress in-source builds.
if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_BINARY_DIR})
    message(FATAL_ERROR
        "CMake generation is not allowed within the source directory:"
        "\n  * mkdir build"
        "\n  * cd build"
        "\n  * Unix-like: cmake -G \"Unix Makefiles\" .."
        "\n  * Windows: cmake -G \"Visual Studio 10\" ..")
endif()

# Adjust CMake's module path.
list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/FindModules)

# Suppress MSVC CRT warnings.
if(MSVC)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
    add_definitions(/Za)
    add_definitions(/MP) # multi-core builds
endif()

include(source_files.cmake)

set(CMAKE_CXX_FLAGS "-std=c++11 -Os -s DEMANGLE_SUPPORT=1 --bind -s RESERVED_FUNCTION_POINTERS=20 -s EXPORT_NAME=\"'ZXing'\" -s WASM=0 -s SINGLE_FILE=1 -s MODULARIZE=1 -s DISABLE_EXCEPTION_CATCHING=0 --memory-init-file 0 -s \"EXTRA_EXPORTED_RUNTIME_METHODS=['addFunction']\" -s EXPORTED_FUNCTIONS=\"['_resize', '_decode_any']\"")

# Add libzxing library.
include_directories(core/src)
add_library(libzxing ${LIBZXING_FILES})
set_target_properties(libzxing PROPERTIES PREFIX "")

find_package(Iconv)
if(ICONV_FOUND)
    include_directories(${ICONV_INCLUDE_DIR})
    target_link_libraries(libzxing ${ICONV_LIBRARIES})
else()
    add_definitions(-DNO_ICONV=1)
endif()

# Add cli executable.
file(GLOB_RECURSE ZXING_FILES
    "./cli/src/*.h"
    "./cli/src/ImageReaderSource.cpp"
    "./cli/zxing.js.cpp"
)
add_executable(zxing ${ZXING_FILES})
target_link_libraries(zxing libzxing)

install(TARGETS zxing libzxing EXPORT zxing-targets
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
    ARCHIVE DESTINATION lib
    INCLUDES DESTINATION include
)

install(EXPORT zxing-targets DESTINATION lib/zxing/cmake NAMESPACE zxing::)

install(
    DIRECTORY core/src/zxing/
    DESTINATION include/zxing
    FILES_MATCHING PATTERN "*.h"
)

configure_file(cmake/zxing-config.cmake.in zxing-config.cmake @ONLY)
install(FILES ${CMAKE_BINARY_DIR}/zxing-config.cmake DESTINATION lib/zxing/cmake)