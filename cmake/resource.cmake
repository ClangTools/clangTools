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

option(ENABLE_FREETYPE "option for FREETYPE" OFF)
if (ENABLE_FREETYPE)
    file(DOWNLOAD
            https://github.com/ClangTools/clangTools/releases/download/OpenDotMatrixFont/OpenDotFont.zip
            ${CMAKE_BINARY_DIR}/OpenDotFont.zip
            TIMEOUT 120 INACTIVITY_TIMEOUT 120 SHOW_PROGRESS EXPECTED_MD5 "93025e5fae8b85151e4b1ad4be460f2d"
            )
    add_custom_target(UnpackingOpenDotFont ALL)
    add_custom_command(TARGET UnpackingOpenDotFont PRE_BUILD
            COMMAND ${CMAKE_COMMAND} -E remove ${CMAKE_BINARY_DIR}/OpenDotMatrixFont.ttf
            COMMAND ${CMAKE_COMMAND} -E remove ${CMAKE_BINARY_DIR}/seguisym.ttf
            COMMAND ${CMAKE_COMMAND} -E tar xzf ${CMAKE_BINARY_DIR}/OpenDotFont.zip
            WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
            DEPENDS ${CMAKE_BINARY_DIR}/OpenDotFont.zip
            COMMENT "Unpacking OpenDotFont.zip"
            VERBATIM)

    install(FILES
            ${CMAKE_BINARY_DIR}/OpenDotMatrixFont.ttf
            ${CMAKE_BINARY_DIR}/seguisym.ttf
            DESTINATION font)

endif (ENABLE_FREETYPE)
option(ENABLE_HZK "option for HZK" OFF)
if (ENABLE_HZK)
    file(DOWNLOAD
            https://github.com/ClangTools/clangTools/releases/download/HzkFont/HzkFont.zip
            ${CMAKE_BINARY_DIR}/HzkFont.zip
            TIMEOUT 120 INACTIVITY_TIMEOUT 120 SHOW_PROGRESS EXPECTED_MD5 "e8d9666f8231ce263f006ea40f2ef3fb"
            )
    add_custom_target(UnpackingHzkFont ALL)
    add_custom_command(TARGET UnpackingHzkFont PRE_BUILD
            COMMAND ${CMAKE_COMMAND} -E remove_directory ${CMAKE_BINARY_DIR}/font
            COMMAND ${CMAKE_COMMAND} -E tar xzf ${CMAKE_BINARY_DIR}/HzkFont.zip
            WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
            DEPENDS ${CMAKE_BINARY_DIR}/HzkFont.zip
            COMMENT "Unpacking HzkFont.zip"
            VERBATIM)

    # 复制 lib 代码到 lib 下
    install(DIRECTORY ${CMAKE_BINARY_DIR}/font/ DESTINATION bin/font
            PATTERN "${CMAKE_BINARY_DIR}/font/*"
            PERMISSIONS
            OWNER_WRITE OWNER_READ
            GROUP_READ
            # SETUID SETGID
            )
endif (ENABLE_HZK)
option(ENABLE_HZK "option for HZK" OFF)
if (ENABLE_HZK)
    file(DOWNLOAD
            https://github.com/ClangTools/clangTools/releases/download/gov.json/gov.json.zip
            ${CMAKE_BINARY_DIR}/gov.json.zip
            TIMEOUT 120 INACTIVITY_TIMEOUT 120 SHOW_PROGRESS EXPECTED_MD5 "6b8b2b0e9f85de3f62cf5fc396c0ca33"
            )
    add_custom_target(UnpackingHzkFont ALL)
    add_custom_command(TARGET UnpackingHzkFont PRE_BUILD
            COMMAND ${CMAKE_COMMAND} -E remove ${CMAKE_BINARY_DIR}/gov.json
            COMMAND ${CMAKE_COMMAND} -E tar xzf ${CMAKE_BINARY_DIR}/gov.json.zip
            WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
            DEPENDS ${CMAKE_BINARY_DIR}/gov.json.zip
            COMMENT "Unpacking gov.json.zip"
            VERBATIM)

    install(FILES
            ${CMAKE_BINARY_DIR}/gov.json
            DESTINATION bin)
endif (ENABLE_HZK)