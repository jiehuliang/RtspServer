cmake_minimum_required(VERSION 3.8)
project(RtspPlayerContextTests)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_BUILD_TYPE Debug)

set(PROJECT_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/..)

include_directories(
    ${PROJECT_ROOT}/src
    ${PROJECT_ROOT}/3rdpart/NWTool/src
)

add_executable(test_rtsp_player_context 
    test_rtsp_player_context.cpp
    ${PROJECT_ROOT}/src/Rtsp/RtspPlayerContext.cpp
)

target_link_libraries(test_rtsp_player_context 
    ${PROJECT_ROOT}/3rdpart/NWTool/bin/libNWTool.so
    pthread
)

set_target_properties(test_rtsp_player_context PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${PROJECT_ROOT}/bin
)
