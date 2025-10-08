include_directories(../3rdpart)

file(GLOB RtspClient_src_list ./*.cpp ./*.h)

add_executable(RtspClient ${RtspClient_src_list})

if (CMAKE_SYSTEM_NAME MATCHES "Linux")
	target_link_libraries(RtspClient -Wl,--start-group ${LINK_LIB_LIST} -Wl,--end-group)
else ()
	target_link_libraries(RtspClient  ${LINK_LIB_LIST})
endif ()

message(STATUS "${LINK_LIB_LIST}")