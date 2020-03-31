include_directories(
	"${PROJECT_ROOT_PATH}/googletest/googletest/include/"
	"${PROJECT_ROOT_PATH}/googletest/googlemock/include/"
	"${PROJECT_ROOT_PATH}/"
)          

link_directories(
	"${PROJECT_ROOT_PATH}/googletest/googletest/build-aux/"
	"${PROJECT_ROOT_PATH}/googletest/googlemock/build-aux/"
    "${LIBRARY_OUTPUT_PATH}/"
)

add_executable(test_hashmap test_hashmap.cpp)
add_executable(test_skiplist test_skiplist.cpp)

target_link_libraries(test_hashmap gtest gmock pthread data)
target_link_libraries(test_skiplist gtest gmock pthread data)


add_test(NAME test_hashmap COMMAND test_hashmap)
add_test(NAME test_skiplist COMMAND test_skiplist)
