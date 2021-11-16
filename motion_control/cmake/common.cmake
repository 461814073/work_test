#set compile strategy
if(USE_CROSS_COMPILE)
	message("-- Use Cross_compile!")
	option (USE_TX2_LIB "The target platform is aarch64!!!" ON)
	message("-- The target platform is aarch64 !")
    
	if (NOT EXISTS ${CMAKE_SYSROOT}/usr)
        message(FATAL_ERROR "Cross compile system directory ${CMAKE_SYSROOT} does not exists!")
    endif()
	
    set(TARGET_ARCHITECTURE "arm64")
else()
	set(CMAKE_SYSROOT "")
	set(CMAKE_PREFIX_PATH /opt/bros)

	if(${CMAKE_SYSTEM_PROCESSOR} STREQUAL x86_64)
        set(TARGET_ARCHITECTURE "amd64")
        option (USE_TX2_LIB "The target platform is x86_64!!!" OFF)
        message("-- The target platform is x86_64 !")
	elseif(${CMAKE_SYSTEM_PROCESSOR} STREQUAL aarch64)
        set(TARGET_ARCHITECTURE "arm64")
        option (USE_TX2_LIB "The target platform is aarch64!!!" ON)
        message("-- The target platform is aarch64 !")
	else()
        message(FATAL_ERROR "target platform is unknow, neither x86_64 or aarch64")
	endif()
endif(USE_CROSS_COMPILE)

