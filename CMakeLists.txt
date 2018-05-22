# quick and dirty CMakeLists.txt

cmake_minimum_required(VERSION 3.11)
project(libmpc VERSION 1.0.0 LANGUAGES CXX C)

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fsanitize=address -fno-omit-frame-pointer")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -mtune=native")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -fomit-frame-pointer")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Ofast")

add_library(party
    src/curve25519/mycurve25519.c
    src/network/dummy_connection.cpp
    src/network/tcp_connection.cpp
    src/ot/ot_hl17.cpp
    src/util/util.c
    src/util/util.cpp
    src/util/options.cpp
)
target_compile_features(party PUBLIC cxx_std_17)
target_include_directories(party PRIVATE src)
target_include_directories(party PRIVATE /usr/include/botan-2)
target_link_libraries(party botan-2)
target_link_libraries(party boost_program_options boost_system)
target_link_libraries(party pthread)

add_executable(baseOT bin/baseOT.cpp)
target_include_directories(baseOT PRIVATE src)
target_link_libraries(baseOT party)

add_executable(test test/test.cpp test/test_curve25519.cpp test/test_ot_hl17.cpp)
target_include_directories(test PRIVATE src)
target_link_libraries(test party)
target_link_libraries(test gtest)

add_executable(bench bench/bench.cpp bench/bench_ot_hl17.cpp bench/bench_hash.cpp)
target_include_directories(bench PRIVATE src)
target_include_directories(bench PRIVATE /usr/include/botan-2)
target_link_libraries(bench party)
target_link_libraries(bench benchmark botan-2)
