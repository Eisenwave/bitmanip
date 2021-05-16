cmake_minimum_required(VERSION 3.5)

project(bitmanip LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(HEADER_DIR include/bitmanip)
set(TEST_DIR test)

add_executable(bitmanip_test
    ${TEST_DIR}/main.cpp
    ${TEST_DIR}/test.cpp
    ${TEST_DIR}/test.hpp
    ${TEST_DIR}/assert.cpp
    ${TEST_DIR}/assert.hpp

    ${HEADER_DIR}/all.hpp
    ${HEADER_DIR}/build.hpp
    ${HEADER_DIR}/builtin.hpp

    ${HEADER_DIR}/bit.hpp
    ${HEADER_DIR}/bitcount.hpp
    ${HEADER_DIR}/bitileave.hpp
    ${HEADER_DIR}/bitrev.hpp
    ${HEADER_DIR}/bitrot.hpp

    ${HEADER_DIR}/intdiv.hpp
    ${HEADER_DIR}/intlog.hpp)

target_include_directories(bitmanip_test PUBLIC include/)
    
