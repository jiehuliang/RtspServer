include_directories(../3rdpart)

file(GLOB RtspServer_src_list ./*.cpp ./*.h)

add_executable(RtspServer ${RtspServer_src_list})

if (CMAKE_SYSTEM_NAME MATCHES "Linux")
	target_link_libraries(RtspServer -Wl,--start-group ${LINK_LIB_LIST} -Wl,--end-group)
else ()
	target_link_libraries(RtspServer  ${LINK_LIB_LIST})
endif ()

message(STATUS "${LINK_LIB_LIST}")