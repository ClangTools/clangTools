option(clangToolsExample "option for Example" OFF)
if (clangToolsExample OR "${Tools_Other_Project}" STREQUAL "ON")
    include_directories(
            .
            ${srcSourceHeader}
    )

    add_executable(logger_Demo Example/logger_demo.cpp)
    add_executable(http_Demo Example/http_demo.cpp)
    add_executable(thread_pool_demo Example/thread_pool_demo.cpp)
    add_executable(tinyxml2_demo Example/tinyxml2_demo.cpp)
    add_executable(json_demo Example/json_demo.cpp)
    add_executable(memory_share_demo Example/memory_share_demo.cpp)
    add_executable(pipe_demo Example/pipe_demo.cpp)
    target_link_libraries(logger_Demo ${libTools_LIBRARIES} ${dl_LIBRARIES})
    target_link_libraries(thread_pool_demo ${libTools_LIBRARIES} ${dl_LIBRARIES})
    target_link_libraries(tinyxml2_demo ${libTools_LIBRARIES} ${dl_LIBRARIES})
    target_link_libraries(json_demo ${libTools_LIBRARIES} ${dl_LIBRARIES})
    target_link_libraries(memory_share_demo ${libTools_LIBRARIES} ${dl_LIBRARIES})
    target_link_libraries(pipe_demo ${libTools_LIBRARIES} ${dl_LIBRARIES})

    option(ENABLE_FILENAME "option for FILENAME" ON)
    if (ENABLE_FILENAME)
        redefine_file_macro(http_Demo)
        redefine_file_macro(logger_Demo)
        redefine_file_macro(tinyxml2_demo)
        redefine_file_macro(thread_pool_demo)
        redefine_file_macro(memory_share_demo)
        redefine_file_macro(pipe_demo)
        redefine_file_macro(json_demo)
    endif ()

    if (CURL_FOUND)
        target_link_libraries(http_Demo ${CURL_LIBRARIES})
    endif ()
    target_link_libraries(http_Demo ${libTools_LIBRARIES} ${dl_LIBRARIES} ${OPENSSL_LIBRARIES})
    install(TARGETS http_Demo logger_Demo thread_pool_demo tinyxml2_demo json_demo memory_share_demo pipe_demo
            RUNTIME DESTINATION bin
            LIBRARY DESTINATION bin)
    message("-- install to " ${CMAKE_INSTALL_PREFIX})
endif ()
