cmake_minimum_required(VERSION 3.15)
project(FastOutReader)

set(BOOST_HDR_PATH /usr/local/include/)
set(BOOST_LIB_PATH /usr/local/lib/)

#FIXME: Fix Boost library dependencies 
#find_package(Boost 1.70.0 COMPONENTS ${BOOST_LIB_PATH} ${BOOST_HDR_PATH})

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
target_include_directories(fastread PUBLIC ${PROJECT_SOURCE_DIR} ${BOOST_HDR_PATH} ${PROJECT_SOURCE_DIR}/fst)
target_link_libraries(fastread fst z)
#FIXME: Fix Boost library dependencies 
#if(Boost_FOUND)
#    target_link_libraries(fastread ${Boost_LIBRARIES})
#endif()
