cmake_minimum_required(VERSION 3.15)
project(FastOutReader)

file(GLOB LIBFST_SRC ${PROJECT_SOURCE_DIR}/fst/fstapi.c 
                     ${PROJECT_SOURCE_DIR}/fst/fastlz.c 
		     ${PROJECT_SOURCE_DIR}/fst/lz4.c)
file(GLOB LIBFST_HDR ${PROJECT_SOURCE_DIR}/fst/fstapi.h
                     ${PROJECT_SOURCE_DIR}/fst/fastlz.h
		     ${PROJECT_SOURCE_DIR}/fst/lz4.h)
add_library(fst ${LIBFST_SRC} ${LIBFST_HDR})

#TODO: Add example program
add_executable(fst.exe ${LIBFST_SRC})
target_link_libraries(fst.exe PUBLIC fst z)

#FIXME: SharedObject.h is missing
include_directories(${PROJECT_SOURCE_DIR})
file(GLOB SRC_FILES ${PROJECT_SOURCE_DIR}/*.cpp ${PROJECT_SOURCE_DIR}/*.c)
add_executable(fastread ${SRC_FILES})
