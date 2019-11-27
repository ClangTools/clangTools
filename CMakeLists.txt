cmake_minimum_required(VERSION 3.0)

if ("${PROJECT_NAME}" STREQUAL "Project")
    set(Tools_Other_Project ON)
elseif ("${PROJECT_NAME}" STREQUAL "")
    set(Tools_Other_Project ON)
endif()
project(Tools)
set(CMAKE_CXX_STANDARD 14)

set(libTools_LIBRARIES Tools)
set_property(GLOBAL PROPERTY libTools_LIBRARIES ${libTools_LIBRARIES})

if (WIN32)
else ()
    find_library(dl dl)
    set(dl_LIBRARIES dl)
endif ()

option(ENABLE_FILENAME "option for FILENAME" ON)
if (ENABLE_FILENAME)
    #重新定义当前目标的源文件的 __FILENAME__ 宏
    function(redefine_file_macro targetname)
        #获取当前目标的所有源文件
        get_target_property(source_files "${targetname}" SOURCES)
        #遍历源文件
        foreach (sourcefile ${source_files})
            #获取当前源文件的编译参数
            get_property(defs SOURCE "${sourcefile}"
                    PROPERTY COMPILE_DEFINITIONS)
            #获取当前文件的绝对路径
            get_filename_component(filepath "${sourcefile}" ABSOLUTE)
            #将绝对路径中的项目路径替换成空,得到源文件相对于项目路径的相对路径
            string(REPLACE ${PROJECT_SOURCE_DIR}/ "" relpath ${filepath})
            #将我们要加的编译参数( __FILENAME__ 定义)添加到原来的编译参数里面
            list(APPEND defs "__FILENAME__=\"${relpath}\"")
            #重新设置源文件的编译参数
            set_property(
                    SOURCE "${sourcefile}"
                    PROPERTY COMPILE_DEFINITIONS ${defs}
            )
        endforeach ()
    endfunction()
endif ()


# 设置为 v140_xp 编译工具编译
# set(CMAKE_GENERATOR_TOOLSET "v140_xp")
# set(CMAKE_GENERATOR_PLATFORM "Win32")

if (MSVC)
    set(CompilerFlags
            CMAKE_CXX_FLAGS
            CMAKE_CXX_FLAGS_DEBUG
            CMAKE_CXX_FLAGS_RELEASE
            CMAKE_C_FLAGS
            CMAKE_C_FLAGS_DEBUG
            CMAKE_C_FLAGS_RELEASE
            )
    foreach (CompilerFlag ${CompilerFlags})
        string(REPLACE "/MD" "/MT" ${CompilerFlag} "${${CompilerFlag}}")
    endforeach ()
    # It should be set to 1 for the static MFC library, and 2 for the shared MFC library.
    set(CMAKE_MFC_FLAG 1)
    #add_definitions(-DUNICODE -D_UNICODE)
    add_definitions(-D_AFXDLL)
endif (MSVC)

option(ENABLE_OPENCV "option for OpenCV" OFF)
option(OpenCV_DIR "option for OpenCV DIR" E:\\OpenCvLib\\build_WIN32\\x86\\vc16\\lib)
if (ENABLE_OPENCV)
    add_definitions(-DENABLE_OPENCV)
    find_package(OpenCV REQUIRED)
    include_directories(${OpenCV_INCLUDE_DIRS})

endif ()
include_directories(
        .
        src/
)

aux_source_directory(src srcSource)

add_definitions(
        -D_CRT_SECURE_NO_WARNINGS

)

add_library(${libTools_LIBRARIES} STATIC
        ${RC_FILES}
        ${srcSource}
        )

target_compile_definitions(
        ${libTools_LIBRARIES}
        PUBLIC -D_${libTools_LIBRARIES}_HEADER_ #-D_LOGGER_USE_THREAD_POOL_
)
target_link_libraries(${libTools_LIBRARIES} ${OpenCV_LIBS} ${dl_LIBRARIES})

if ("${Tools_Other_Project}" STREQUAL "ON")
    add_executable(logger_Demo Example/logger_demo.cpp)
    add_executable(thread_pool_demo Example/thread_pool_demo.cpp)
    target_link_libraries(logger_Demo ${libTools_LIBRARIES} ${dl_LIBRARIES})
    target_link_libraries(thread_pool_demo ${libTools_LIBRARIES} ${dl_LIBRARIES})

    option(ENABLE_FILENAME "option for FILENAME" ON)
    if (ENABLE_FILENAME)
        redefine_file_macro(logger_Demo)
        redefine_file_macro(thread_pool_demo)
    endif ()

    install(TARGETS logger_Demo thread_pool_demo
            RUNTIME DESTINATION bin
            LIBRARY DESTINATION bin)
    message("-- install to " ${CMAKE_INSTALL_PREFIX})
endif ()


add_custom_command(TARGET ${libTools_LIBRARIES} POST_BUILD        # Adds a post-build event to MyTest
        COMMAND ${CMAKE_COMMAND} -E copy_directory   # which executes "cmake - E copy_if_different..."
        "${PROJECT_SOURCE_DIR}/Depend"      # <--this is in-file
        "${CMAKE_CURRENT_BINARY_DIR}"
        )                 # <--this is out-file path


option(ENABLE_FILENAME "option for FILENAME" ON)
if (ENABLE_FILENAME)
    #重新定义当前目标的源文件的 __FILENAME__ 宏
    redefine_file_macro(${libTools_LIBRARIES})
endif ()



########安装脚本##########
install(TARGETS ${libTools_LIBRARIES}
        RUNTIME DESTINATION lib
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib)

#install(FILES ${libTools_LIBRARIES}/${libTools_LIBRARIES}.h DESTINATION include)
#
## 复制 bin 代码到 bin 下
#install(DIRECTORY Depend/ DESTINATION bin
#        PATTERN "Depend/*"
#        PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ
#        GROUP_EXECUTE GROUP_READ)
## 复制 lib 代码到 lib 下
#install(DIRECTORY Example/ DESTINATION Example
#        PATTERN "Example/*"
#        PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ
#        GROUP_EXECUTE GROUP_READ)
