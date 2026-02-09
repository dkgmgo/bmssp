cmake_minimum_required(VERSION 4.0)
project(BMSSP_Project)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_BUILD_TYPE Release)

find_package(Boost REQUIRED COMPONENTS graph)

include(FetchContent)
FetchContent_Declare(
        googlebenchmark
        GIT_REPOSITORY https://github.com/google/benchmark.git
        GIT_TAG v1.9.5
)
set(BENCHMARK_ENABLE_TESTING OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googlebenchmark)

add_executable(main main.cpp)
target_include_directories(main PRIVATE . include data_structures)
target_link_libraries(main PRIVATE Boost::graph)

add_executable(bench_it apps/benchmark.cpp)
target_include_directories(bench_it PRIVATE . include data_structures)
target_link_libraries(bench_it PRIVATE benchmark::benchmark Boost::graph)

FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/refs/heads/main.zip
        DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)

set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)
add_subdirectory(tests)
