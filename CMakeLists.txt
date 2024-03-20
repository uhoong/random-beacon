cmake_minimum_required(VERSION 3.9)
project(drg)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_definitions(
    -DCURVE_ALT_BN128   
)

include_directories("include/pvss")
include_directories("include")

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_SOURCE_DIR})

add_executable(drg src/DrgBase.cpp src/DrgCore.cpp src/erasure.cpp src/set_up.cpp)

target_link_libraries(drg
    sodium
    ff
    gmp
    gmpxx
    salticidae
    Jerasure
    uv
    ssl
    crypto
)

add_executable(pvss-setup src/pvss_setup.cpp)
target_link_libraries(pvss-setup 
    sodium
    ff
    gmp
    gmpxx
    salticidae
    Jerasure
    uv
    ssl
    crypto
)