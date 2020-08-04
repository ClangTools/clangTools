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
        # message("CMAKE_SOURCE_DIR ${CMAKE_SOURCE_DIR}")
        # message("PROJECT_SOURCE_DIR ${PROJECT_SOURCE_DIR}")
        # message("_SOURCE_DIR ${_SOURCE_DIR}")
        #将绝对路径中的项目路径替换成空,得到源文件相对于项目路径的相对路径
        string(REPLACE ${CMAKE_SOURCE_DIR}/ "" relpath ${filepath})
        if ("${CMAKE_BUILD_TYPE}" STREQUAL "Release")
            string(REPLACE src/ "" relpath ${relpath})
        endif ()
        #将我们要加的编译参数( __FILENAME__ 定义)添加到原来的编译参数里面
        list(APPEND defs "__FILENAME__=\"${relpath}\"")
        #重新设置源文件的编译参数
        set_property(
                SOURCE "${sourcefile}"
                PROPERTY COMPILE_DEFINITIONS ${defs}
        )
    endforeach ()
endfunction()