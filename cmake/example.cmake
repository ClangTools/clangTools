option(clangToolsExample "option for Example" OFF)
if (clangToolsExample OR "${Tools_Other_Project}" STREQUAL "ON")
    include_directories(
            .
            ${srcSourceHeader}
    )


    FILE(GLOB ExampleSourceSrc "Example/*")

    option(ENABLE_I2C "option for I2C tool" OFF)
    if (ENABLE_I2C)
    else ()
        list(FILTER ExampleSourceSrc EXCLUDE REGEX ".*i2c.cpp")
    endif ()

    option(ENABLE_OPENCV "option for OPENCV tool" OFF)
    if (ENABLE_OPENCV)
    else ()
        list(FILTER ExampleSourceSrc EXCLUDE REGEX ".*opencv.*\\.cpp")
    endif ()
    option(ENABLE_LIBUSB1 "option for libusb-1.0 tool" OFF)
    if (ENABLE_LIBUSB1)
    else ()
        list(FILTER ExampleSourceSrc EXCLUDE REGEX ".*libusb.cpp")
    endif ()

    option(ENABLE_FILENAME "option for FILENAME" ON)
    set(demo_name_list "")
    foreach (item ${ExampleSourceSrc})
        #获取当前文件的绝对路径
        get_filename_component(filepath "${item}" ABSOLUTE)
        string(REGEX REPLACE ".+Example/(.+)\\..*" "\\1" demo_name ${filepath})
        list(APPEND demo_name_list "${demo_name}\\;")
        add_executable(${demo_name} ${filepath})
        target_link_libraries(${demo_name} ${libTools_LIBRARIES} ${dl_LIBRARIES} ${OPENSSL_LIBRARIES})
        if (ENABLE_FILENAME)
            redefine_file_macro(${demo_name})
        endif ()
        if (CURL_FOUND)
            target_link_libraries(${demo_name} ${CURL_LIBRARIES})
        endif ()
        install(TARGETS ${demo_name}
                RUNTIME DESTINATION bin
                LIBRARY DESTINATION bin)
    endforeach ()
    function(JOIN VALUES GLUE OUTPUT)
        string(REPLACE ";" "${GLUE}" _TMP_STR "${VALUES}")
        set(${OUTPUT} "${_TMP_STR}" PARENT_SCOPE)
    endfunction()
    message(STATUS Example " : " ${demo_name_list})
    message("-- install to " ${CMAKE_INSTALL_PREFIX})
endif ()
