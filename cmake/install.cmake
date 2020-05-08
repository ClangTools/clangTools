
if ("${Tools_Other_Project}" STREQUAL "ON")
    ########安装脚本##########
    install(TARGETS ${libTools_LIBRARIES}
            RUNTIME DESTINATION lib
            LIBRARY DESTINATION lib
            ARCHIVE DESTINATION lib)

    FILE(GLOB srcInstallSourceHeader "${CMAKE_CURRENT_LIST_DIR}/../src/*")

    set(include_header "")
    foreach (item in ${srcInstallSourceHeader})
        FILE(GLOB subSrcSource "${item}/*.h*")
        list(APPEND include_header ${subSrcSource})
    endforeach ()

    install(FILES
            ${include_header}
            DESTINATION include)

    # 复制 lib 代码到 lib 下
    install(DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/../Example/ DESTINATION Example
            PATTERN "${CMAKE_CURRENT_LIST_DIR}/../Example/*"
            PERMISSIONS
            OWNER_WRITE OWNER_READ
            GROUP_READ
            # SETUID SETGID
            )
    include(${CMAKE_CURRENT_SOURCE_DIR}/vInstallCpack.cmake)
endif ()