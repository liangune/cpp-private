
cmake_minimum_required(VERSION 3.4)

project(db)

if(MSVC)
    set(PROJECT_NAME libdb)
endif()

#-------------------------------------------------------------------
#-------------------------------Platform Architecture---------------
if(MSVC)
    set(PLATFORM windows)
    if(CMAKE_CL_64)
        set(ARCHITECTURE x64)
    else()
        set(ARCHITECTURE x86)
    endif()
elseif(UNIX)
    set(PLATFORM linux)
    if(CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64")
        set(ARCHITECTURE x64)
    elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "aarch64")
        set(ARCHITECTURE arm64)
    else()
        set(ARCHITECTURE x86)
    endif()
endif()

#-------------------------------------------------------------------
#-------------------------------Export Symbols----------------------
if(MSVC AND BUILD_SHARED_LIBS)
	set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
endif()

#-------------------------------------------------------------------
#-------------------------------Function----------------------------
#遍历添加头文件包含路径函数
function(include_sub_directories_recursively root_dir)
    if(IS_DIRECTORY ${root_dir}) # 当前路径是一个目录就加入到包含目录
        #if(${root_dir} MATCHES "include")
            #message("include dir: " ${root_dir})
            include_directories(${root_dir})
        #endif()
    endif()

    # 获得当前目录下的所有文件，加入ALL_SUB列表中
    file(GLOB ALL_SUB RELATIVE ${root_dir} ${root_dir}/*) 
    foreach(sub ${ALL_SUB})
        if (IS_DIRECTORY ${root_dir}/${sub})
            include_sub_directories_recursively(${root_dir}/${sub}) # 对子目录递归调用
        endif()
    endforeach()
endfunction()

#-------------------------------------------------------------------
#-------------------------------Source------------------------------
# 获取目录及子目录全部源文件
# file(GLOB_RECURSE ALL_SOURCES 
#    ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp 
#    ${CMAKE_CURRENT_SOURCE_DIR}/*.c
#)

# 获取目录全部源文件, 不遍历子目录
file(GLOB ALL_SOURCES 
    ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
)

# library path
set(COMPONENT ${CMAKE_CURRENT_SOURCE_DIR}/../../Depend)

# 添加头文件路径
# include_sub_directories_recursively(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
	${COMPONENT}/include
)

# 子目录
# add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/xxx)

#-------------------------------------------------------------------
#-------------------------------Flags-------------------------------
#  c++编译选项
if(MSVC)
	set(CMAKE_CXX_FLAGS_DEBUG "-Wall /Ob0 /MDd /Zi")
	set(CMAKE_CXX_FLAGS_RELEASE "-O2 -Wall /MD")
	set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /Zi")
elseif(UNIX)
	set(CMAKE_CXX_FLAGS_DEBUG "-g -ggdb -pipe -Wall -fPIC")
	set(CMAKE_CXX_FLAGS_RELEASE "-O2 -g -ggdb -fno-strict-aliasing -pipe -Wall -fPIC")
endif()

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# c编译选项
if(UNIX)
	set(CMAKE_C_FLAGS_DEBUG "-g -ggdb -pipe -Wall -fPIC -std=c99 -pedantic")
	set(CMAKE_C_FLAGS_RELEASE "-O2 -g -ggdb -fno-strict-aliasing -pipe -Wall -fPIC -std=c99 -pedantic")
endif()

#-------------------------------------------------------------------
#--------------------------------Link-------------------------------
if(BUILD_SHARED_LIBS)
    set(BUILD_LIB_TYPE dynamic)
else()
    set(BUILD_LIB_TYPE static)
endif()

set(LINK_ARCHIVES_DIR ${COMPONENT}/lib/${PLATFORM}/${ARCHITECTURE}/static)
set(LINK_LIBS_DIR ${COMPONENT}/lib/${PLATFORM}/${ARCHITECTURE}/dynamic)

set(libpq libpq_c)
if(ARCHITECTURE MATCHES x64) 
    set(libpq libpq_c64)
endif()

if(MSVC)
    # 动态库
    set(LINK_LIBS
        ${LINK_LIBS_DIR}/${libpq}/libpq.lib
    )

    # 静态库
    set(LINK_ARCHIVES

    )
elseif(UNIX)
    # 动态库
    set(LINK_LIBS
        pthread
    )

    # 静态库
    set(LINK_ARCHIVES
        ${LINK_ARCHIVES_DIR}/libpq/libpq.a
    )
endif()

set(LINK_LIBS_DIR
    ${COMPONENT}/lib/${PLATFORM}/${ARCHITECTURE}/dynamic
)

# 链接库路径
link_directories(${LINK_LIBS_DIR})

#-------------------------------------------------------------------
#-----------------------------Out Put Dir---------------------------

SET(LIBRARY_OUTPUT_PATH_PREFIX ${COMPONENT})
set(INCLUDE_OUTPUT_DIR ${LIBRARY_OUTPUT_PATH_PREFIX}/include)
set(PROJECT_DIR libdb)
set(OUTPUT_DIR ${LIBRARY_OUTPUT_PATH_PREFIX}/lib/${PLATFORM}/${ARCHITECTURE}/${BUILD_LIB_TYPE}/${PROJECT_DIR})

#--------------------------------------------------------------------
#-------------------------------Compile------------------------------
set(LIBRARY_VERSION 1.1)
# 生成库文件，STATIC静态库，SHARED动态库
if(BUILD_SHARED_LIBS)
    add_library(${PROJECT_NAME} SHARED ${ALL_SOURCES})
    #set_target_properties(${PROJECT_NAME} PROPERTIES VERSION ${LIBRARY_VERSION})
else()
	add_library(${PROJECT_NAME} STATIC ${ALL_SOURCES})
endif()

# pdb文件
if(MSVC)
	set_target_properties(${PROJECT_NAME} PROPERTIES 
		COMPILE_PDB_NAME ${PROJECT_NAME}
	)
endif()

# 链接库
target_link_libraries(${PROJECT_NAME} ${LINK_LIBS} ${LINK_ARCHIVES})

# 输出库文件(ARCHIVE静态库，RUNTIME可执行文件，LIBRARY动态库)到指定目录,make install生效
if(BUILD_SHARED_LIBS)
   	if(MSVC)
		install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION ${OUTPUT_DIR})
		install(TARGETS ${PROJECT_NAME} ARCHIVE DESTINATION ${OUTPUT_DIR})
		install(FILES $<TARGET_PDB_FILE:${PROJECT_NAME}> DESTINATION ${OUTPUT_DIR})
	elseif(UNIX)
		install(TARGETS ${PROJECT_NAME} LIBRARY DESTINATION ${OUTPUT_DIR})
	endif()
else()
	install(TARGETS ${PROJECT_NAME} ARCHIVE DESTINATION ${OUTPUT_DIR})
	if(MSVC)
		install(FILES "${PROJECT_BINARY_DIR}/${CMAKE_BUILD_TYPE}/${PROJECT_NAME}.pdb"
			DESTINATION ${OUTPUT_DIR}
		)
	endif()
endif()

#--------------------------------------------------------------------
#-------------------------------Copy File----------------------------
# 拷贝文件
file(GLOB CURRENT_INCLUDE 
    ${CMAKE_CURRENT_SOURCE_DIR}/*.h)

add_custom_target(copy)

if(NOT EXISTS ${INCLUDE_OUTPUT_DIR}/${PROJECT_DIR})
    #make_directory("${INCLUDE_OUTPUT_DIR}/${PROJECT_DIR}")
    file(MAKE_DIRECTORY "${INCLUDE_OUTPUT_DIR}/${PROJECT_DIR}")
endif()

add_custom_command(TARGET copy 
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E 
    copy ${CURRENT_INCLUDE} ${INCLUDE_OUTPUT_DIR}/${PROJECT_DIR}/
)