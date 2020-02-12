cmake_minimum_required(VERSION 3.15)
project(FastOutReader)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -ggdb -O3 -Wall")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11  -ggdb -O3 -Wall")

find_package(Boost 1.53.0 REQUIRED)

file(GLOB LIBFST_SRC ${PROJECT_SOURCE_DIR}/fst/fstapi.c 
                     ${PROJECT_SOURCE_DIR}/fst/fastlz.c 
		     ${PROJECT_SOURCE_DIR}/fst/lz4.c)
file(GLOB LIBFST_HDR ${PROJECT_SOURCE_DIR}/fst/fstapi.h
                     ${PROJECT_SOURCE_DIR}/fst/fastlz.h
		     ${PROJECT_SOURCE_DIR}/fst/lz4.h)
add_library(fst ${LIBFST_SRC} ${LIBFST_HDR})

#FIXME: Add example program main
#add_executable(fst.exe ${LIBFST_SRC})
#target_link_libraries(fst.exe PUBLIC fst z)

include_directories(${PROJECT_SOURCE_DIR})
file(GLOB SRC_FILES ${PROJECT_SOURCE_DIR}/*.cpp ${PROJECT_SOURCE_DIR}/*.c)
add_executable(fastread ${SRC_FILES})
target_include_directories(fastread PUBLIC ${PROJECT_SOURCE_DIR} ${PROJECT_SOURCE_DIR}/fst)
target_link_libraries(fastread fst z)
if(Boost_FOUND)
    target_include_directories(fastread PUBLIC ${Boost_INCLUDE_DIRS})
    target_link_libraries(fastread ${Boost_LIBRARIES})
endif()
