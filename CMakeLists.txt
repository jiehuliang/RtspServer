﻿# CMakeList.txt: cppserverstudio 的 CMake 项目，在此处包括源代码并定义
# 项目特定的逻辑。
#
cmake_minimum_required (VERSION 3.8)


project ("RtspServer")

# 使能 C++11
# Enable C++11
set(CMAKE_CXX_STANDARD 11)

# 设置编译类型为 Debug（自动添加 -g）
set(CMAKE_BUILD_TYPE Debug)

# 针对 Debug 模式添加额外的编译选项
set(CMAKE_CXX_FLAGS_DEBUG "-g -O0")  # -g: 生成调试信息, -O0: 禁用优化

#设置bin和lib库目录
set(RELEASE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/release)

if (CMAKE_SYSTEM_NAME MATCHES "Linux")
    SET(LIBRARY_OUTPUT_PATH ${RELEASE_DIR}/linux/${CMAKE_BUILD_TYPE})
    SET(EXECUTABLE_OUTPUT_PATH ${RELEASE_DIR}/linux/${CMAKE_BUILD_TYPE})
    execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink ${CMAKE_CURRENT_SOURCE_DIR}/bin/2.264 ${EXECUTABLE_OUTPUT_PATH}/2.264)
endif()

link_directories(${LIBRARY_OUTPUT_PATH})

#设置工程源码根目录
set(NWTool_Root ${CMAKE_CURRENT_SOURCE_DIR}/3rdpart/NWTool/src)
set(Media_Root ${CMAKE_CURRENT_SOURCE_DIR}/src)

#设置头文件目录
include_directories(${NWTool_Root})
include_directories(${Media_Root})

set(LINK_LIB_LIST Media NWTool)

#收集源代码
file(GLOB NWTool_src_list ${NWTool_Root}/*/*.cpp ${NWTool_Root}/*/*.h ${NWTool_Root}/*/*.c)
file(GLOB Media_src_list ${Media_Root}/*/*.cpp ${Media_Root}/*/*.h ${Media_Root}/*/*.c)

#添加库
add_library(NWTool STATIC ${NWTool_src_list})
add_library(Media STATIC ${Media_src_list})


if(WIN32)
    list(APPEND LINK_LIB_LIST WS2_32 Iphlpapi shlwapi)
else()
    list(APPEND LINK_LIB_LIST pthread)
endif()

add_subdirectory(server)

# Expected directory structure.
set(PINE_BUILD_SUPPORT_DIR "${CMAKE_SOURCE_DIR}/build_support")
set(PINE_CLANG_SEARCH_PATH "/usr/bin/" "/usr/local/bin" "/usr/lib/llvm-18/bin/" "/usr/local/opt/llvm/bin" "/usr/local/opt/llvm-18/bin" "/usr/local/Cellar/llvm/8.0.1/bin")
######################################################################################################################
# DEPENDENCIES
######################################################################################################################

# CTest
# enable_testing()

# clang-format
if (NOT DEFINED CLANG_FORMAT_BIN)
    # attempt to find the binary if user did not specify
    find_program(CLANG_FORMAT_BIN
            NAMES clang-format clang-format-18
            HINTS ${PINE_CLANG_SEARCH_PATH})
endif ()
if ("${CLANG_FORMAT_BIN}" STREQUAL "CLANG_FORMAT_BIN-NOTFOUND")
    message(WARNING "Pine/main couldn't find clang-format.")
else ()
    message(STATUS "Pine/main found clang-format at ${CLANG_FORMAT_BIN}")
endif ()

# clang-tidy
if (NOT DEFINED CLANG_TIDY_BIN)
    # attempt to find the binary if user did not specify
    find_program(CLANG_TIDY_BIN
            NAMES clang-tidy clang-fidy-18
            HINTS ${PINE_CLANG_SEARCH_PATH})
endif ()
if ("${CLANG_TIDY_BIN}" STREQUAL "CLANG_TIDY_BIN-NOTFOUND")
    message(WARNING "Pine/main couldn't find clang-tidy.")
else ()
    # Output compile_commands.json
    set(CMAKE_EXPORT_COMPILE_COMMANDS 1)
    message(STATUS "Pine/main found clang-fidy at ${CLANG_TIDY_BIN}")
endif ()

# cpplint
find_program(CPPLINT_BIN
        NAMES cpplint cpplint.py
        HINTS ${PINE_BUILD_SUPPORT_DIR})
if ("${CPPLINT_BIN}" STREQUAL "CPPLINT_BIN-NOTFOUND")
    message(WARNING "Pine/main couldn't find cpplint.")
else ()
    message(STATUS "Pine/main found cpplint at ${CPPLINT_BIN}")
endif ()

#set(CMAKE_MAKE_PROGRAM "/usr/bin/ninja")

##########################################
# "make format"
# "make check-format"
##########################################
string(CONCAT PINE_FORMAT_DIRS
        "${CMAKE_CURRENT_SOURCE_DIR}/src,"
        )

# runs clang format and updates files in place.
add_custom_target(format ${PINE_BUILD_SUPPORT_DIR}/run_clang_format.py
        ${CLANG_FORMAT_BIN}
        ${PINE_BUILD_SUPPORT_DIR}/clang_format_exclusions.txt
        --source_dirs
        ${PINE_FORMAT_DIRS}
        --fix
        --quiet
        )

##########################################
# "make cpplint"
##########################################

file(GLOB_RECURSE PINE_LINT_FILES
        "${CMAKE_CURRENT_SOURCE_DIR}/src/*.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
        )

add_custom_target(cpplint echo '${SRC_LIST}' | xargs -n12 -P8
        ${CPPLINT_BIN}
        --verbose=2 --quiet
        --linelength=120
        --filter=-legal/copyright,-build/include_subdir,-readability/casting
        )

add_custom_target(clang-tidy
        ${PINE_BUILD_SUPPORT_DIR}/run_clang_tidy.py # run LLVM's clang-tidy script
        -clang-tidy-binary ${CLANG_TIDY_BIN}        # using our clang-tidy binary
        -p ${CMAKE_BINARY_DIR}                      # using cmake's generated compile commands
        )


# TODO: 如有需要，请添加测试并安装目标。
