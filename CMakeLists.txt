cmake_minimum_required(VERSION 3.12)
project(air_license LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin)
set(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR}/lib)

include_directories(${CMAKE_SOURCE_DIR}/include)

add_library(air_license INTERFACE)

target_include_directories(air_license INTERFACE $<INSTALL_INTERFACE:include>)

install(DIRECTORY ${CMAKE_SOURCE_DIR}/include/air
    	DESTINATION include
        FILES_MATCHING
        PATTERN *.hpp
)