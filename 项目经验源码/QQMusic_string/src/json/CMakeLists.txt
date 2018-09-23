
#cmake_minimum_required(VERSION 2.6 FATAL_ERROR)

#cmake_policy(VERSION 2.6)

#project(json)

#include(GNUInstallDirs)

# 1. ENV setup

# 1.1 Include/link path

include_directories("${CMAKE_CURRENT_SOURCE_DIR}/src/json/include")
include_directories("${CMAKE_INSTALL_INCLUDEDIR}")

include_directories("${CMAKE_INSTALL_INCLUDEDIR_UI}")
link_directories("${CMAKE_INSTALL_LIBDIR}")

# 1.2 C/CXX/LD Flags

add_definitions("-std=c++11 -fPIC -Wno-deprecated")

# 2. Build
set(LIBRARY_OUTPUT_PATH "${CMAKE_CURRENT_SOURCE_DIR}/lib")

#SET(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE) 

set(SRC_FILES	
		./src/json_reader.cpp
		./src/json_value.cpp
		./src/json_writer.cpp
)

add_library(json SHARED ${SRC_FILES})


