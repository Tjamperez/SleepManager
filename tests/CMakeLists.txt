find_package(GTest REQUIRED)
include_directories(${GTEST_INCLUDE_DIRS})
add_executable(discovery_test discovery_test.cpp ../src/discovery.cpp)
target_link_libraries(discovery_test gtest_main)

include(GoogleTest)
gtest_discover_tests(discovery_test)
target_link_libraries(discovery_test ${GTEST_LIBRARIES} pthread)