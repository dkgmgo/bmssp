enable_testing()
add_executable(test_BBL_DS test_BBL_DS.cpp)
target_link_libraries(test_BBL_DS
        gtest
        gtest_main
)

include(GoogleTest)
gtest_discover_tests(test_BBL_DS)

add_executable(test_RBT test_RBT.cpp)
target_link_libraries(test_RBT
        gtest
        gtest_main
)

include(GoogleTest)
gtest_discover_tests(test_RBT)

add_executable(test_utils test_utils.cpp ../utils.cpp)
target_link_libraries(test_utils
        gtest
        gtest_main
)

include(GoogleTest)
gtest_discover_tests(test_utils)