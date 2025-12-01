cmake_minimum_required(VERSION 4.0)
project(main)

set(CMAKE_CXX_STANDARD 14)

include_directories(.)
find_package( Boost COMPONENTS graph)

add_executable(main
        utils.cpp
        1.cpp
        2.cpp
        3.cpp
        main.cpp
        RBT.hpp
        BBL_DS.hpp
)
target_link_libraries(main PRIVATE ${Boost_LIBRARIES})

include(FetchContent)
FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/refs/heads/main.zip
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)
add_subdirectory(tests)

