##-------------------------------------------------##:
#准备的版本设置
set(_VERSION_MAJOR 1)
set(_VERSION_MINOR 0)
set(_VERSION_PATCH 0)

#说明要生成的是deb包
set(CPACK_GENERATOR "DEB")

############下面是设置debian/control文件中的内容
#设置版本信息
set(CPACK_PACKAGE_VERSION_MAJOR "${_VERSION_MAJOR}")
set(CPACK_PACKAGE_VERSION_MINOR "${_VERSION_MINOR}")
set(CPACK_PACKAGE_VERSION_PATCH "${_VERSION_PATCH}")

#设置架构
set(CPACK_DEBIAN_PACKAGE_ARCHITECTURE "arm64")

#设置安装包的包名，打好的包将会是packagename-version-linux.deb，如果不设置，默认是工程名
set(CPACK_PACKAGE_NAME "${CMAKE_PROJECT_NAME}-${CPACK_DEBIAN_PACKAGE_ARCHITECTURE}")

#设置程序名，就是程序安装后的名字
set(CPACK_DEBIAN_PACKAGE_NAME ${CMAKE_PROJECT_NAME})


#设置依赖
set(CPACK_DEBIAN_PACKAGE_DEPENDS 
  #"libc6 (>= 2.17),fast-rtps,zlib1g-dev,libpcre3-dev,uuid-dev,libatomic1"
)

#设置section
set(CPACK_DEBIAN_PACKAGE_SECTION "devel")

#设置priority
set(CPACK_DEBIAN_PACKAGE_PRIORITY "optional")

#设置description
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "xxxxxxxxxx")

#设置联系方式
set(CPACK_PACKAGE_CONTACT "xxx@countrygarden.com.cn")

#设置维护人
set(CPACK_DEBIAN_PACKAGE_MAINTAINER "xxx@countrygarden.com.cn")

include(CPack)
##-------------------------------------------------##
