cmake_minimum_required(VERSION 3.0.0)

option (USE_CROSS_COMPILE "Use Cross-compile" ON)
if(USE_CROSS_COMPILE)
  set(CMAKE_SYSROOT /opt/cross-compile/ubuntu-16.04-arm64)
  set(CMAKE_TOOLCHAIN_FILE ${CMAKE_SYSROOT}/toolchain.cmake)
endif(USE_CROSS_COMPILE)

set(ENABLE_COVERAGE FALSE)

if(ENABLE_COVERAGE)

    message(STATUS "Use gcov for code coverage test")

    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fprofile-arcs -ftest-coverage")

endif()

project(motion_control VERSION 0.0.1)
if(NOT DEFINED DEBIAN_VERSION)
  set(DEBIAN_VERSION ${PROJECT_VERSION})
endif()

## Compile as C++11, supported in ROS Kinetic and newer
add_definitions(-DSPDLOG_COMPILED_LIB)
add_compile_options(-std=c++14
  -pthread
  -Wl,-rpath=/usr/local/bzl_robot/lib
  -Werror
  -Wswitch
  -Wuninitialized
  -Wno-return-local-addr
  -Wreturn-type)
add_link_options(-Wl,-rpath,/usr/local/bzl_robot/lib)
include(cmake/common.cmake)

## System dependencies are found with CMake's conventions
find_package(Boost REQUIRED COMPONENTS system filesystem)

## Protobuf
#find_package(Protobuf REQUIRED)
  #protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS
  #proto/chatter.proto
  #proto/Dev_control.proto
  #proto/Hoister.proto
  #proto/SpiStm32.proto
  #proto/String.proto
  #proto/FaultMsg.proto
#)

## Bros
find_package(brclcpp REQUIRED)
find_package(brclcpp_action REQUIRED)
find_package(message_filters REQUIRED)
find_package(bros_stdmsg REQUIRED)

###########
## Build ##
###########

## Specify additional locations of header files
## Your package locations should be listed before other locations

include_directories(
  include
  ${Boost_INCLUDE_DIRS}
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_PREFIX_PATH}/include/common_msgs
  ${CMAKE_PREFIX_PATH}/include/common_msgs/geometry_msgs
  ${CMAKE_PREFIX_PATH}/include/common_msgs/nav_msgs
  ${CMAKE_PREFIX_PATH}/include/common_msgs/sensor_msgs
  ${CMAKE_PREFIX_PATH}/include/common_msgs/std_msgs
)

#cpplint and cppcheck 
#Cpplint checks C/C++ files for style issues following Google's C++ style guide.
option(CODECHECK "cpplint and cppcheck to detect the code" OFF)
if(CODECHECK)
  set(CMAKE_CXX_CPPLINT "cpplint" --verbose=5 --filter=-whitespace/tab,-whitespace/braces,-whitespace/parens,-readability/braces,-build/c++11
                        --linelength=120 --extensions=h,cpp,cc --quiet --exclude=${CMAKE_CURRENT_BINARY_DIR})
  set(CMAKE_CXX_CPPCHECK "cppcheck" --inline-suppr --enable=warning,style,performance,portability,information -j4 --force
                         -DBROS_VERSION_2_ABOVE -USQLITE_INT64_TYPE -UERROR --platform=unix64 --quiet --std=c++11
                         -i ${CMAKE_CURRENT_BINARY_DIR} --template='{file}\t{line}\t{severity}\t{id}\t{message}\t{code}')
endif()

## Declare a C++ library
# add_library(${PROJECT_NAME}
#   src/${PROJECT_NAME}/web_server.cpp
# )
## Add cmake target dependencies of the library
## as an example, code may need to be generated before libraries
## either from message generation or dynamic reconfigure
#add_dependencies(${PROJECT_NAME} ${${PROJECT_NAME}_EXPORTED_TARGETS} ${catkin_EXPORTED_TARGETS})

## Declare a C++ executable
## With catkin_make all packages are built within a single CMake context
## The recommended prefix ensures that target names across packages don't collide
# add_executable(${PROJECT_NAME}_node src/web_server_node.cpp)
add_executable(${PROJECT_NAME}
  ${PROTO_SRCS}
  #src/FSM/FSM.cpp
  src/main.cpp 
  #src/pad_base.cpp
  src/modbus_mode.cpp
  #src/robot_outwallspray.cpp
  src/robot_motion.cpp
  #src/singleton.cpp
  #src/hanger_driver.cpp
  #src/RS485.cpp
  #src/robotlink.cpp
)

## Rename C++ executable without prefix
## The above recommended prefix causes long target names, the following renames the
## target back to the shorter version for ease of user use
## e.g. "rosrun someones_pkg node" instead of "rosrun someones_pkg someones_pkg_node"
# set_target_properties(${PROJECT_NAME}_node PROPERTIES OUTPUT_NAME node PREFIX "")
set_target_properties(${PROJECT_NAME} PROPERTIES LINK_FLAGS -pthread)

## Add cmake target dependencies of the executable
## same as for the library above
# add_dependencies(${PROJECT_NAME}_node ${${PROJECT_NAME}_EXPORTED_TARGETS} ${catkin_EXPORTED_TARGETS})
# add_dependencies(${PROJECT_NAME} ${PROJECT_NAME}_gencpp)

## Specify libraries to link a library or executable target against
# target_link_libraries(${PROJECT_NAME}_node
#   ${catkin_LIBRARIES}
# )
if(USE_TX2_LIB)
  #tx2_platform
  set(LIBRARY_LINK_PATH "${PROJECT_SOURCE_DIR}/lib/tx2")
else()
  #x86_platform
  set(LIBRARY_LINK_PATH "${PROJECT_SOURCE_DIR}/lib/x86")
endif()

target_link_libraries(${PROJECT_NAME} 
  ${Boost_LIBRARIES}
  #${CMAKE_THREAD_LIBS_INIT}
  #sqlite3
  #${LIBRARY_LINK_PATH}/libsqlcipher.so.0
  bros_stdmsg
  brclcpp
  brshm
  yaml-cpp
  jsoncpp
  spdlog
)
#target_link_libraries(${PROJECT_NAME} /home/liu/radar_control/lib/radar)
option (USE_LICENSE "Use license" off)

if(USE_LICENSE)
  message("use license")
  add_definitions(-DUSE_LICENSE)
  target_link_libraries(${PROJECT_NAME} ${LIBRARY_LINK_PATH}/liblicense.a)
endif()

#############
## Package ##
#############
string(REPLACE "_" "-" DEBIAN_CONTROL_PACKAGE ${PROJECT_NAME})
set(DEBIAN_CONTROL_MAINTAINER huhuilian <huhuilian@countrygarden.com.cn>)
set(DEBIAN_CONTROL_DEPENDS )
set(DEBIAN_CONTROL_SECTION "BZL")
set(DEBIAN_CONTROL_DESCRIPTION external wall spray robot)
set(          DEBIAN_CONTROL "Package: ${DEBIAN_CONTROL_PACKAGE}\\n")
string(APPEND DEBIAN_CONTROL "Version: ${DEBIAN_VERSION}\\n")
string(APPEND DEBIAN_CONTROL "Architecture: ${TARGET_ARCHITECTURE}\\n")
string(APPEND DEBIAN_CONTROL "Maintainer: ${DEBIAN_CONTROL_MAINTAINER}\\n")
string(APPEND DEBIAN_CONTROL "Depends: ${DEBIAN_CONTROL_DEPENDS}\\n")
string(APPEND DEBIAN_CONTROL "Section: ${DEBIAN_CONTROL_SECTION}\\n")
string(APPEND DEBIAN_CONTROL "Priority: optional\\n")
string(APPEND DEBIAN_CONTROL "Homepage: \\n")
string(APPEND DEBIAN_CONTROL "Description: ${DEBIAN_CONTROL_DESCRIPTION}")
set(DEBIAN_POSTRM "\#!/bin/sh\\n")
set(DEBIAN_PREINST "\#!/bin/sh\\n")
set(DEBIAN_POSTINST "\#!/bin/sh\\n")
set(DEBIAN_CONTROL_PACKAGE_NAME ${DEBIAN_CONTROL_PACKAGE}-${DEBIAN_VERSION}_${TARGET_ARCHITECTURE})
set(PACKAGE_INSTALL_DIR ${CMAKE_CURRENT_BINARY_DIR}/${DEBIAN_CONTROL_PACKAGE_NAME}/usr/local/bzl_robot)

add_custom_target(pack
  COMMAND echo "CMAKE_CURRENT_BINARY_DIR:" ${CMAKE_CURRENT_BINARY_DIR}
  COMMAND echo "CMAKE_CURRENT_SOURCE_DIR:" ${CMAKE_CURRENT_SOURCE_DIR}
  COMMAND cd ${CMAKE_CURRENT_BINARY_DIR} && rm -rf ${DEBIAN_CONTROL_PACKAGE_NAME} && mkdir -p ${DEBIAN_CONTROL_PACKAGE_NAME}/DEBIAN
  COMMAND cd ${CMAKE_CURRENT_BINARY_DIR}/${DEBIAN_CONTROL_PACKAGE_NAME}/DEBIAN && echo \"${DEBIAN_CONTROL}\" | sed 's|\\\\ | |g' >control
  COMMAND cd ${CMAKE_CURRENT_BINARY_DIR}/${DEBIAN_CONTROL_PACKAGE_NAME}/DEBIAN && echo \"${DEBIAN_POSTRM}\" | sed 's|\\\\ | |g' >postrm && chmod +x postrm
  COMMAND cd ${CMAKE_CURRENT_BINARY_DIR}/${DEBIAN_CONTROL_PACKAGE_NAME}/DEBIAN && echo '${DEBIAN_PREINST}' | sed 's|\\\\ | |g' >preinst && chmod +x preinst
  COMMAND cd ${CMAKE_CURRENT_BINARY_DIR}/${DEBIAN_CONTROL_PACKAGE_NAME}/DEBIAN && echo '${DEBIAN_POSTINST}' | sed 's|\\\\ | |g' >postinst && chmod +x postinst
  COMMAND cd ${CMAKE_CURRENT_BINARY_DIR}/${DEBIAN_CONTROL_PACKAGE_NAME} && mkdir -p usr/local/bzl_robot/sbin && mkdir -p usr/local/bzl_robot/ewsr_ms && mkdir -p usr/local/bzl_robot/ewsr_ms/log && mkdir -p usr/local/bzl_robot/ewsr_ms/config && mkdir -p usr/local/bzl_robot/ewsr_ms/info && mkdir -p home/nvidia
  #COMMAND cd ${CMAKE_CURRENT_BINARY_DIR}/${DEBIAN_CONTROL_PACKAGE_NAME} && cp ${CMAKE_CURRENT_SOURCE_DIR}/init_file/user_startup.sh home/nvidia/ && chmod +x home/nvidia/user_startup.sh
  COMMAND cd ${PACKAGE_INSTALL_DIR} && cp ${CMAKE_CURRENT_BINARY_DIR}/basket_control sbin && chmod +x sbin/basket_control
  COMMAND cd ${PACKAGE_INSTALL_DIR} && cp ${CMAKE_CURRENT_SOURCE_DIR}/init_file/config/basket_control.yaml ewsr_ms/config/
  COMMAND cd ${PACKAGE_INSTALL_DIR} && cp ${CMAKE_CURRENT_SOURCE_DIR}/init_file/config/hoister.yaml ewsr_ms/config/
  COMMAND cd ${PACKAGE_INSTALL_DIR} && cp ${CMAKE_CURRENT_SOURCE_DIR}/init_file/info/version.yaml ewsr_ms/info/
  #COMMAND cd ${PACKAGE_INSTALL_DIR} && sudo chown -R nvidia:nvidia ${PACKAGE_INSTALL_DIR}
  COMMAND cd ${CMAKE_CURRENT_BINARY_DIR}/${DEBIAN_CONTROL_PACKAGE_NAME} && md5sum `find usr -type f` >DEBIAN/md5sums
  COMMAND cd ${CMAKE_CURRENT_BINARY_DIR} && dpkg -b ${DEBIAN_CONTROL_PACKAGE_NAME} ${DEBIAN_CONTROL_PACKAGE_NAME}.deb
  COMMAND cd ${CMAKE_CURRENT_BINARY_DIR} && ([ -n \"${BUILD_TIME}\" ] && (mv ${DEBIAN_CONTROL_PACKAGE_NAME}.deb ${DEBIAN_CONTROL_PACKAGE_NAME}-${BUILD_TIME}.deb || exit 1)) || true
  COMMAND echo "package success"
  COMMENT "package ${DEBIAN_CONTROL_PACKAGE} deb"
  DEPENDS ${PROJECT_NAME}
)

#############
## Install ##
#############

# all install targets should use catkin DESTINATION variables
# See http:// to setup.py, you can choose the destination
# install(PROGRAMS
#   scripts/my_python_script
#   DESTINATION ${CATKIN_PACKAGE_BIN_DESTINATION}
# )

## Mark executables and/or libraries for installation
# install(TARGETS ${PROJECT_NAME} ${PROJECT_NAME}_node
#   ARCHIVE DESTINATION ${CATKIN_PACKAGE_LIB_DESTINATION}
#   LIBRARY DESTINATION ${CATKIN_PACKAGE_LIB_DESTINATION}
#   RUNTIME DESTINATION ${CATKIN_PACKAGE_BIN_DESTINATION}
# )

## Mark cpp header files for installation
# install(DIRECTORY include/${PROJECT_NAME}/
#   DESTINATION ${CATKIN_PACKAGE_INCLUDE_DESTINATION}
#   FILES_MATCHING PATTERN "*.h"
#   PATTERN ".svn" EXCLUDE
# )

## Mark other files for installation (e.g. launch and bag files, etc.)
# install(FILES
#   # myfile1
#   # myfile2
#   DESTINATION ${CATKIN_PACKAGE_SHARE_DESTINATION}
# )

#############
## Testing ##
#############

## Add gtest based cpp test target and link libraries
# catkin_add_gtest(${PROJECT_NAME}-test test/test_web_server.cpp)
# if(TARGET ${PROJECT_NAME}-test)
#   target_link_libraries(${PROJECT_NAME}-test ${PROJECT_NAME})
# endif()

## Add folders to be run by python nosetests
# catkin_add_nosetests(test)
