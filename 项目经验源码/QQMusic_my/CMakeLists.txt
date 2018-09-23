
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
#include_directories("${CMAKE_PROJECT_ROOT}/src/json/include")
#link_directories(${COMMONAPI_LIBDIR})

include_directories("src")
include_directories("src/svpinclude")
#include_directories("src/json/include")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11  -D_GLIBCXX_USE_NANOSLEEP")

set(CMAKE_LD_FLAGS "${CMAKE_LD_FLAGS} -m32")

add_definitions("-DLINUX_PC -DLIBSVTOOLS")

# 3. Project Build

set(APP_NAME "QQMusicMy")

set(APP_SRC_FILES src/SVPMain.cpp
									src/json/src/Utils.cpp
									src/json/src/json_reader.cpp
									src/json/src/json_value.cpp
									src/json/src/json_writer.cpp
									src/NetMgr/NetMgr.cpp
									src/NetMgr/DataMgr.cpp
									src/QQData.cpp
									src/SVPPcmPaSimple.cpp
#									src/LoopCenter.cpp
#									src/wav.cpp
    )

add_executable(${APP_NAME} ${APP_SRC_FILES})

target_link_libraries(${APP_NAME} pthread asound m rt dl PocoNet PocoFoundation pulse pulse-simple)


# 4. Project Install

install(TARGETS ${APP_NAME}
        RUNTIME DESTINATION ${CMAKE_CURRENT_SOURCE_DIR}/bin
        LIBRARY DESTINATION ${CMAKE_CURRENT_SOURCE_DIR}/bin
        ARCHIVE DESTINATION ${CMAKE_CURRENT_SOURCE_DIR}/bin)
