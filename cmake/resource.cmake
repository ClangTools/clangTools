#https://github.com/ClangTools/clangTools/releases/download/OpenCV_Cascades/OpenCV_Cascades.zip

option(ENABLE_OPENCV_CASCADES "option for OPENCV_CASCADES" OFF)
if (ENABLE_OPENCV_CASCADES)
    file(DOWNLOAD
            https://github.com/ClangTools/clangTools/releases/download/OpenCV_Cascades/OpenCV_Cascades.zip
            ${CMAKE_BINARY_DIR}/OpenCV_Cascades.zip
            TIMEOUT 120 INACTIVITY_TIMEOUT 120 SHOW_PROGRESS EXPECTED_MD5 "335ce200afc2ddd8e4cd28cda3b5a6c9"
            )
    add_custom_target(UnpackingOpenCV_Cascades ALL)
    add_custom_command(TARGET UnpackingOpenCV_Cascades PRE_BUILD
            COMMAND ${CMAKE_COMMAND} -E remove_directory ${CMAKE_BINARY_DIR}/data/
            COMMAND ${CMAKE_COMMAND} -E tar xzf ${CMAKE_BINARY_DIR}/OpenCV_Cascades.zip
            WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
            DEPENDS ${CMAKE_BINARY_DIR}/OpenCV_Cascades.zip
            COMMENT "Unpacking OpenCV_Cascades.zip"
            VERBATIM)

    # 复制 lib 代码到 lib 下
    install(DIRECTORY ${CMAKE_BINARY_DIR}/data/ DESTINATION data
            PATTERN "${CMAKE_BINARY_DIR}/data/*"
            PERMISSIONS
            OWNER_WRITE OWNER_READ
            GROUP_READ
            # SETUID SETGID
            )
endif (ENABLE_OPENCV_CASCADES)