### name  : buildprotofiles.cmake
### vesion: 1.0.0
### author: liuyoulin@countrygarden.com.cn
### date  : 2020.01.09


##设置生成的proto文件的路径.
SET(PROTO_META_BASE_DIR ${CMAKE_CURRENT_BINARY_DIR})
message("PROTO_META_BASE_DIR = ${PROTO_META_BASE_DIR}")

##设置proto文件的主目录.
SET(PROTO_SRC_ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/proto)
message("PROTO_SRC_ROOT_DIR = ${PROTO_SRC_ROOT_DIR}")

##添加proto编译flag
LIST(APPEND PROTO_FLAGS -I${PROTO_SRC_ROOT_DIR})

##递归获取目录下所有.proto文件
FILE(GLOB_RECURSE ALL_PROTOS ${PROTO_SRC_ROOT_DIR}/*.proto)

message("---")
##循环编译
FOREACH(FIL ${ALL_PROTOS})
  ##获取文件路径
  GET_FILENAME_COMPONENT(ABS_DIRECTORY ${FIL} DIRECTORY)

  ##获取文件名
  GET_FILENAME_COMPONENT(FIL_WE ${FIL} NAME_WE)
  message("Add-file:${FIL}")

  ##增加判定,解决子目录下文件的编译问题
  if(${ABS_DIRECTORY} STREQUAL ${PROTO_SRC_ROOT_DIR})
      LIST(APPEND PROTO_SRCS "${CMAKE_CURRENT_BINARY_DIR}/${FIL_WE}.pb.cc")
      LIST(APPEND PROTO_HDRS "${CMAKE_CURRENT_BINARY_DIR}/${FIL_WE}.pb.h")
   else()
      #string(LENGTH (string) (output variable))
      string(LENGTH ${PROTO_SRC_ROOT_DIR} ROOT_LENGTH)
      string(LENGTH ${ABS_DIRECTORY} ABS_DIRECTORY_LENGTH)

      # string(SUBSTRING (string) (begin) (length) (output variable))
      string(SUBSTRING ${ABS_DIRECTORY} ${ROOT_LENGTH} ${ABS_DIRECTORY_LENGTH} SUB_DIR)

      LIST(APPEND PROTO_SRCS "${CMAKE_CURRENT_BINARY_DIR}/${SUB_DIR}/${FIL_WE}.pb.cc")
      LIST(APPEND PROTO_HDRS "${CMAKE_CURRENT_BINARY_DIR}/${SUB_DIR}/${FIL_WE}.pb.h")
  endif()

  EXECUTE_PROCESS(
      COMMAND ${PROTOBUF_PROTOC_EXECUTABLE} ${PROTO_FLAGS} --cpp_out=${PROTO_META_BASE_DIR} ${FIL}
      WORKING_DIRECTORY ${PROTO_SRC_ROOT_DIR}
  )
ENDFOREACH()
 


