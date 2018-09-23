
####################################################################################################

# Company       : Huizhou Desay SV Automotive Co., Ltd.

# Division      : Automotive Electronics, Desay Corporation

# Business Unit : Central Technology

# Department    : Advanced Development (Huizhou)

####################################################################################################

# 0. CMake Defs

cmake_minimum_required(VERSION 2.6 FATAL_ERROR)

cmake_policy(VERSION 2.6)

# 1. Project Name

project(app)

# 2. Project Env (Include/Lib Path, C/CXX/LD FLAGS)

include_directories("src")
include_directories("src/svpinclude")
#include_directories("src/json/include")

link_directories("/usr/local/lib")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -D_GLIBCXX_USE_NANOSLEEP")

add_definitions("-DLINUX_PC -DLIBSVTOOLS")

# 3. Project Build

set(APP_NAME "localsocketclient")

set(APP_SRC_FILES src/SVPMain.cpp
                  src/QQTcp.cpp
                  src/QQUdp.cpp
                  src/QQData.cpp
                  src/QQDataMgr.cpp
                  src/SVPTime.cpp
				  src/SVPPcmPaSimple.cpp
                  src/json/src/Utils.cpp
                  src/json/src/json_reader.cpp
                  src/json/src/json_value.cpp
                  src/json/src/json_writer.cpp
)

add_executable(${APP_NAME} ${APP_SRC_FILES})

#target_link_libraries(${APP_NAME} ${DATA_LIB_NAME})
target_link_libraries(${APP_NAME} pthread m rt dl PocoFoundation pulse pulse-simple)


# 4. Project Install

install(TARGETS ${APP_NAME}
        RUNTIME DESTINATION ${CMAKE_CURRENT_SOURCE_DIR}/bin
        LIBRARY DESTINATION ${CMAKE_CURRENT_SOURCE_DIR}/bin
        ARCHIVE DESTINATION ${CMAKE_CURRENT_SOURCE_DIR}/bin)
