enable_testing()
include(GoogleTest)

function(add_gtest test_name)
    add_executable(${test_name} ${ARGN})
    target_link_libraries(${test_name} gtest gtest_main)
    gtest_discover_tests(${test_name})
endfunction()

add_gtest(test_BBL_DS test_BBL_DS.cpp)
add_gtest(test_utils test_utils.cpp ../include/utils/graph_utils.hpp)
