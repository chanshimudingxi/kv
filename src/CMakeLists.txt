INCLUDE_DIRECTORIES(
    "${PROJECT_ROOT_PATH}/deps"
)   

add_subdirectory(proto)

set(CLI_SRC_LIST cs_client.cpp testcli.cpp)
set(GWD_SRC_LIST ss_client.cpp gateway.cpp)
set(NAME_SRC_LIST namesvr.cpp)

add_executable(cli ${CLI_SRC_LIST})
add_executable(gwd ${GWD_SRC_LIST})
add_executable(named ${NAME_SRC_LIST})


target_link_libraries(cli proto net data)
target_link_libraries(gwd proto net data)
target_link_libraries(named proto net data)
