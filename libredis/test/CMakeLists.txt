cmake_minimum_required(VERSION 3.4)

project(testredis)

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
    if(CMAKE_SIZEOF_VOID_P)
        set(ARCHITECTURE x64)
    else()
        set(ARCHITECTURE x86)
    endif()
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
set(COMPONENT ${CMAKE_CURRENT_SOURCE_DIR}/../../../Depend)

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
set(LINK_ARCHIVES_DIR ${COMPONENT}/lib/${PLATFORM}/${ARCHITECTURE}/static)
set(LINK_LIBS_DIR ${COMPONENT}/lib/${PLATFORM}/${ARCHITECTURE}/dynamic)
if(MSVC)
    # 动态库
    set(LINK_LIBS
        ${LINK_LIBS_DIR}/libredis/libredis.lib
    )

    # 静态库
    set(LINK_ARCHIVES

    )
elseif(UNIX)
    # 动态库
    set(LINK_LIBS
        pthread
        ${LINK_LIBS_DIR}/libredis/libredis.so
        ${LINK_LIBS_DIR}/libpublic/libpublic.so
    )

    # 静态库
    set(LINK_ARCHIVES
 
    )
endif()

# 链接库路径
link_directories(${LINK_LIBS_DIR})

#-------------------------------------------------------------------
#-----------------------------Out Put Dir---------------------------
set(OUTPUT_DIR ${PROJECT_SOURCE_DIR}/bin)

#--------------------------------------------------------------------
#-------------------------------Compile------------------------------
# 生成可执行文件
add_executable(${PROJECT_NAME} ${ALL_SOURCES})

# 链接库
target_link_libraries(${PROJECT_NAME} ${LINK_LIBS} ${LINK_ARCHIVES})

# 输出库文件(ARCHIVE静态库，RUNTIME可执行文件，LIBRARY动态库)到指定目录,make install生效
install(TARGETS ${PROJECT_NAME} 
        RUNTIME DESTINATION ${OUTPUT_DIR}
)
