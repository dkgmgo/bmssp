cmake_minimum_required(VERSION 4.0)
project(main)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_BUILD_TYPE Release)

include_directories(.)
find_package(Boost 1.82 REQUIRED COMPONENTS graph)

SET(INCLUDE_DIR include/bmssp.hpp include/dijkstras.hpp include/utils/graph_utils.hpp include/utils/file_utils.hpp include/common.hpp)
SET(OTHER_DIRS data_structures/BBL_DS.hpp apps/runner.hpp apps/benchmark.hpp)
add_executable(main main.cpp ${INCLUDE_DIR} ${OTHER_DIRS})
target_link_libraries(main PRIVATE ${Boost_LIBRARIES})

include(FetchContent)
FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/refs/heads/main.zip
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)
add_subdirectory(tests)

