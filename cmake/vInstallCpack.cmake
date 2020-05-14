
if ("${Tools_Other_Project}" STREQUAL "ON")
    #vInstallCpack.cmake
    ###cpack
    string(TIMESTAMP vTimeStamp "%Y%m%d%H%M%S")

    set(CPACK_GENERATOR "TGZ")
    set(CPACK_PACKAGE_FILE_NAME "${PROJECT_NAME}-${vTimeStamp}")
    include(CPack)
endif ()