cmake_minimum_required(VERSION 3.15)
project(FastOutReader)

include(ExternalProject)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -ggdb -O3 -Wall")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ggdb -O3 -Wall")

find_package(Boost 1.53.0 REQUIRED COMPONENTS program_options filesystem)

ExternalProject_Add(rxterm
    GIT_REPOSITORY    git@github.com:olehzdubna/rxterm.git
    BUILD_IN_SOURCE true
    PREFIX ${CMAKE_BINARY_DIR}
    INSTALL_COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_BINARY_DIR}/src/rxterm/rxterm/include ${CMAKE_SOURCE_DIR}/rxterm-headers
)

if(${BUILD_GTKWAVE})
    ExternalProject_Add(gtkwave
       GIT_REPOSITORY    git@github.com:maxsteciuk/gtkwave.git
       GIT_SUBMODULES ""
       INSTALL_COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_BINARY_DIR}/gtkwave-prefix/src/gtkwave-build/gtkwave3/gtkwave3 ${CMAKE_BINARY_DIR}/
    )
endif()
 

file(GLOB LIBFST_SRC ${PROJECT_SOURCE_DIR}/fst/fstapi.c 
                     ${PROJECT_SOURCE_DIR}/fst/fastlz.c 
		     ${PROJECT_SOURCE_DIR}/fst/lz4.c)
file(GLOB LIBFST_HDR ${PROJECT_SOURCE_DIR}/fst/fstapi.h
                     ${PROJECT_SOURCE_DIR}/fst/fastlz.h
		     ${PROJECT_SOURCE_DIR}/fst/lz4.h)
add_library(fst ${LIBFST_SRC} ${LIBFST_HDR})

include_directories(${PROJECT_SOURCE_DIR})
file(GLOB LIBFASTREAD_SRC ${PROJECT_SOURCE_DIR}/*.cpp)
list(FILTER LIBFASTREAD_SRC EXCLUDE REGEX "ReadFastOut.cpp")
file(GLOB FASTREAD_SRC ${PROJECT_SOURCE_DIR}/ReadFastOut.cpp)
add_library(libfastread ${LIBFASTREAD_SRC})

add_executable(fastread ${FASTREAD_SRC})
add_dependencies(fastread rxterm libfastread)

if(${BUILD_GTKWAVE})
    add_dependencies(fastread gtkwave)
endif()

target_include_directories(fastread PUBLIC 
                          ${PROJECT_SOURCE_DIR} 
			  ${PROJECT_SOURCE_DIR}/fst
			  ${PROJECT_SOURCE_DIR}/rxterm-headers
)
target_link_libraries(fastread fst z pthread libfastread)
if(Boost_FOUND)
    target_include_directories(libfastread PUBLIC ${Boost_INCLUDE_DIRS})
    target_include_directories(fastread PUBLIC ${Boost_INCLUDE_DIRS})
    target_link_libraries(libfastread ${Boost_LIBRARIES})
    target_link_libraries(fastread ${Boost_LIBRARIES})
endif()
